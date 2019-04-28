//=====================================
//
//�u���[�t�B���^�[����[BlurFilter.cpp]
//Author:GP12B332 21 ���ԗY��
//
//=====================================
#include "BlurFilter.h"

/**************************************
�}�N����`
***************************************/
#define EFFECTFILE_BLUR_PATH	"data/EFFECT/BlurFilter.fx"

/**************************************
�\���̒�`
***************************************/

/**************************************
�O���[�o���ϐ�
***************************************/

/**************************************
�R���X�g���N�^
***************************************/
BlurFilter::BlurFilter()
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	D3DXCreateEffectFromFile(pDevice, (LPSTR)EFFECTFILE_BLUR_PATH, 0, 0, 0, 0, &effect, 0);
	effect->GetParameterByName(texelU, "texelU");
	effect->GetParameterByName(texelV, "texelV");
	effect->SetTechnique("tech");
}

/**************************************
�f�X�g���N�^
***************************************/
BlurFilter::~BlurFilter()
{
	SAFE_RELEASE(effect);
}

/**************************************
�`�揈��
***************************************/
void BlurFilter::Draw(UINT pass)
{
	effect->Begin(0, 0);
	effect->BeginPass(pass);

	ScreenObject::Draw();

	effect->EndPass();
	effect->End();
}

/**************************************
�T�[�t�F�C�X�T�C�Y�Z�b�g����
***************************************/
void BlurFilter::SetSurfaceSize(float width, float height)
{
	float u[5], v[5];
	for (int i = 0; i < 5; i++)
	{
		u[i] = 1.0f / width * (i + 1);
		v[i] = 1.0f / height * (i + 1);
	}

	effect->SetFloatArray(texelU, u, 5);
	effect->SetFloatArray(texelV, v, 5);
	effect->CommitChanges();

	ScreenObject::Resize(width, height);
}