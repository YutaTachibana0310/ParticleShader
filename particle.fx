float4x4 mtxView;		//ビュー行列
float4x4 mtxProj;		//プロジェクション行列
float4x4 mtxInvView;	//ビュー逆行列
float threthold;		//明度を抽出するしきい値（インスタンシングには関係ない）

/**************************************************************
テクスチャサンプラー（0番にSetTextureしたテクスチャを使用する
***************************************************************/
sampler s0 : register(s0);

/**************************************************************
頂点シェーダ出力構造体
***************************************************************/
struct VS_OUTPUT {
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
};

/**************************************************************
頂点シェーダ
***************************************************************/
VS_OUTPUT vsMain(
	float3 pos : POSITION,
	float2 localUV : TEXCOORD0,
	float3 worldPos : TEXCOORD1,
	float3 worldRot : TEXCOORD2,
	float3 worldScl : TEXCOORD3,
	float2 texUV : TEXCOORD4
) {
	VS_OUTPUT Out;

	Out.pos = float4(pos, 1.0f);

	//scale
	Out.pos.x = Out.pos.x * worldScl.x;
	Out.pos.y = Out.pos.y * worldScl.y;
	Out.pos.z = Out.pos.z * worldScl.z;

	//rotX
	float4 tmpPos = Out.pos;
	Out.pos.y = tmpPos.y * cos(worldRot.x) - tmpPos.z * sin(worldRot.x);
	Out.pos.z = tmpPos.y * sin(worldRot.x) + tmpPos.z * sin(worldRot.x);

	//rotY
	tmpPos = Out.pos;
	Out.pos.x = tmpPos.x * cos(worldRot.y) + tmpPos.z * sin(worldRot.y);
	Out.pos.z = tmpPos.x * -sin(worldRot.y) + tmpPos.z * cos(worldRot.y);

	//rotZ
	tmpPos = Out.pos;
	Out.pos.x = tmpPos.x * cos(worldRot.z) - tmpPos.y * sin(worldRot.z);
	Out.pos.y = tmpPos.x * sin(worldRot.z) + tmpPos.y * cos(worldRot.z);

	//InvView
	Out.pos = mul(Out.pos, mtxInvView);

	//Translate
	Out.pos.x = Out.pos.x + worldPos.x;
	Out.pos.y = Out.pos.y + worldPos.y;
	Out.pos.z = Out.pos.z + worldPos.z;

	//view & projection
	Out.pos = mul(Out.pos, mtxView);
	Out.pos = mul(Out.pos, mtxProj);

	//texUV
	Out.uv = localUV + texUV;

	//Color
	Out.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return Out;
}

/**************************************************************
ピクセルシェーダ出力構造体
***************************************************************/
struct PS_OUTPUT
{
	float4 color1 : COLOR0; //カラー情報
	float4 color2 : COLOR1; //輝度情報
};

/**************************************************************
ピクセルシェーダ
***************************************************************/
PS_OUTPUT psMain(VS_OUTPUT In)
{
	PS_OUTPUT Out;

	//テクスチャからカラーを取得
	Out.color1 = tex2D(s0, In.uv) * In.color;

	//輝度を計算(インスタンシングには関係なし)
	const float3 perception = float3(0.2126f, 0.7152f, 0.0722f);
	float brightness = dot(perception, float3(Out.color1.r, Out.color1.g, Out.color1.b));

	//輝度がしきい値以下のテクセルは黒にする（インスタンシングには関係ない）
	Out.color2 = lerp(float4(0.0f, 0.0f, 0.0f, 0.0f), Out.color1, step(threthold, brightness));

	return Out;
}

/**************************************************************
テクニック
***************************************************************/
technique tech {
	pass p0 {
		VertexShader = compile vs_2_0 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
};