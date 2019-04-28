#include "Particle.h"
#include "debugWindow.h"

/************************************
マクロ定義
*************************************/
#define PARTICLE_NUM	(20)

/************************************
プロトタイプ宣言
*************************************/
void DrawColorAndBloom();
void ProcessBlur();
void BlendBloom(bool useBloom);
void ResizeScreenBuff(float width, float height);

/************************************
グローバル変数
*************************************/
static Transform trasnform;
static ParticleTex uv;

static ParticleVertex vtx[4] =
{
	{ D3DXVECTOR3(-10.0f,  10.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0)},
	{ D3DXVECTOR3(10.0f,  10.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0) },
	{ D3DXVECTOR3(-10.0f, -10.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0) },
	{ D3DXVECTOR3(10.0f, -10.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0) },
};

static LPDIRECT3DVERTEXBUFFER9 vtxBuff;
static LPDIRECT3DVERTEXBUFFER9 worldBuff;
static LPDIRECT3DVERTEXBUFFER9 uvBuff;
static LPDIRECT3DVERTEXBUFFER9 anotherVtx;

static LPDIRECT3DVERTEXDECLARATION9 declare;
static LPD3DXEFFECT effect;
static LPDIRECT3DINDEXBUFFER9 indexBuff;
static LPDIRECT3DTEXTURE9 texture;

static LPDIRECT3DTEXTURE9 colorTexture, bloomTexture;
static LPDIRECT3DTEXTURE9 blurTexture[2];
static LPDIRECT3DSURFACE9 colorSurface, bloomSurface;
static LPDIRECT3DSURFACE9 blurSurface[2];

static LPDIRECT3DVERTEXBUFFER9 screenBuff;

static LPD3DXEFFECT blurFilter;

/************************************
初期化処理
*************************************/
void InitParticle(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	texture = CreateTextureFromFile((LPSTR)"bulletParticle.png", pDevice);

	pDevice->SetTexture(0, texture);

	pDevice->CreateVertexBuffer(sizeof(vtx), 0, 0, D3DPOOL_MANAGED, &vtxBuff, 0);
	pDevice->CreateVertexBuffer(sizeof(Transform) * PARTICLE_NUM, 0, 0, D3DPOOL_MANAGED, &worldBuff, 0);
	pDevice->CreateVertexBuffer(sizeof(ParticleTex) * PARTICLE_NUM, 0, 0, D3DPOOL_MANAGED, &uvBuff, 0);
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4, D3DUSAGE_WRITEONLY, FVF_VERTEX_3D, D3DPOOL_MANAGED, &anotherVtx, 0);

	ParticleVertex *pVtx;
	vtxBuff->Lock(0, 0, (void**)&pVtx, 0);
	memcpy(pVtx, vtx, sizeof(vtx));
	vtxBuff->Unlock();

	Transform *pTr;
	worldBuff->Lock(0, 0, (void**)&pTr, 0);
	for (int i = 0; i < PARTICLE_NUM; i++, pTr++)
	{
		pTr->pos = D3DXVECTOR3(RandomRangef(-50.0f, 50.0f), RandomRangef(-20.0f, 20.0f), RandomRangef(50.0f, 100.0f));
		pTr->rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		pTr->scl = D3DXVECTOR3(0.5f, 0.5f, 1.0f);
	}
	worldBuff->Unlock();

	ParticleTex *pTex;
	uvBuff->Lock(0, 0, (void**)&pTex, 0);
	for (int i = 0; i < PARTICLE_NUM; i++, pTex++)
	{
		pTex->tex = D3DXVECTOR2(0.0f, 0.0f);
	}
	uvBuff->Unlock();

	VERTEX_3D *pVertex;
	anotherVtx->Lock(0, 0, (void**)&pVertex, 0);
	pVertex[0].vtx = D3DXVECTOR3(-10.0f, 10.0f, 0.0f);
	pVertex[1].vtx = D3DXVECTOR3(10.0f, 10.0f, 0.0f);
	pVertex[2].vtx = D3DXVECTOR3(-10.0f, -10.0f, 0.0f);
	pVertex[3].vtx = D3DXVECTOR3(10.0f, -10.0f, 0.0f);
	pVertex[0].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[1].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[2].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[3].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	anotherVtx->Unlock();

	D3DVERTEXELEMENT9 elems[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		{1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
		{1, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
		{2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},
		D3DDECL_END()
	};
	pDevice->CreateVertexDeclaration(elems, &declare);

	WORD index[6] = { 0, 1, 2, 2, 1, 3 };
	pDevice->CreateIndexBuffer(sizeof(index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &indexBuff, NULL);
	void *p;
	indexBuff->Lock(0, 0, &p, 0);
	memcpy(p, index, sizeof(index));
	indexBuff->Unlock();

	D3DXCreateEffectFromFile(pDevice, "particle.fx", 0, 0, 0, 0, &effect, 0);
	D3DXCreateEffectFromFile(pDevice, "BlurFilter.fx", 0, 0, 0, 0, &blurFilter, 0);

	pDevice->CreateTexture(SCREEN_WIDTH,
		SCREEN_HEIGHT,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&colorTexture,
		NULL);
	colorTexture->GetSurfaceLevel(0, &colorSurface);

	pDevice->CreateTexture(SCREEN_WIDTH,
		SCREEN_HEIGHT,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&bloomTexture,
		NULL);
	bloomTexture->GetSurfaceLevel(0, &bloomSurface);

	for (int i = 0; i < 2; i++)
	{
		pDevice->CreateTexture(SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2,
			1,
			D3DUSAGE_RENDERTARGET,
			D3DFMT_X8R8G8B8,
			D3DPOOL_DEFAULT,
			&blurTexture[i],
			NULL);
		blurTexture[i]->GetSurfaceLevel(0, &blurSurface[i]);
	}

	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&screenBuff,
		NULL);

	VERTEX_2D *pScreen;
	screenBuff->Lock(0, 0, (void**)&pScreen, 0);
	pScreen[0].vtx = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pScreen[1].vtx = D3DXVECTOR3((float)SCREEN_WIDTH, 0.0f, 0.0f);
	pScreen[2].vtx = D3DXVECTOR3(0.0f, (float)SCREEN_HEIGHT, 0.0f);
	pScreen[3].vtx = D3DXVECTOR3((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f);

	float tmp = 0.0f;
	pScreen[0].tex = D3DXVECTOR2(0.0f + 0.5f / (float)SCREEN_WIDTH, 0.0f + 0.5f / (float)SCREEN_HEIGHT);
	pScreen[1].tex = D3DXVECTOR2(1.0f + 0.5f / (float)SCREEN_WIDTH, 0.0f + 0.5f / (float)SCREEN_HEIGHT);
	pScreen[2].tex = D3DXVECTOR2(0.0f + 0.5f / (float)SCREEN_WIDTH, 1.0f + 0.5f / (float)SCREEN_HEIGHT);
	pScreen[3].tex = D3DXVECTOR2(1.0f + 0.5f / (float)SCREEN_WIDTH, 1.0f + 0.5f / (float)SCREEN_HEIGHT);

	pScreen[0].rhw =
		pScreen[1].rhw =
		pScreen[2].rhw =
		pScreen[3].rhw = 1.0f;

	pScreen[0].diffuse =
		pScreen[1].diffuse =
		pScreen[2].diffuse =
		pScreen[3].diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	screenBuff->Unlock();

	float texelU = 1.0f / (SCREEN_WIDTH / 2.0f);
	float texelV = 1.0f / (SCREEN_HEIGHT / 2.0f);
	float u[5], v[5];
	for (int i = 0; i < 5; i++)
	{
		u[i] = texelU * (i + 1);
		v[i] = texelV * (i + 1);
	}

	blurFilter->SetFloatArray("texelU", u, 5);
	blurFilter->SetFloatArray("texelV", u, 5);

	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
}

/************************************
終了処理
*************************************/
void UninitParticle(void)
{
	SAFE_RELEASE(vtxBuff);
	SAFE_RELEASE(worldBuff);
	SAFE_RELEASE(uvBuff);
	SAFE_RELEASE(declare);
	SAFE_RELEASE(effect);
	SAFE_RELEASE(blurFilter);

	SAFE_RELEASE(colorTexture);
	SAFE_RELEASE(colorSurface);
	SAFE_RELEASE(bloomTexture);
	SAFE_RELEASE(bloomSurface);
	for (int i = 0; i < 2; i++)
	{
		SAFE_RELEASE(blurTexture[i]);
		SAFE_RELEASE(blurSurface[i]);
	}
}

/************************************
更新処理
*************************************/
void UpdateParticle(void)
{

}

/************************************
描画処理
*************************************/
#include "input.h"
void DrawParticle(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	static bool useEffect = true;
	static bool useBloom = false;

	pDevice->SetRenderState(D3DRS_LIGHTING, false);

	if (GetKeyboardTrigger(DIK_F1))
		useEffect = !useEffect;
	if (GetKeyboardTrigger(DIK_F2))
		useBloom = !useBloom;

	if (useEffect)
	{
		DrawColorAndBloom();

		if (useBloom)
		{
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			DrawColorAndBloom();

			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
	}
	else
	{
		LPDIRECT3DSURFACE9 oldSuf = NULL;
		pDevice->GetRenderTarget(0, &oldSuf);
		pDevice->SetRenderTarget(0, colorSurface);
		pDevice->SetRenderTarget(1, bloomSurface);

		pDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);

		DrawColorAndBloom();

		BeginDebugWindow("Color & Bloom");
		DebugDrawTexture(colorTexture, SCREEN_WIDTH / 5.0f, SCREEN_HEIGHT / 5.0f);
		DebugDrawTexture(bloomTexture, SCREEN_WIDTH / 5.0f, SCREEN_HEIGHT / 5.0f);
		EndDebugWindow("Color & Bloom");

		pDevice->SetRenderTarget(1, NULL);

		pDevice->SetStreamSource(0, screenBuff, 0, sizeof(VERTEX_2D));
		pDevice->SetFVF(FVF_VERTEX_2D);

		ResizeScreenBuff(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
		ProcessBlur();

		BeginDebugWindow("Blur");
		DebugDrawTexture(blurTexture[1], SCREEN_WIDTH / 5.0f, SCREEN_HEIGHT / 5.0f);
		DebugDrawTexture(blurTexture[1], SCREEN_WIDTH / 5.0f, SCREEN_HEIGHT / 5.0f);
		EndDebugWindow("Blur");

		pDevice->SetRenderTarget(0, oldSuf);
		SAFE_RELEASE(oldSuf);

		ResizeScreenBuff(SCREEN_WIDTH, SCREEN_HEIGHT);
		BlendBloom(useBloom);
	}

	BeginDebugWindow("Particle");

	if (useEffect)
		DebugText("Shader:OFF");
	else
		DebugText("Shader:ON");

	if (useBloom)
		DebugText("Bloom:ON");
	else
		DebugText("Bloom:OFF");

	static float threthold = 0.5f;
	DebugSliderFloat("Threthold", &threthold, 0.0f, 1.0f);
	effect->SetFloat("threthold", threthold);
	effect->CommitChanges();

	EndDebugWindow("Particle");
}

void DrawColorAndBloom()
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	pDevice->SetRenderState(D3DRS_ZENABLE, false);

	pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | PARTICLE_NUM);
	pDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1);
	pDevice->SetStreamSourceFreq(2, D3DSTREAMSOURCE_INSTANCEDATA | 1);

	pDevice->SetVertexDeclaration(declare);

	pDevice->SetTexture(0, texture);

	pDevice->SetStreamSource(0, vtxBuff, 0, sizeof(ParticleVertex));
	pDevice->SetStreamSource(1, worldBuff, 0, sizeof(Transform));
	pDevice->SetStreamSource(2, uvBuff, 0, sizeof(ParticleTex));
	pDevice->SetIndices(indexBuff);

	D3DXMATRIX  view, proj, invView;
	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &proj);

	D3DXMatrixInverse(&invView, NULL, &view);
	invView._41 = invView._42 = invView._43 = 0.0f;

	effect->SetMatrix("mtxView", &view);
	effect->SetMatrix("mtxProj", &proj);
	effect->SetMatrix("mtxInvView", &invView);

	effect->SetTechnique("tech");
	effect->Begin(0, 0);
	effect->BeginPass(0);

	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	//pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	effect->EndPass();
	effect->End();

	pDevice->SetStreamSourceFreq(0, 1);
	pDevice->SetStreamSourceFreq(1, 1);
	pDevice->SetStreamSourceFreq(2, 1);

	pDevice->SetRenderState(D3DRS_ZENABLE, true);
}

void ProcessBlur()
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	pDevice->SetRenderState(D3DRS_ZENABLE, false);

	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	D3DVIEWPORT9 oldViewPort, newViewPort;
	pDevice->GetViewport(&oldViewPort);
	newViewPort.Width = SCREEN_WIDTH / 2;
	newViewPort.Height = SCREEN_HEIGHT / 2;
	newViewPort.MaxZ = 1.0f;
	newViewPort.MinZ = 0.0f;
	newViewPort.X = 0;
	newViewPort.Y = 0;
	pDevice->SetViewport(&newViewPort);

	for (int i = 0; i < 2; i++)
	{
		pDevice->SetRenderTarget(0, blurSurface[i]);
		pDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 0, 0);
	}

	pDevice->SetRenderTarget(0, blurSurface[0]);
	pDevice->SetTexture(0, bloomTexture);
	blurFilter->SetTechnique("tech");
	blurFilter->Begin(0, 0);
	blurFilter->BeginPass(0);

	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, NUM_POLYGON);

	blurFilter->EndPass();
	blurFilter->End();

	for (int i = 0; i < 10; i++)
	{
		pDevice->SetRenderTarget(0, blurSurface[(i + 1) % 2]);
		pDevice->SetTexture(0, blurTexture[i % 2]);
		blurFilter->SetTechnique("tech");
		blurFilter->Begin(0, 0);
		blurFilter->BeginPass((i + 1) % 2);

		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, NUM_POLYGON);

		blurFilter->EndPass();
		blurFilter->End();
	}

	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	pDevice->SetViewport(&oldViewPort);

	ResizeScreenBuff(SCREEN_WIDTH, SCREEN_HEIGHT);
	pDevice->SetRenderTarget(0, bloomSurface);
	pDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 0, 0);
	pDevice->SetTexture(0, blurTexture[1]);

	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, NUM_POLYGON);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	BeginDebugWindow("Blend");
	DebugDrawTexture(bloomTexture, SCREEN_WIDTH / 5.0f, SCREEN_HEIGHT / 5.0f);
	EndDebugWindow("Blend");
}

void BlendBloom(bool useBloom)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	pDevice->SetTexture(0, colorTexture);
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, NUM_POLYGON);

	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	if (useBloom)
	{
		pDevice->SetTexture(0, bloomTexture);
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, NUM_POLYGON);
	}

	pDevice->SetRenderState(D3DRS_ZENABLE, true);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void ResizeScreenBuff(float width, float height)
{
	VERTEX_2D *pVtx;
	screenBuff->Lock(0, 0, (void**)&pVtx, 0);

	pVtx[0].vtx = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVtx[1].vtx = D3DXVECTOR3(width, 0.0f, 0.0f);
	pVtx[2].vtx = D3DXVECTOR3(0.0f, height, 0.0f);
	pVtx[3].vtx = D3DXVECTOR3(width, height, 0.0f);

	pVtx[0].tex = D3DXVECTOR2(0.0f + 0.5f / (float)width, 0.0f + 0.5f / (float)height);
	pVtx[1].tex = D3DXVECTOR2(1.0f + 0.5f / (float)width, 0.0f + 0.5f / (float)height);
	pVtx[2].tex = D3DXVECTOR2(0.0f + 0.5f / (float)width, 1.0f + 0.5f / (float)height);
	pVtx[3].tex = D3DXVECTOR2(1.0f + 0.5f / (float)width, 1.0f + 0.5f / (float)height);

	screenBuff->Unlock();
}