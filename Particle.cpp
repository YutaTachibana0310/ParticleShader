#include "Particle.h"
#include "debugWindow.h"
#include "BlurFilter.h"

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

//パーティクルの単位頂点
static ParticleVertex vtx[4] =
{
	{ D3DXVECTOR3(-10.0f,  10.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0)},
	{ D3DXVECTOR3(10.0f,  10.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0) },
	{ D3DXVECTOR3(-10.0f, -10.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0) },
	{ D3DXVECTOR3(10.0f, -10.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0) },
};

//各種頂点バッファ
static LPDIRECT3DVERTEXBUFFER9 vtxBuff;		//単位頂点用
static LPDIRECT3DVERTEXBUFFER9 worldBuff;	//ワールド変換用
static LPDIRECT3DVERTEXBUFFER9 uvBuff;		//個別のテクスチャ

static LPDIRECT3DVERTEXBUFFER9 anotherVtx;	//テスト用に普通に描画する用

//頂点宣言
static LPDIRECT3DVERTEXDECLARATION9 declare;

//シェーダ
static LPD3DXEFFECT effect;

//インデックスバッファ
static LPDIRECT3DINDEXBUFFER9 indexBuff;

//テクスチャ
static LPDIRECT3DTEXTURE9 texture;

//レンダリングターゲット用のテクスチャ（インスタンシングには関係ない）
static LPDIRECT3DTEXTURE9 colorTexture, bloomTexture;
static LPDIRECT3DTEXTURE9 blurTexture[2];
static LPDIRECT3DSURFACE9 colorSurface, bloomSurface;
static LPDIRECT3DSURFACE9 blurSurface[2];

//最終的な描画用の頂点バッファ（インスタンシングには関係ない）
static LPDIRECT3DVERTEXBUFFER9 screenBuff;
static BlurFilter *blurFilter;

/************************************
初期化処理
*************************************/
void InitParticle(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	//テクスチャ読み込みして設定
	texture = CreateTextureFromFile((LPSTR)"bulletParticle.png", pDevice);
	pDevice->SetTexture(0, texture);

	//頂点バッファ作成
	pDevice->CreateVertexBuffer(sizeof(vtx), 0, 0, D3DPOOL_MANAGED, &vtxBuff, 0);
	pDevice->CreateVertexBuffer(sizeof(Transform) * PARTICLE_NUM, 0, 0, D3DPOOL_MANAGED, &worldBuff, 0);
	pDevice->CreateVertexBuffer(sizeof(ParticleTex) * PARTICLE_NUM, 0, 0, D3DPOOL_MANAGED, &uvBuff, 0);

	//普通描画用の頂点バッファ作成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4, D3DUSAGE_WRITEONLY, FVF_VERTEX_3D, D3DPOOL_MANAGED, &anotherVtx, 0);

	//単位頂点の中身を埋める
	ParticleVertex *pVtx;
	vtxBuff->Lock(0, 0, (void**)&pVtx, 0);
	memcpy(pVtx, vtx, sizeof(vtx));
	vtxBuff->Unlock();

	//頂点バッファ（ワールド変換用）の中身を埋める
	Transform *pTr;
	worldBuff->Lock(0, 0, (void**)&pTr, 0);
	for (int i = 0; i < PARTICLE_NUM; i++, pTr++)
	{
		pTr->pos = D3DXVECTOR3(RandomRangef(-50.0f, 50.0f), RandomRangef(-20.0f, 20.0f), RandomRangef(50.0f, 100.0f));
		pTr->rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		pTr->scl = D3DXVECTOR3(0.5f, 0.5f, 1.0f);
	}
	worldBuff->Unlock();

	//頂点バッファ（テクスチャ用）の中身を埋める
	ParticleTex *pTex;
	uvBuff->Lock(0, 0, (void**)&pTex, 0);
	for (int i = 0; i < PARTICLE_NUM; i++, pTex++)
	{
		pTex->tex = D3DXVECTOR2(0.0f, 0.0f);
	}
	uvBuff->Unlock();

	//普通の描画用の中身を埋める
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

	//頂点宣言を作成
	D3DVERTEXELEMENT9 elems[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},	//単位頂点（頂点座標）
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},	//単位頂点（テクスチャ座標）
		{1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},	//ワールド変換情報（ポジション）
		{1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},	//ワールド変換情報（ローテーション）
		{1, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},	//ワールド変換情報（スケール）
		{2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},	//個別のテクスチャ
		D3DDECL_END()
	};
	pDevice->CreateVertexDeclaration(elems, &declare);

	//インデックスバッファ作成
	WORD index[6] = { 0, 1, 2, 2, 1, 3 };
	pDevice->CreateIndexBuffer(sizeof(index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &indexBuff, NULL);
	void *p;
	//中身を埋める
	indexBuff->Lock(0, 0, &p, 0);
	memcpy(p, index, sizeof(index));
	indexBuff->Unlock();

	//シェーダ読み込み
	D3DXCreateEffectFromFile(pDevice, "particle.fx", 0, 0, 0, 0, &effect, 0);
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

	SAFE_RELEASE(colorTexture);
	SAFE_RELEASE(colorSurface);
	SAFE_RELEASE(bloomTexture);
	SAFE_RELEASE(bloomSurface);
	for (int i = 0; i < 2; i++)
	{
		SAFE_RELEASE(blurTexture[i]);
		SAFE_RELEASE(blurSurface[i]);
	}

	delete blurFilter;
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
	
	//デバッグ用入力判定
	if (GetKeyboardTrigger(DIK_F1))
		useEffect = !useEffect;
	if (GetKeyboardTrigger(DIK_F2))
		useBloom = !useBloom;

	//インスタンシング描画
	//if (useEffect)
	{
		DrawColorAndBloom();	//ここが一番重要

		//ここから下は加算合成なので直接は関係ない
		if (useBloom)
		{
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			DrawColorAndBloom();

			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
	}

	//デバッグ表示処理
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
/*****************************************************
インスタンシングをした描画処理
******************************************************/
void DrawColorAndBloom()
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	//Zバッファへ書き込まない
	pDevice->SetRenderState(D3DRS_ZENABLE, false);

	//ストリームソース周波数を設定
	pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | PARTICLE_NUM);
	pDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1);
	pDevice->SetStreamSourceFreq(2, D3DSTREAMSOURCE_INSTANCEDATA | 1);

	//頂点宣言設定
	pDevice->SetVertexDeclaration(declare);

	//テクスチャ設定
	pDevice->SetTexture(0, texture);

	//ストリームソース設定
	pDevice->SetStreamSource(0, vtxBuff, 0, sizeof(ParticleVertex));
	pDevice->SetStreamSource(1, worldBuff, 0, sizeof(Transform));
	pDevice->SetStreamSource(2, uvBuff, 0, sizeof(ParticleTex));
	pDevice->SetIndices(indexBuff);

	//ビュー行列、プロジェクション行列、ビュー逆行列
	D3DXMATRIX  view, proj, invView;

	//ビュー、プロジェクション行列を取得
	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &proj);

	//ビルボード用に逆行列を計算
	D3DXMatrixInverse(&invView, NULL, &view);
	invView._41 = invView._42 = invView._43 = 0.0f;

	//シェーダに行列を設定
	effect->SetMatrix("mtxView", &view);
	effect->SetMatrix("mtxProj", &proj);
	effect->SetMatrix("mtxInvView", &invView);

	//シェーダによる描画開始
	effect->SetTechnique("tech");
	effect->Begin(0, 0);
	effect->BeginPass(0);

	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

	effect->EndPass();
	effect->End();

	//描画が終わったのでストリームソースを元に戻す
	pDevice->SetStreamSourceFreq(0, 1);
	pDevice->SetStreamSourceFreq(1, 1);
	pDevice->SetStreamSourceFreq(2, 1);

	pDevice->SetRenderState(D3DRS_ZENABLE, true);
}
