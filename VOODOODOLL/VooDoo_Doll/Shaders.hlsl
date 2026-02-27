struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	float3					texMat: packoffset(c4);
};

cbuffer cbMaterialInfo : register(b3)
{
	MATERIAL				gMaterial : packoffset(c0);
	uint					gnTexturesMask : packoffset(c4);
};

#include "Light.hlsl"

cbuffer cbDrawOptions : register(b5)
{
	int4 gvDrawOptions : packoffset(c0);
};


struct CB_TOOBJECTSPACE
{
	matrix		mtxToTexture;
	float4		f4Position;
};

cbuffer cbToLightSpace : register(b6)
{
	CB_TOOBJECTSPACE gcbToLightSpaces[MAX_SHADOW_LIGHTS];
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtPrevFrame : register(t11);
Texture2D gtxtIlluminationTexture : register(t13);
Texture2D gtxtzDepthTexture : register(t14);
Texture2D gtxtDepthTexture : register(t15);
Texture2D gtxtInput : register(t0);
Texture2DArray gtxtTextureArray : register(t1);
RWTexture2D<float4> gtxtRWOutput : register(u0);

SamplerState gssWrap : register(s0);


struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;

	//정점 위에서의 좌표계 3축
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;


	float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}


//정점 안에 들어있는 정보
//position normal tangent bitangent
//normal : 표면이 위로 향하고 있는 방향
//tangent : 표면에서 "가로 방향"
//bitangent : 표면에서 "세로 방향"
//이 세 개가 모이면 Tangent Space 좌표계가 만들어짐
//normal은 회전했는데 tangent가 원래 방향 그대로면 TBN 축이 서로 직교하지 않게 됨
//좌표계가 찌그러짐

//메쉬는 그냥 평면인데
//노멀맵을 쓰면 돌이 튀어나온 것처럼 보이게 빛이 다르게 반사되게 만듦
//노멀맵 텍스처 안에 저장된 노멀은
//월드 좌표 기준이 아님
//텍스처 기준 좌표(Tangent Space) 기준
//노멀맵의 RGB는 이렇게 해석됨
//R -> tangent 방향
//G -> bitangent 방향
//B -> normal 방향
//이 3축이 있어야 노멀맵의 벡터를 월드 공간으로 바꿀 수 있음.
//rgb는 각각 T, B, N 방향으로 얼마나 기울어졌는지를 의미함.


float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);

	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float3 normalW;
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		//이 행렬은 
		//tangent space -> world space로 변환하는 행렬
		//흐름을 그림으로 정리
		//노멀맵 텍스처 -> TBN 곱함 -> World Space Normal -> Lighting 계산
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}

	//조명은 여기서 쓰임.
	float4 cIllumination = Lighting(input.positionW, normalW);
	
	if (cColor.x == 1 && cColor.y == 1 && cColor.z == 1)
		discard;

	return lerp(cColor, cIllumination, 0.5f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
};


//VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input) 
//위 함수 설명 

//각 정점에 대해
//
//1️ 영향받는 뼈 찾고
//2️ 뼈 변형 행렬 만들고
//3️ weight로 섞어서
//4️ 정점 위치 / 노멀 변형
//5️ 화면 공간으로 보냄

//Offset * Transform
//=> 왜 이 순서인지 이해하려면
//정점은 원래 모델 공간 기준 위치
//근데 뼈는 자기 기준 좌표계
//그래서 기준이 다름
//그래서 바로 transform 못함
//현재 뼈 위치 x (정점을 뼈 기준으로 옮긴 것)


//=>Transform * Offset 
//이렇게 반대로 하면
//정점 아직 뼈 기준도 아닌 상태
//뼈 움직임 먼저 적용
//그 다음 뼈공간 이동
//결과 : 메쉬 찢어짐, 관절 터짐, 팔이 몸에서 분리됨, 스파게티 메쉬

//손 위치를 팔꿈치 기준으로 옮기고 
//=> 몸 기준에서 손 먼저 회전시키고, 팔꿈치 기준 이동
//결과 : 행렬 순서 변경에 따른 결과 변경 발생

//offset이 하는 일 
//Offset = BindPos 역행렬

//정점을 "뼈 기준 공간"으로 옮김
//정점 : 모델 기준 위치 => Offset 적용 => 정점 : 뼈 기준 위치
//=> 뼈 회전은 뼈 기준 원점 중심으로 회전
//Offset이 없으면
//뼈 공간으로 이동 안 한 채로 회전
//Offset이 없으면,
//안 움직이는 게 아니라, 잘못된 축 기준으로 망가져 움직인다


//뼈 기준 공간에 있는 정점 => Transform 적용 => 움직인 결과


//VS_STANDARD_OUTPUT output => 이거는 결과 저장용 구조체
VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	//이건 최종 정점 변형 행렬
	//아무 뼈 영향도 없는 상태고 0으로 초기화
	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;

	for (int i=0; i < MAX_VERTEX_INFLUENCES; ++i)
	{
		mtxVertexToBoneWorld += 
			//input.weights[i] => 각 뼈 영향 비율
			//예를 들면 [0.6, 0.3, 0.1, 0]
			//weight 곱해서 더하는 이유는 정점은 보통 여러 뼈 영향 받음
			//부드러운 관절 움직임 생성하기 위함이고 
			//이 과정이 없으면 로봇처럼 꺾임
			input.weights[i] * 
			mul(
				//gpmtxBoneOffsets => Bind Pose 역행렬
				//정점을 뼈 기준 좌표계로 이동
				//정점을 뼈 기준 공간으로 옮겨주는 행렬

				//input.indices[i] => 이 정점이 영향받는 뼈 번호
				//예를 들면 [2, 5, 7, 0]
				//뼈2 : 60%, 뼈5 : 30%, 뼈7 : 10%
				gpmtxBoneOffsets[input.indices[i]], 

				//현재 애니메이션에서 뼈 위치
				//CPU가 매 프레임 계산해서 GPU로 올림
				//예를 들면 팔 들어올림, 머리 회전
				gpmtxBoneTransforms[input.indices[i]]
			);
	}


	//월드좌표 위치
	//정점 실제 변형
	//메쉬 움직임을 위한 코드 : 정점 위치가 뼈 따라 변함
	//정점을 뼈 행렬로 변형함
	//여기서 모델이 실제로 휘어지고 움직임
	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;

	//노멀도 같이 변형
	//조명 계산 때문
	//정점만 움직이고 노멀 안 움직이면 빛 계산 깨짐
	output.normalW = mul(input.normal, (float3x3)mtxVertexToBoneWorld).xyz;

	//tangent/bitangent 동일 이유
	//노멀맵 조명 위해 필요
	output.tangentW = mul(input.tangent, (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)mtxVertexToBoneWorld).xyz;



	////표준 그래픽스 파이프라인
	//	World
	//	-> View
	//	-> Projection
	//	-> Screen


	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	//uv 전달
	//텍스처 샘플링용
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	return(output);
}


float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
	input.normalW = normalize(input.normalW);
	return(float4(input.normalW * 0.5f + 0.5f, 1.0f));
}
//===============================================================================================================

struct PS_DEPTH_OUTPUT
{
	float fzPosition : SV_Target;
	float fDepth : SV_Depth;
};

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_LIGHTING_OUTPUT input)
{
	PS_DEPTH_OUTPUT output;

	output.fzPosition = input.position.z;
	output.fDepth = input.position.z;

	return(output);
}

//==================================================================================================================
struct VS_SHADOW_MAP_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float4 uvs[MAX_SHADOW_LIGHTS] : TEXCOORD0;
};

VS_SHADOW_MAP_OUTPUT VSShadowMapShadow(VS_LIGHTING_INPUT input)
{
	VS_SHADOW_MAP_OUTPUT output = (VS_SHADOW_MAP_OUTPUT)0;

	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.positionW = positionW.xyz;
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;

	for (int i = 0; i < MAX_SHADOW_LIGHTS; ++i)
	{
		if (gcbToLightSpaces[i].f4Position.w != 0.0f)
			output.uvs[i] = mul(positionW, gcbToLightSpaces[i].mtxToTexture);
	}

	return(output);
}

float4 PSShadowMapShadow(VS_SHADOW_MAP_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uvs[0].xy);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uvs[0].xy);

	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	float4 cIllumination = shadowLighting(input.positionW, normalize(input.normalW), true, input.uvs);
	return(lerp(cColor, cIllumination, 0.5f));
}
//===================================================================================================================================
struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
	VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
	return float4(1.f,0.f,0.f,0.f);
}
//=========================================================================================
struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSSpriteAnimation(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;
	
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	
	if (texMat.z == 6 )//���� 
	{
		output.uv.x = (input.uv.x) / texMat.z + texMat.x;
		output.uv.y = input.uv.y / texMat.z + texMat.y;
	}
	else if (texMat.z == 4)//�ε� ��ƼŬ
	{
		output.uv.x = (input.uv.x) / texMat.z + texMat.x;
		output.uv.y = input.uv.y / (texMat.z*1.5f) + texMat.y;
	}
	else//�� ȭ��
		output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;

	if (texMat.z == 6)
	{
		if (cColor.x <= 0.05f&& cColor.y <= 0.05f && cColor.z <= 0.05f)
			discard;
		if (cColor.x > 0.25f && cColor.x <= 0.26f &&
			cColor.y > 0.25f && cColor.y <= 0.26f &&
			cColor.z > 0.25f && cColor.z <= 0.26f)
			discard;
	}
	else if (texMat.z == 1 || texMat.z == 4 || texMat.z == 3)
	{
		if(texMat.z == 3)
			cColor.a = 0.7f;

		if (cColor.x < 0.4f)
			discard;
	}

	return(cColor);
}



#define _WITH_2D_GAUSSIAN_BLUR
#define _WITH_GROUPSHARED_MEMORY

#ifdef _WITH_2D_GAUSSIAN_BLUR
groupshared float4 gf4GroupSharedCache[2 + 32 + 2][2 + 32 + 2];

static float gfGaussianBlurMask2D[5][5] = {
	{ 1.0f / 273.0f, 4.0f / 273.0f, 7.0f / 273.0f, 4.0f / 273.0f, 1.0f / 273.0f },
	{ 4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f },
	{ 7.0f / 273.0f, 26.0f / 273.0f, 41.0f / 273.0f, 26.0f / 273.0f, 7.0f / 273.0f },
	{ 4.0f / 273.0f, 16.0f / 273.0f, 26.0f / 273.0f, 16.0f / 273.0f, 4.0f / 273.0f },
	{ 1.0f / 273.0f, 4.0f / 273.0f, 7.0f / 273.0f, 4.0f / 273.0f, 1.0f / 273.0f }
};

#define MotionBlurStrength 5.1f // ��� ��� ����

[numthreads(32, 32, 1)]

void CSGaussian2DBlur(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
	if ((n3DispatchThreadID.x < 2) || (n3DispatchThreadID.x >= int(gtxtInput.Length.x - 2)) || (n3DispatchThreadID.y < 2) || (n3DispatchThreadID.y >= int(gtxtInput.Length.y - 2)))
	{
		gtxtRWOutput[n3DispatchThreadID.xy] = gtxtInput[n3DispatchThreadID.xy];
	}
	else
	{
		float4 f4Color = float4(0, 0, 0, 0);
		for (int i = -2; i <= 2; ++i)
		{
			for (int j = -2; j <= 2; ++j)
			{
				float2 offset = float2(i, j) * MotionBlurStrength;
				f4Color += gfGaussianBlurMask2D[i + 2][j + 2]/float(1.03) * gtxtInput[n3DispatchThreadID.xy + offset];
			}
		}

		gtxtRWOutput[n3DispatchThreadID.xy] = f4Color;
	}
}

#endif

Texture2D gtxtOutput : register(t1);


float4 PSTextureToFullScreen(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cEdgeColor = gtxtOutput.Sample(gssWrap, input.uv) * 1.25f;
	
	return(cEdgeColor);
}
float4 PSTextureToFull(VS_TEXTURED_OUTPUT input) : SV_Target
{
	//float4 cColor = gtxtInput.Sample(gssWrap, input.uv);
	float4 cEdgeColor = gtxtOutput.Sample(gssWrap, input.uv) * 1.25f;//gtxtPrevFrame
	//float4 cEdgeColor = gtxtPrevFrame.Sample(gssWrap, input.uv) * 1.25f;//gtxtPrevFrame

	//if (//texMat.x == 2&&
	//	/*cEdgeColor.x >= 0.5 &&*/ cEdgeColor.x == 1 &&
	//	/*cEdgeColor.y >= 0.5 &&*/ cEdgeColor.y == 1 &&
	//	/*cEdgeColor.z >= 0.5 &&*/ cEdgeColor.z == 1
	//	)//|| cEdgeColor.x==0&& cEdgeColor.y == 0 && cEdgeColor.z == 0 )
	//	discard;

	cEdgeColor.a = 0.7f;

	return(cEdgeColor);
	//return(cColor * cEdgeColor);
	//return(cColor + cEdgeColor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 


VS_TEXTURED_OUTPUT VSNewTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSNewTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID): SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssWrap, uvw);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID): SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssWrap, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

	return(cColor * cIllumination);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//



struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 color : SV_TARGET0;

	float4 cTexture : SV_TARGET1;
	float4 cIllumination : SV_TARGET2;
	float4 normal : SV_TARGET3;
	float zDepth : SV_TARGET4;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.cTexture = gtxtTextureArray.Sample(gssWrap, uvw);

	input.normalW = normalize(input.normalW);
	output.cIllumination = Lighting(input.positionW, input.normalW);

	output.color = output.cIllumination * output.cTexture;

	output.normal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);

	output.zDepth = input.position.z;

	return(output);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//
float4 VSPostProcessing(uint nVertexID : SV_VertexID): SV_POSITION
{
	if (nVertexID == 0)	return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 1)	return(float4(+1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 2)	return(float4(+1.0f, -1.0f, 0.0f, 1.0f));

	if (nVertexID == 3)	return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 4)	return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 5)	return(float4(-1.0f, -1.0f, 0.0f, 1.0f));

	return(float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION): SV_Target
{
	return(float4(0.0f, 0.0f, 0.0f, 1.0f));
}


///////////////////////////////////////////////////////////////////////////////
//
struct VS_SCREEN_RECT_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SCREEN_RECT_TEXTURED_OUTPUT VSScreenRectSamplingTextured(uint nVertexID : SV_VertexID)
{
	VS_SCREEN_RECT_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
							 
	else if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

float4 GetColorFromDepth(float fDepth)
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (fDepth >= 1.0f) cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.00625f) cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	else if (fDepth < 0.0125f) cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.025f) cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.05f) cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.075f) cColor = float4(0.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.1f) cColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
	else if (fDepth < 0.4f) cColor = float4(0.5f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.6f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.8f) cColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
	else if (fDepth < 0.9f) cColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
	else cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return(cColor);
}

float4 PSScreenRectSamplingTextured(VS_TEXTURED_OUTPUT input): SV_Target
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	switch (gvDrawOptions.x)
	{
		case 84: //'T'
		{
			cColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
			break;
		}
		case 89: //'Y'
		{
			cColor = gtxtIlluminationTexture.Sample(gssWrap, input.uv);
			break;
		}
		case 85: //'U'
		{
			cColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
			break;
		}
		case 73: //'I'
		{
			float fDepth = gtxtDepthTexture.Load(uint3((uint)input.position.x, (uint)input.position.y, 0));
			cColor = fDepth;
//			cColor = GetColorFromDepth(fDepth);
			break; 
		}
		case 79: //'O'
		{
			float fzDepth = gtxtzDepthTexture.Load(uint3((uint)input.position.x, (uint)input.position.y, 0));
			cColor = fzDepth;
//			cColor = GetColorFromDepth(fDepth);
			break;
		}
	}
	return(cColor);
}