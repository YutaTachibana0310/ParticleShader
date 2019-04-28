//=====================================
//
//ブラーフィルターヘッダ[BlurFilter.h]
//Author:GP12B332 21 立花雄太
//
//=====================================
#ifndef _BLURFILTER_H_
#define _BLURFILTER_H_

#include "main.h"
#include "ScreenObject.h"

/**************************************
マクロ定義
***************************************/

/**************************************
クラス定義
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