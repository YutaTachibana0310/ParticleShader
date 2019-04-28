//=====================================
//
//�u���[�t�B���^�[�w�b�_[BlurFilter.h]
//Author:GP12B332 21 ���ԗY��
//
//=====================================
#ifndef _BLURFILTER_H_
#define _BLURFILTER_H_

#include "main.h"
#include "ScreenObject.h"

/**************************************
�}�N����`
***************************************/

/**************************************
�N���X��`
***************************************/
class BlurFilter : ScreenObject {
public:
	BlurFilter();
	~BlurFilter();
	void Draw(UINT pass);
	void SetSurfaceSize(float width, float height);

private:
	LPD3DXEFFECT effect;
	D3DXHANDLE texelU, texelV;
};

#endif