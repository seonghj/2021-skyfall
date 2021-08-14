#pragma enable_d3d12_debug_symbols

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
	MATERIAL				gMaterial : packoffset(c4);
	float					gfHpPercent : packoffset(c8);
	//uint					gnTexturesMask : packoffset(c8);
};

cbuffer cbFrameworkInfo : register(b3)
{
	float		gfCurrentTime : packoffset(c0.x);
	float		gfElapsedTime : packoffset(c0.y);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define LINEAR_FOG 1.0f
#define EXP_FOG 2.0f
#define EXP2_FOG 3.0f

cbuffer cbFog:register(b5)
{
	float4 gcFogColor;
	float4 gvFogParameter; //(Mode, Start, Range, Density)
	float2 gvFogPos;
}

float4 Fog(float4 cColor, float3 vPosition)
{
	float3 vCameraPosition = gvCameraPosition.xyz;
	float3 vPositionToCamera = vCameraPosition - vPosition;
	float fDistanceToCamera = length(vPositionToCamera);
	float fFogFactor = 0.f;
	float fDistanceToFog = length(float2(vCameraPosition.x, vCameraPosition.z) - gvFogPos);
	if (gvFogParameter.x == LINEAR_FOG) {
		fFogFactor = (fDistanceToCamera - gvFogParameter.y - fDistanceToFog / 3) / (gvFogParameter.z + fDistanceToFog);
	}
	else if (gvFogParameter.x == EXP_FOG) {
		fFogFactor = 1.f - (1 / exp(fDistanceToCamera * gvFogParameter.w / fDistanceToFog));
	}
	else if (gvFogParameter.x == EXP2_FOG) {
		fFogFactor = 1.f - (1 / exp2(fDistanceToCamera * gvFogParameter.w));
	}
	fFogFactor = saturate(fFogFactor);
	float4 cColorByFog = lerp(cColor, gcFogColor, fFogFactor);
	return cColorByFog;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

static matrix gmtxProjectToTexture = {
	0.5f,0.0f,0.0f,0.0f,
	0.0f,-0.5f,0.0f,0.0f,
	0.0f,0.0f,1.0f,0.0f,
	0.5f,0.5f,0.0f,1.0f };

Texture2D gtxtShadowMap : register(t0);
SamplerState gssShadowMap : register(s2);
//SamplerComparisonState gssShadowMap : register(s2);
cbuffer cbShadow :register(b0)
{
	matrix gmtxShadowTransform : packoffset(c0);
	float gfBias : packoffset(c4);
};

struct VS_SHADOW_INPUT
{
	float3 position :POSITION;
};

struct VS_SHADOW_OUTPUT
{
	float4 position :SV_POSITION;
};

VS_SHADOW_OUTPUT VSShadow(VS_SHADOW_INPUT input)
{
	VS_SHADOW_OUTPUT output;
	float4 positionW = mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(positionW, gmtxView), gmtxProjection);

	return output;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);
Texture2D gtxtTexture : register(t6);

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

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

	cColor = Fog(cColor, input.positionW);
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_WIREFRAME_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_WIREFRAME_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
	float4 shadowPosition: TEXCOORD1;
};

VS_WIREFRAME_OUTPUT VSWireFrame(VS_WIREFRAME_INPUT input)
{
	VS_WIREFRAME_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	//output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.normalW = input.normal;
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	matrix shadowProject = mul(gmtxGameObject, gmtxShadowTransform);
	output.shadowPosition = mul(float4(input.position, 1.0f), shadowProject);

	return(output);
}

float4 PSWireFrame(VS_WIREFRAME_OUTPUT input) : SV_TARGET
{
	float3 shadowPosition = input.shadowPosition.xyz / input.shadowPosition.w;
	float fShadowFactor = 0.3f, fBias = 0.00006f;

	//float fDepth = shadowPosition.z;
	//float fPercentLit = gtxtShadowMap.SampleCmpLevelZero(gssShadowMap, shadowPosition.xy, fDepth).r;
	//if (shadowPosition.z <= (fPercentLit + fBias)) fShadowFactor = 1.f; //not shadow

	float fsDepth = gtxtShadowMap.Sample(gssShadowMap, shadowPosition.xy).r;
	if (shadowPosition.z <= (fsDepth + gfBias)) fShadowFactor = 1.f; //not shadow

	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

	float4 cNormalColor = float4(0.f,0.f,1.f, 1.f);


	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);

	if (/*gnTexturesMask & MATERIAL_NORMAL_MAP*/true)
	{
		float3 normalW = normalize(input.normalW);
		//float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		//float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] -> [-1, 1]
		//normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW, fShadowFactor);
		cColor = lerp(cColor, cIllumination, 0.5f);
		cColor = Fog(cColor, input.positionW);
	}
	//cColor =float4(0,0,1,1);
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

Texture2D gtxtTextureSkin : register(t7);
cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_WIREFRAME_INPUT
{
	float3 position : POSITION;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_SKINNED_WIREFRAME_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
	float4 shadowPosition: TEXCOORD1;
};

VS_SKINNED_WIREFRAME_OUTPUT VSSkinnedAnimationWireFrame(VS_SKINNED_WIREFRAME_INPUT input)
{
	VS_SKINNED_WIREFRAME_OUTPUT output;

	output.positionW = float3(0.0f, 0.0f, 0.0f);
	matrix mtxVertexToBoneWorld;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
		output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	}

	//output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
//	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);

	output.normalW = input.normal;
	//output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	
	matrix shadowProject = mul(gmtxGameObject, gmtxShadowTransform);
	output.shadowPosition = mul(float4(input.position, 1.0f), shadowProject);

	return(output);
}

float4 PSSkinnedAnimationWireFrame(VS_SKINNED_WIREFRAME_OUTPUT input) : SV_TARGET
{
	float3 shadowPosition = input.shadowPosition.xyz / input.shadowPosition.w;
	float fShadowFactor = 0.3f, fBias = 0.00006f;

	//float fDepth = shadowPosition.z;
	//float fPercentLit = gtxtShadowMap.SampleCmpLevelZero(gssShadowMap, shadowPosition.xy, fDepth).r;
	//if (shadowPosition.z <= (fPercentLit + fBias)) fShadowFactor = 1.f; //not shadow

	float fsDepth = gtxtShadowMap.Sample(gssShadowMap, shadowPosition.xy).r;
	if (shadowPosition.z <= (fsDepth + gfBias)) fShadowFactor = 1.f; //not shadow
	
	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

	float4 cNormalColor = float4(0.f,0.f, 0.f, 1.0f);

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);

	if (/*gnTexturesMask & MATERIAL_NORMAL_MAP*/true)
	{
		float3 normalW = normalize(input.normalW);
		//float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		//float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] -> [-1, 1]
		//normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW, fShadowFactor);
		cColor = lerp(cColor, cIllumination, 0.5f);
		cColor = Fog(cColor, input.positionW);
	}
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);
Texture2D gtxtWater : register(t8);
Texture2D gtxtWaterNormal : register(t9);

cbuffer cbTerrainInfo : register(b9)
{
	float		gfTime : packoffset(c0.x);
	bool		gbFalling : packoffset(c0.y);
};

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float3 normal :NORMAL;
	float3 tangent :TANGENT;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW: POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float3 normalW :NORMAL;
	float3 tangentW :TANGENT;
	float4 shadowPosition: TEXCOORD2;
	int nState : STATE;
};

static const int GROUND = 1;
static const int WATER = 2;


VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	if (output.positionW.y <= 87.f) {
		output.positionW.y = 85.f + 2 * cos(output.positionW.x /3 + output.positionW.z /2 + gfCurrentTime * 2);
		output.nState = WATER;
		float3 e1 = float3(1, 85.f + 2 * cos((output.positionW.x + 1) / 3 + output.positionW.z / 2 + gfCurrentTime * 2), 0);
		float3 e2 = float3(0, 85.f + 2 * cos((output.positionW.x) / 3 + (output.positionW.z + 1) / 2 + gfCurrentTime * 2), 1);
		output.normalW = mul(normalize(cross(e1, e2)), (float3x3)gmtxGameObject);
		output.tangentW = mul(normalize(e1), (float3x3)gmtxGameObject);
	}
	else {
		output.nState = GROUND;
		output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
		output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	}


	//const int2 arrange[10] = { int2(0,0),int2(1,0),int2(2,0),int2(0,1),int2(1,1),int2(2,1),int2(0,2),int2(1,2),int2(2,2),int2(-1,-1) };
	float2 pos = float2(output.positionW.x, output.positionW.z) % 2048;
	
	if (gbFalling) {		
		//output.nState |= STATE::FALL;
		float r = length(float2(1024, 1024) - pos);
		if (r < gfTime * 100) {
			output.positionW.y -= 50 * (gfTime - r) + 9.8f * (gfTime - r) * (gfTime - r);
		}
	}

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;
	

	


	//matrix shadowProject = mul(mul(mul(gmtxGameObject, gmtxLightView), gmtxLightProjection), gmtxProjectToTexture);
	//matrix shadowProject = mul(mul(gmtxGameObject, gmtxShadowTransform), gmtxProjectToTexture);
	output.shadowPosition = mul(float4(output.positionW, 1.0f), gmtxShadowTransform);

	//output.shadowPosition = mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxShadowTransform);

	return(output);
}


float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{

	float3 shadowPosition = input.shadowPosition.xyz / input.shadowPosition.w;
	float fShadowFactor = 0.0f, fBias = 0.046f;

	//float fDepth = shadowPosition.z;
	//float fPercentLit = gtxtShadowMap.SampleCmpLevelZero(gssShadowMap, shadowPosition.xy, fDepth).r;
	//if (shadowPosition.z <= (fPercentLit + fBias)) fShadowFactor = 1.f; //not shadow

	float fsDepth = gtxtShadowMap.Sample(gssShadowMap, shadowPosition.xy).r;

	float4 cColor;
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float3 normalW = normalize(input.normalW);
	float4 cIllumination;
	if ((!gbFalling || input.nState == WATER) && input.positionW.y <= 87.f) {
		float2 uv = input.positionW.xz/50;
		uv.x += gfCurrentTime / 10;
		uv.y -= gfCurrentTime / 10;
		float4 cWater = gtxtWater.Sample(gssWrap, uv);
		cNormalColor = gtxtWaterNormal.Sample(gssWrap, uv);
		cColor = cWater;
		if (shadowPosition.z <= (fsDepth + gfBias)) fShadowFactor = 0.f; //not shadow
		else fShadowFactor = 1.f;
		//cIllumination = float4(fShadowFactor, fShadowFactor, fShadowFactor, fShadowFactor);
	}
	else {
		float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
		float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
		cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
		//float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
		if (shadowPosition.z <= (fsDepth + gfBias)) fShadowFactor = 1.f; // not shadow
	}	

	if (input.nState == WATER && input.positionW.y <= 87.f)
	{
		float3 tangentW = normalize(input.tangentW - dot(input.tangentW, normalW) * normalW);
		float3 bitangentW = normalize(cross(normalW, tangentW));
		float3x3 TBN = float3x3(tangentW, bitangentW, normalW);
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] -> [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	cIllumination = Lighting(input.positionW, normalW, fShadowFactor);
	
	cColor = lerp(cColor, cIllumination, 0.5f);

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
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);
	gvFogPos;
	cColor = Fog(cColor, float3(1200.f, 1200.f, 1200.f));
	return(cColor);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


struct VS_BOUNDINGBOX_INPUT
{
	float3 position : POSITION;
};

struct VS_BOUNDINGBOX_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_BOUNDINGBOX_OUTPUT VSBoundingBox(VS_BOUNDINGBOX_INPUT input)
{
	VS_BOUNDINGBOX_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	return output;
}

float4 PSBoundingBox(VS_BOUNDINGBOX_OUTPUT input) :SV_TARGET
{
	float4 cColor = float4(0.f,0.f,1.f,1.f);
	return(cColor);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VS_HPBAR_INPUT
{
	float3 center : POSITION;
	float2 size : TEXCOORD;
};

VS_HPBAR_INPUT VSHPBar(VS_HPBAR_INPUT input)
{
	input.center = (float3)(mul(float4(input.center, 1.f), gmtxGameObject));
	return(input);
}

struct GS_HPBAR_GEOMETRY_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float2 uv : TEXCOORD;
	float percent : TEXCOORD1;
};

static float2 pf2UVs[4] = { float2(0.0f,1.0f), float2(0.0f,0.0f), float2(1.0f,1.0f), float2(1.0f,0.0f) };

[maxvertexcount(4)]
void GSHPBar(point VS_HPBAR_INPUT input[1], inout TriangleStream<GS_HPBAR_GEOMETRY_OUTPUT> outStream)
{
	float3 f3Up = float3(0.0f, 1.0f, 0.0f);
	float3 f3Look = normalize(gvCameraPosition - input[0].center.xyz);
	float3 f3Right = cross(f3Up, f3Look);
	float fHalfWidth = input[0].size.x * 0.5f;
	float fHalfHeight = input[0].size.y * 0.5f;

	float4 pf4Vertices[4];
	pf4Vertices[0] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[1] = float4(input[0].center.xyz + (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[2] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[3] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);


	GS_HPBAR_GEOMETRY_OUTPUT output;
	for (int i = 0; i < 4; i++)
	{
		output.positionW = pf4Vertices[i].xyz;
		output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
		output.uv = pf2UVs[i];
		output.percent = gfHpPercent;
		outStream.Append(output);
	}
}

float4 PSHPBar(GS_HPBAR_GEOMETRY_OUTPUT input) : SV_TARGET
{
	float4 cColor = float4(1,0,0,1);
	if (input.uv.x > input.percent) cColor = float4(0.3, 0.3, 0.3, 0.3);


	cColor = Fog(cColor, input.positionW);
	return(cColor);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

cbuffer cbUIInfo : register(b6)
{
	float		gfAlpha : packoffset(c0.x);
	float		gfPercentV : packoffset(c0.y);
	float		gfPercentH : packoffset(c0.z);

};

Texture2D gtxtUI : register(t3);

struct VS_UI_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float alpha : COLOR;
};

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float alpha : COLOR;
};

VS_UI_OUTPUT VSUI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;
	output.position = float4(input.position, 1.f);
	output.uv = float2(input.uv.x, 1 - input.uv.y);
	
	//output.alpha = input.alpha;
	return output;
}

float4 PSUI(VS_UI_OUTPUT input) :SV_TARGET
{
	float4 cColor = gtxtUI.Sample(gssWrap, input.uv);
	cColor.a = gfAlpha;
	if (1 - input.uv.y > gfPercentV)
		cColor = float4(0.3, 0.3, 0.3, 0.5);

	return cColor;
}