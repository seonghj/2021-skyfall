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
	uint					gnTexturesMask : packoffset(c8);
};


#include "Light.hlsl"

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


	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);

	//if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	//{
	//	float3 normalW = input.normalW;
	//	float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
	//	float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
	//	normalW = normalize(mul(vNormal, TBN));
	//	cIllumination = Lighting(input.positionW, normalW);
	//	cColor = lerp(cColor, cIllumination, 0.5f);
	//}
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
};

VS_WIREFRAME_OUTPUT VSWireFrame(VS_WIREFRAME_INPUT input)
{
	VS_WIREFRAME_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSWireFrame(VS_WIREFRAME_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);


	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);

	if (/*gnTexturesMask & MATERIAL_NORMAL_MAP*/true)
	{
		float3 normalW = input.normalW;
		//float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		//float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
		//normalW = normalize(mul(vNormal, TBN));
		//cIllumination = Lighting(input.positionW, normalW);
		//cColor = lerp(cColor, cIllumination, 0.5f);
	}

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

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSSkinnedAnimationWireFrame(VS_SKINNED_WIREFRAME_OUTPUT input) : SV_TARGET
{
	
	float4 cColor = gtxtTexture.Sample(gssWrap, input.uv);

	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);

	if (/*gnTexturesMask & MATERIAL_NORMAL_MAP*/true)
	{
		float3 normalW = input.normalW;
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		cIllumination = Lighting(input.positionW, normalW);
		cColor = lerp(cColor, cIllumination, 0.5f);
	}
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t1);
Texture2D gtxtTerrainDetailTexture : register(t2);


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
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gssWrap, input.uv1);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));
	//float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

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