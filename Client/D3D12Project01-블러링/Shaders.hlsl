struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxWorld : packoffset(c0);
	MATERIAL	gMaterial : packoffset(c4);
	uint		gnTexturesMask : packoffset(c8);
};

cbuffer cbFrameworkInfo : register(b3)
{
	float		gfCurrentTime : packoffset(c0.x);
	float		gfElapsedTime : packoffset(c0.y);
	uint		gnRenderMode : packoffset(c0.z);
};

cbuffer cbExplodeInfo :register(b5)
{
	uint time : packoffset(c0);
}

static matrix gmtxProjectToTexture = {
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f,-0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

Texture2D gtxtShadowMap:register(t26);
SamplerState gssShadowMap:register(s3);

cbuffer cbShadow:register(b8) {
	matrix gmtxShadowTransform:packoffset(c0);
};


#define DYNAMIC_TESSELLATION		0x10
#define DEBUG_TESSELLATION			0x20

#define LINEAR_FOG 1.0f
#define EXP_FOG 2.0f
#define EXP2_FOG 3.0f

cbuffer cbFog:register(b6)
{
	float4 gcFogColor;
	float4 gvFogParameter; //(Mode, Start, Range, Density)
}

float4 Fog(float4 cColor, float3 vPosition)
{
	float3 vCameraPosition = gvCameraPosition.xyz;
	float3 vPositionToCamera = vCameraPosition - vPosition;
	float fDistanceToCamera = length(vPositionToCamera);
	float fFogFactor = 0.f;
	if (gvFogParameter.x == LINEAR_FOG) {
		fFogFactor = (fDistanceToCamera - gvFogParameter.y) / gvFogParameter.z - 1.f;
	}
	else if (gvFogParameter.x == EXP_FOG) {
		fFogFactor = 1.f-(1 / exp(fDistanceToCamera * gvFogParameter.w));
	}
	else if (gvFogParameter.x == EXP2_FOG) {
		fFogFactor = 1.f - (1 / exp2(fDistanceToCamera * gvFogParameter.w));
	}
	fFogFactor = saturate(fFogFactor);
	float4 cColorByFog = lerp(cColor, gcFogColor, fFogFactor);
	return cColorByFog;
}

#include "Light.hlsl"

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

#define _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);
#else
Texture2D gtxtStandardTextures[7] : register(t6);
#endif

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
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxWorld);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
#else
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtStandardTextures[0].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtStandardTextures[1].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtStandardTextures[2].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtStandardTextures[3].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtStandardTextures[4].Sample(gssWrap, input.uv);
#endif

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3 normalW = input.normalW;
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW);
		cColor = lerp(cColor, cIllumination, 0.5f);
	}
	cColor = Fog(cColor, input.positionW);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3 positionW : POSITION1;
	float3	positionL : POSITION0;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.positionL = input.position;
	output.positionW = float3(5000.f, 5000.f, 5000.f);

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);
	cColor = Fog(cColor, input.positionW);
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D<float4> gtxtWaterBaseTexture : register(t21);
Texture2D<float4> gtxtWaterDetailTexture : register(t22);
Texture2D<float> gtxtWaterAlphaTexture : register(t23);

struct VS_SPRITE_TEXTURED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_SPRITE_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_SPRITE_TEXTURED_OUTPUT VSTextured(VS_SPRITE_TEXTURED_INPUT input)
{
	VS_SPRITE_TEXTURED_OUTPUT output;

	input.position.y += cos((input.position.x + input.position.z) / 20 + (gfCurrentTime * 4)) * 0.7f;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}


float4 PSTextured(VS_SPRITE_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	input.uv1.x += gfCurrentTime / 2;
	input.uv1.y -= gfCurrentTime / 2;
	input.uv0.x += gfCurrentTime / 200;
	input.uv0.y += gfCurrentTime / 200;

	float4 cBaseTexColor = gtxtWaterBaseTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtWaterDetailTexture.Sample(gssWrap, input.uv1);
	float fAlpha = gtxtWaterAlphaTexture.Sample(gssWrap, input.uv0);
	float4 cColor = saturate(lerp(cBaseTexColor, cDetailTexColor, fAlpha));

	cColor = Fog(cColor, input.positionW);
	return(cColor);
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


Texture2D<float4> gtxtTerrainBaseTexture : register(t1);
Texture2D<float4> gtxtTerrainDetailTexture : register(t2);
Texture2D<float> gtxtTerrainAlphaTexture : register(t3);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW: POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
	float fAlpha = gtxtTerrainAlphaTexture.Sample(gssWrap, input.uv0);

	float4 cColor = saturate(lerp(cBaseTexColor, cDetailTexColor, fAlpha));
	cColor = Fog(cColor, input.positionW);
	return(cColor);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


struct VS_TERRAIN_TESSELLATION_OUTPUT
{
	float3 position : POSITION;
	float3 positionW : POSITION1;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float4 shadowPosition :TEXCOORD2;
};

VS_TERRAIN_TESSELLATION_OUTPUT VSTerrainTessellation(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_TESSELLATION_OUTPUT output;

	output.position = input.position;
	output.positionW = (float3)(mul(float4(input.position, 1.0f), gmtxWorld));
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	//matrix shadowProject = mul(mul(gmtxWorld, gmtxShadowTransform), gmtxProjectToTexture);
	//output.shadowPosition = mul(float4(input.position, 1.f), shadowProject);
	return(output);
}

struct HS_TERRAIN_TESSELLATION_CONSTANT
{
	float fTessEdges[4] : SV_TessFactor;
	float fTessInsides[2] : SV_InsideTessFactor;
};

struct HS_TERRAIN_TESSELLATION_OUTPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct DS_TERRAIN_TESSELLATION_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float4 tessellation : TEXCOORD2;
};

void BernsteinCoeffcient5x5(float t, out float fBernstein[5])
{
	float tInv = 1.0f - t;
	fBernstein[0] = tInv * tInv * tInv * tInv;
	fBernstein[1] = 4.0f * t * tInv * tInv * tInv;
	fBernstein[2] = 6.0f * t * t * tInv * tInv;
	fBernstein[3] = 4.0f * t * t * t * tInv;
	fBernstein[4] = t * t * t * t;
}

float3 CubicBezierSum5x5(OutputPatch<HS_TERRAIN_TESSELLATION_OUTPUT, 25> patch, float uB[5], float vB[5])
{
	float3 f3Sum = float3(0.0f, 0.0f, 0.0f);
	f3Sum = vB[0] * (uB[0] * patch[0].position + uB[1] * patch[1].position + uB[2] * patch[2].position + uB[3] * patch[3].position + uB[4] * patch[4].position);
	f3Sum += vB[1] * (uB[0] * patch[5].position + uB[1] * patch[6].position + uB[2] * patch[7].position + uB[3] * patch[8].position + uB[4] * patch[9].position);
	f3Sum += vB[2] * (uB[0] * patch[10].position + uB[1] * patch[11].position + uB[2] * patch[12].position + uB[3] * patch[13].position + uB[4] * patch[14].position);
	f3Sum += vB[3] * (uB[0] * patch[15].position + uB[1] * patch[16].position + uB[2] * patch[17].position + uB[3] * patch[18].position + uB[4] * patch[19].position);
	f3Sum += vB[4] * (uB[0] * patch[20].position + uB[1] * patch[21].position + uB[2] * patch[22].position + uB[3] * patch[23].position + uB[4] * patch[24].position);

	return(f3Sum);
}

float CalculateTessFactor(float3 f3Position)
{
	float fDistToCamera = distance(f3Position, gvCameraPosition);
	float s = saturate((fDistToCamera - 10.0f) / (1000.0f - 10.0f));

	return(lerp(64.0f, 1.0f, s));
	//	return(pow(2, lerp(20.0f, 4.0f, s)));
}

[domain("quad")]
[partitioning("fractional_even")]
//[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(25)]
[patchconstantfunc("HSTerrainTessellationConstant")]
[maxtessfactor(64.0f)]
HS_TERRAIN_TESSELLATION_OUTPUT HSTerrainTessellation(InputPatch<VS_TERRAIN_TESSELLATION_OUTPUT, 25> input, uint i : SV_OutputControlPointID)
{
	HS_TERRAIN_TESSELLATION_OUTPUT output;

	output.position = input[i].position;
	output.color = input[i].color;
	output.uv0 = input[i].uv0;
	output.uv1 = input[i].uv1;

	return(output);
}

HS_TERRAIN_TESSELLATION_CONSTANT HSTerrainTessellationConstant(InputPatch<VS_TERRAIN_TESSELLATION_OUTPUT, 25> input)
{
	HS_TERRAIN_TESSELLATION_CONSTANT output;

	if (gnRenderMode & DYNAMIC_TESSELLATION)
	{
		float3 e0 = 0.5f * (input[0].positionW + input[4].positionW);
		float3 e1 = 0.5f * (input[0].positionW + input[20].positionW);
		float3 e2 = 0.5f * (input[4].positionW + input[24].positionW);
		float3 e3 = 0.5f * (input[20].positionW + input[24].positionW);

		output.fTessEdges[0] = CalculateTessFactor(e0);
		output.fTessEdges[1] = CalculateTessFactor(e1);
		output.fTessEdges[2] = CalculateTessFactor(e2);
		output.fTessEdges[3] = CalculateTessFactor(e3);

		float3 f3Sum = float3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < 25; i++) f3Sum += input[i].positionW;
		float3 f3Center = f3Sum / 25.0f;
		output.fTessInsides[0] = output.fTessInsides[1] = CalculateTessFactor(f3Center);
	}
	else
	{
		output.fTessEdges[0] = 20.0f;
		output.fTessEdges[1] = 20.0f;
		output.fTessEdges[2] = 20.0f;
		output.fTessEdges[3] = 20.0f;

		output.fTessInsides[0] = 20.0f;
		output.fTessInsides[1] = 20.0f;
	}

	return(output);
}

[domain("quad")]
DS_TERRAIN_TESSELLATION_OUTPUT DSTerrainTessellation(HS_TERRAIN_TESSELLATION_CONSTANT patchConstant, float2 uv : SV_DomainLocation, OutputPatch<HS_TERRAIN_TESSELLATION_OUTPUT, 25> patch)
{
	DS_TERRAIN_TESSELLATION_OUTPUT output = (DS_TERRAIN_TESSELLATION_OUTPUT)0;

	float uB[5], vB[5];
	BernsteinCoeffcient5x5(uv.x, uB);
	BernsteinCoeffcient5x5(uv.y, vB);

	output.color = lerp(lerp(patch[0].color, patch[4].color, uv.x), lerp(patch[20].color, patch[24].color, uv.x), uv.y);
	output.uv0 = lerp(lerp(patch[0].uv0, patch[4].uv0, uv.x), lerp(patch[20].uv0, patch[24].uv0, uv.x), uv.y);
	output.uv1 = lerp(lerp(patch[0].uv1, patch[4].uv1, uv.x), lerp(patch[20].uv1, patch[24].uv1, uv.x), uv.y);

	float3 position = CubicBezierSum5x5(patch, uB, vB);
	matrix mtxWorldViewProjection = mul(mul(gmtxWorld, gmtxView), gmtxProjection);
	output.position = mul(float4(position, 1.0f), mtxWorldViewProjection);
	output.positionW = position;

	output.tessellation = float4(patchConstant.fTessEdges[0], patchConstant.fTessEdges[1], patchConstant.fTessEdges[2], patchConstant.fTessEdges[3]);

	return(output);
}

float4 PSTerrainTessellation(DS_TERRAIN_TESSELLATION_OUTPUT input) : SV_TARGET
{
	//input.shadowPosition.xyz /= input.shadowPosition.w;
	//float fShadowFactor = 0.3f, fBias = 0.006f;
	//float fsDepth = gtxtShadowMap.Sample(gssClamp, input.shadowPosition.xy).r;
	//if (input.shadowPosition.z <= (fsDepth + fBias)) fShadowFactor = 1.f;
	//input.normalW = normalize(input.normalW);
	//float4 cIllumination = Lighting(input.positionW, input.normalW, fShadowFactor);

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);


	if (gnRenderMode & DEBUG_TESSELLATION)
	{
		if (input.tessellation.w <= 5.0f) cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
		else if (input.tessellation.w <= 10.0f) cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
		else if (input.tessellation.w <= 20.0f) cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
		else if (input.tessellation.w <= 30.0f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
		else if (input.tessellation.w <= 40.0f) cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
		else if (input.tessellation.w <= 50.0f) cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		else if (input.tessellation.w <= 55.0f) cColor = float4(0.2f, 0.2f, 0.72f, 1.0f);
		else if (input.tessellation.w <= 60.0f) cColor = float4(0.5f, 0.75f, 0.75f, 1.0f);
		else cColor = float4(0.87f, 0.17f, 1.0f, 1.0f);
	}
	else
	{
		float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
		float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
		float fAlpha = gtxtTerrainAlphaTexture.Sample(gssWrap, input.uv0);

		cColor = saturate(lerp(cBaseTexColor, cDetailTexColor, fAlpha));
	}
	cColor = Fog(cColor, input.positionW);
	return(cColor);
}





//정점 셰이더의 입력을 위한 구조체를 선언한다. 
struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다. 
struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW: POSITION;
	float4 color : COLOR;
};

//정점 셰이더를 정의한다. 
VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	//정점을 변환(월드 변환, 카메라 변환, 투영 변환)한다.
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	//입력되는 픽셀의 색상(래스터라이저 단계에서 보간하여 얻은 색상)을 그대로 출력한다. 
	output.color = input.color;

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	//입력되는 픽셀의 색상을 그대로 출력-병합 단계(렌더 타겟)로 출력한다. 
	float4 cColor = Fog(input.color,input.positionW);
	return(cColor);
}



Texture2D gtxtExplosionTexture: register(t0);

struct VS_EXPLOSION_INPUT
{
	float3 position: POSITION;
	float2 uv : TEXCOORD;
};

struct VS_EXPLOSION_OUTPUT
{
	float4 position: SV_POSITION;
	float3 positionW: POSITION;
	float2 uv : TEXCOORD;
};

VS_EXPLOSION_OUTPUT VSExplosion(VS_EXPLOSION_INPUT input)
{
	VS_EXPLOSION_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	input.uv.x = input.uv.x + (time % 4) * 64;
	input.uv.y = input.uv.y + (time / 4) * 64;
	output.uv = input.uv / 256;

	return output;
}

float4 PSExplosion(VS_EXPLOSION_OUTPUT input) :SV_TARGET
{
	float4 cColor = gtxtExplosionTexture.Sample(gssClamp,input.uv);
	if (cColor.a <= 0.3f) discard;  /*clip(cColor.a - 0.3f);*/
	cColor = Fog(cColor, input.positionW);
	return cColor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


cbuffer cbVelocity : register(b7)
{
	float bias;
}

static float gfWeights[11] = { 0.05f,0.05f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.05f,0.05f };

Texture2D gtxtRenderTarget : register(t24);
RWTexture2D<float4> gtxtRWOutput : register(u0);
Texture2D gtxtOutput : register(t25);

groupshared float4 gTextureCache[(256 + 2 * 5)];

[numthreads(256, 1, 1)]
void HorzBlurCS(int3 vGroupThreadID : SV_GroupThreadID, int3 vDispatchThreadID : SV_DispatchThreadID)
{

	if (vGroupThreadID.x < 5)
	{
		int x = max(vDispatchThreadID.x - 5, 0);
		gTextureCache[vGroupThreadID.x] = gtxtRenderTarget[int2(x, vDispatchThreadID.y)];
	}
	else if (vGroupThreadID.x >= 256 - 5)
	{
		int x = min(vDispatchThreadID.x + 5, gtxtRenderTarget.Length.x - 1);
		gTextureCache[vGroupThreadID.x + 2 * 5] = gtxtRenderTarget[int2(x, vDispatchThreadID.y)];
	}
	gTextureCache[vGroupThreadID.x + 5] = gtxtRenderTarget[min(vDispatchThreadID.xy, gtxtRenderTarget.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBlurredColor = float4(0, 0, 0, 0);

	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.x + 5 + i;
		if(i!=0)
			cBlurredColor += gfWeights[i + 5] * gTextureCache[k] / (1.0f + bias);
	}
	cBlurredColor += (gfWeights[5]+bias) * gTextureCache[vGroupThreadID.x + 5] / (1.0f + bias);


	gtxtRWOutput[vDispatchThreadID.xy] = cBlurredColor;
}

[numthreads(1, 256, 1)]
void VertBlurCS(int3 vGroupThreadID : SV_GroupThreadID, int3 vDispatchThreadID : SV_DispatchThreadID)
{
	if (vGroupThreadID.y < 5)
	{
		int y = max(vDispatchThreadID.y - 5, 0);
		gTextureCache[vGroupThreadID.y] = gtxtRenderTarget[int2(vDispatchThreadID.x, y)];
	}
	else if (vGroupThreadID.y >= 256 - 5)
	{
		int y = min(vDispatchThreadID.y + 5, gtxtRenderTarget.Length.y - 1);
		gTextureCache[vGroupThreadID.y + 2 * 5] = gtxtRenderTarget[int2(vDispatchThreadID.x, y)];
	}
	gTextureCache[vGroupThreadID.y + 5] = gtxtRenderTarget[min(vDispatchThreadID.xy, gtxtRenderTarget.Length.xy - 1)];

	GroupMemoryBarrierWithGroupSync();

	float4 cBlurredColor = float4(0, 0, 0, 0);
	for (int i = -5; i <= 5; ++i)
	{
		int k = vGroupThreadID.y + 5 + i;
		if (i != 0)
			cBlurredColor += gfWeights[i + 5] * gTextureCache[k] / (1.0f + bias);
	}
	cBlurredColor += (gfWeights[5] + bias) * gTextureCache[vGroupThreadID.y + 5] / (1.0f + bias);
	gtxtRWOutput[vDispatchThreadID.xy] = cBlurredColor;
}


struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToFullScreen(uint nVertexID : SV_VertexID)
{
	VS_TEXTURED_OUTPUT output;
	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

float4 PSTextureToFullScreen(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtOutput.Sample(gssClamp, input.uv);
	
	return(cColor);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//




