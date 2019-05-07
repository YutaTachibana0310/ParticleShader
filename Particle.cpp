#include "Particle.h"
#include "debugWindow.h"
#include "BlurFilter.h"

/************************************
�}�N����`
*************************************/
#define PARTICLE_NUM	(20)

/************************************
�v���g�^�C�v�錾
*************************************/
void DrawColorAndBloom();
void ProcessBlur();
void BlendBloom(bool useBloom);
void ResizeScreenBuff(float width, float height);

/************************************
�O���[�o���ϐ�
*************************************/
static Transform trasnform;
static ParticleTex uv;

//�p�[�e�B�N���̒P�ʒ��_
static ParticleVertex vtx[4] =
{
	{ D3DXVECTOR3(-10.0f,  10.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0)},
	{ D3DXVECTOR3(10.0f,  10.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0) },
	{ D3DXVECTOR3(-10.0f, -10.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0) },
	{ D3DXVECTOR3(10.0f, -10.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0) },
};

//�e�풸�_�o�b�t�@
static LPDIRECT3DVERTEXBUFFER9 vtxBuff;		//�P�ʒ��_�p
static LPDIRECT3DVERTEXBUFFER9 worldBuff;	//���[���h�ϊ��p
static LPDIRECT3DVERTEXBUFFER9 uvBuff;		//�ʂ̃e�N�X�`��

static LPDIRECT3DVERTEXBUFFER9 anotherVtx;	//�e�X�g�p�ɕ��ʂɕ`�悷��p

//���_�錾
static LPDIRECT3DVERTEXDECLARATION9 declare;

//�V�F�[�_
static LPD3DXEFFECT effect;

//�C���f�b�N�X�o�b�t�@
static LPDIRECT3DINDEXBUFFER9 indexBuff;

//�e�N�X�`��
static LPDIRECT3DTEXTURE9 texture;

//�����_�����O�^�[�Q�b�g�p�̃e�N�X�`���i�C���X�^���V���O�ɂ͊֌W�Ȃ��j
static LPDIRECT3DTEXTURE9 colorTexture, bloomTexture;
static LPDIRECT3DTEXTURE9 blurTexture[2];
static LPDIRECT3DSURFACE9 colorSurface, bloomSurface;
static LPDIRECT3DSURFACE9 blurSurface[2];

//�ŏI�I�ȕ`��p�̒��_�o�b�t�@�i�C���X�^���V���O�ɂ͊֌W�Ȃ��j
static LPDIRECT3DVERTEXBUFFER9 screenBuff;
static BlurFilter *blurFilter;

/************************************
����������
*************************************/
void InitParticle(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	//�e�N�X�`���ǂݍ��݂��Đݒ�
	texture = CreateTextureFromFile((LPSTR)"bulletParticle.png", pDevice);
	pDevice->SetTexture(0, texture);

	//���_�o�b�t�@�쐬
	pDevice->CreateVertexBuffer(sizeof(vtx), 0, 0, D3DPOOL_MANAGED, &vtxBuff, 0);
	pDevice->CreateVertexBuffer(sizeof(Transform) * PARTICLE_NUM, 0, 0, D3DPOOL_MANAGED, &worldBuff, 0);
	pDevice->CreateVertexBuffer(sizeof(ParticleTex) * PARTICLE_NUM, 0, 0, D3DPOOL_MANAGED, &uvBuff, 0);

	//���ʕ`��p�̒��_�o�b�t�@�쐬
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4, D3DUSAGE_WRITEONLY, FVF_VERTEX_3D, D3DPOOL_MANAGED, &anotherVtx, 0);

	//�P�ʒ��_�̒��g�𖄂߂�
	ParticleVertex *pVtx;
	vtxBuff->Lock(0, 0, (void**)&pVtx, 0);
	memcpy(pVtx, vtx, sizeof(vtx));
	vtxBuff->Unlock();

	//���_�o�b�t�@�i���[���h�ϊ��p�j�̒��g�𖄂߂�
	Transform *pTr;
	worldBuff->Lock(0, 0, (void**)&pTr, 0);
	for (int i = 0; i < PARTICLE_NUM; i++, pTr++)
	{
		pTr->pos = D3DXVECTOR3(RandomRangef(-50.0f, 50.0f), RandomRangef(-20.0f, 20.0f), RandomRangef(50.0f, 100.0f));
		pTr->rot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		pTr->scl = D3DXVECTOR3(0.5f, 0.5f, 1.0f);
	}
	worldBuff->Unlock();

	//���_�o�b�t�@�i�e�N�X�`���p�j�̒��g�𖄂߂�
	ParticleTex *pTex;
	uvBuff->Lock(0, 0, (void**)&pTex, 0);
	for (int i = 0; i < PARTICLE_NUM; i++, pTex++)
	{
		pTex->tex = D3DXVECTOR2(0.0f, 0.0f);
	}
	uvBuff->Unlock();

	//���ʂ̕`��p�̒��g�𖄂߂�
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

	//���_�錾���쐬
	D3DVERTEXELEMENT9 elems[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},	//�P�ʒ��_�i���_���W�j
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},	//�P�ʒ��_�i�e�N�X�`�����W�j
		{1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},	//���[���h�ϊ����i�|�W�V�����j
		{1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},	//���[���h�ϊ����i���[�e�[�V�����j
		{1, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},	//���[���h�ϊ����i�X�P�[���j
		{2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},	//�ʂ̃e�N�X�`��
		D3DDECL_END()
	};
	pDevice->CreateVertexDeclaration(elems, &declare);

	//�C���f�b�N�X�o�b�t�@�쐬
	WORD index[6] = { 0, 1, 2, 2, 1, 3 };
	pDevice->CreateIndexBuffer(sizeof(index), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &indexBuff, NULL);
	void *p;
	//���g�𖄂߂�
	indexBuff->Lock(0, 0, &p, 0);
	memcpy(p, index, sizeof(index));
	indexBuff->Unlock();

	//�V�F�[�_�ǂݍ���
	D3DXCreateEffectFromFile(pDevice, "particle.fx", 0, 0, 0, 0, &effect, 0);
}

/************************************
�I������
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
�X�V����
*************************************/
void UpdateParticle(void)
{

}

/************************************
�`�揈��
*************************************/
#include "input.h"
void DrawParticle(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	static bool useEffect = true;
	static bool useBloom = false;

	pDevice->SetRenderState(D3DRS_LIGHTING, false);
	
	//�f�o�b�O�p���͔���
	if (GetKeyboardTrigger(DIK_F1))
		useEffect = !useEffect;
	if (GetKeyboardTrigger(DIK_F2))
		useBloom = !useBloom;

	//�C���X�^���V���O�`��
	//if (useEffect)
	{
		DrawColorAndBloom();	//��������ԏd�v

		//�������牺�͉��Z�����Ȃ̂Œ��ڂ͊֌W�Ȃ�
		if (useBloom)
		{
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			DrawColorAndBloom();

			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
	}

	//�f�o�b�O�\������
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
�C���X�^���V���O�������`�揈��
******************************************************/
void DrawColorAndBloom()
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	//Z�o�b�t�@�֏������܂Ȃ�
	pDevice->SetRenderState(D3DRS_ZENABLE, false);

	//�X�g���[���\�[�X���g����ݒ�
	pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | PARTICLE_NUM);
	pDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1);
	pDevice->SetStreamSourceFreq(2, D3DSTREAMSOURCE_INSTANCEDATA | 1);

	//���_�錾�ݒ�
	pDevice->SetVertexDeclaration(declare);

	//�e�N�X�`���ݒ�
	pDevice->SetTexture(0, texture);

	//�X�g���[���\�[�X�ݒ�
	pDevice->SetStreamSource(0, vtxBuff, 0, sizeof(ParticleVertex));
	pDevice->SetStreamSource(1, worldBuff, 0, sizeof(Transform));
	pDevice->SetStreamSource(2, uvBuff, 0, sizeof(ParticleTex));
	pDevice->SetIndices(indexBuff);

	//�r���[�s��A�v���W�F�N�V�����s��A�r���[�t�s��
	D3DXMATRIX  view, proj, invView;

	//�r���[�A�v���W�F�N�V�����s����擾
	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &proj);

	//�r���{�[�h�p�ɋt�s����v�Z
	D3DXMatrixInverse(&invView, NULL, &view);
	invView._41 = invView._42 = invView._43 = 0.0f;

	//�V�F�[�_�ɍs���ݒ�
	effect->SetMatrix("mtxView", &view);
	effect->SetMatrix("mtxProj", &proj);
	effect->SetMatrix("mtxInvView", &invView);

	//�V�F�[�_�ɂ��`��J�n
	effect->SetTechnique("tech");
	effect->Begin(0, 0);
	effect->BeginPass(0);

	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

	effect->EndPass();
	effect->End();

	//�`�悪�I������̂ŃX�g���[���\�[�X�����ɖ߂�
	pDevice->SetStreamSourceFreq(0, 1);
	pDevice->SetStreamSourceFreq(1, 1);
	pDevice->SetStreamSourceFreq(2, 1);

	pDevice->SetRenderState(D3DRS_ZENABLE, true);
}
