float4x4 mtxView;
float4x4 mtxProj;
float4x4 mtxInvView;
float threthold;

sampler s0 : register(s0);

struct VS_OUTPUT {
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
};

VS_OUTPUT vsMain(
	float3 pos : POSITION,
	float2 localUV : TEXCOORD0,
	float3 worldPos : TEXCOORD1,
	float3 worldRot : TEXCOORD2,
	float3 worldScl : TEXCOORD3,
	float2 texUV : TEXCOORD4
) {
	VS_OUTPUT Out;

	float4x4 mtxWorld = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
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

	Out.pos = mul(Out.pos, mtxView);
	Out.pos = mul(Out.pos, mtxProj);

	Out.uv = localUV + texUV;

	Out.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return Out;
}

struct PS_OUTPUT
{
	float4 color1 : COLOR0; //ÉJÉâÅ[èÓïÒ
	float4 color2 : COLOR1; //ãPìxèÓïÒ
};

PS_OUTPUT psMain(VS_OUTPUT In){
	PS_OUTPUT Out;
	Out.color1 = tex2D(s0, In.uv) * In.color;

	const float3 perception = float3(0.2126f, 0.7152f, 0.0722f);
	float brightness = dot(perception, float3(Out.color1.r, Out.color1.g, Out.color1.b));
	Out.color2 = lerp(float4(0.0f, 0.0f, 0.0f, 0.0f), Out.color1, step(threthold, brightness));
	return Out;
}

technique tech {
	pass p0 {
		VertexShader = compile vs_2_0 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
};