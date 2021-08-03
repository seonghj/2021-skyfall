//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include "Scene.h"
#include <iostream>

CGameObject** CMap::m_ppObjectInstance;
int CMap::m_nObjectInstance;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;

	if (m_nTextures > 0)
	{
		m_pGraphicsSrvRootArgumentInfos = new ROOTARGUMENTINFO[m_nTextures];
		m_pComputeSrvRootArgumentInfos = new ROOTARGUMENTINFO[m_nTextures];
		m_pComputeUavRootArgumentInfos = new ROOTARGUMENTINFO[m_nTextures];
		m_ppd3dTextureUploadBuffers = new ID3D12Resource*[m_nTextures];
		m_ppd3dTextures = new ID3D12Resource*[m_nTextures];

		m_pd3dSrvGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_pd3dSrvGpuDescriptorHandles[i].ptr = NULL;

		m_pd3dUavGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_pd3dUavGpuDescriptorHandles[i].ptr = NULL;
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}

	if (m_pGraphicsSrvRootArgumentInfos)
	{
		delete[] m_pGraphicsSrvRootArgumentInfos;
	}

	if (m_pComputeSrvRootArgumentInfos)
	{
		delete[] m_pComputeSrvRootArgumentInfos;
	}

	if (m_pComputeUavRootArgumentInfos)
	{
		delete[] m_pComputeUavRootArgumentInfos;
	}
	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetSrvGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetUavGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGpuDescriptorHandle)
{
	m_pd3dUavGpuDescriptorHandles[nIndex] = d3dUavGpuDescriptorHandle;
}

void CTexture::SetGraphicsSrvRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pGraphicsSrvRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pGraphicsSrvRootArgumentInfos[nIndex].m_d3dGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetComputeSrvRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pComputeSrvRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pComputeSrvRootArgumentInfos[nIndex].m_d3dGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetComputeUavRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGpuDescriptorHandle)
{
	m_pComputeUavRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pComputeUavRootArgumentInfos[nIndex].m_d3dGpuDescriptorHandle = d3dUavGpuDescriptorHandle;
}

void CTexture::SetGraphicsSrvRootArgument(int nIndex, UINT nRootParameterIndex, int nGpuHandleIndex)
{
	m_pGraphicsSrvRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pGraphicsSrvRootArgumentInfos[nIndex].m_d3dGpuDescriptorHandle = m_pd3dSrvGpuDescriptorHandles[nGpuHandleIndex];
}

void CTexture::SetComputeSrvRootArgument(int nIndex, UINT nRootParameterIndex, int nGpuHandleIndex)
{
	m_pComputeSrvRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pComputeSrvRootArgumentInfos[nIndex].m_d3dGpuDescriptorHandle = m_pd3dSrvGpuDescriptorHandles[nGpuHandleIndex];
}

void CTexture::SetComputeUavRootArgument(int nIndex, UINT nRootParameterIndex, int nGpuHandleIndex)
{
	m_pComputeUavRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pComputeUavRootArgumentInfos[nIndex].m_d3dGpuDescriptorHandle = m_pd3dUavGpuDescriptorHandles[nGpuHandleIndex];
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateGraphicsShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pGraphicsSrvRootArgumentInfos[0].m_nRootParameterIndex, m_pGraphicsSrvRootArgumentInfos[0].m_d3dGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pGraphicsSrvRootArgumentInfos[i].m_nRootParameterIndex, m_pGraphicsSrvRootArgumentInfos[i].m_d3dGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pGraphicsSrvRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pGraphicsSrvRootArgumentInfos[nIndex].m_d3dGpuDescriptorHandle);
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex, bool bIsDDSFile)
{
	if (bIsDDSFile)
		m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &(m_ppd3dTextureUploadBuffers[nIndex]), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	else
		m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromWICFile(pd3dDevice, pd3dCommandList, pszFileName, &(m_ppd3dTextureUploadBuffers[nIndex]), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, nWidth, nHeight, 1, 1, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}

CMaterial::CMaterial()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial(int nTextures)
{
	m_nTextures = nTextures;

	m_ppTextures = new CTexture*[m_nTextures];
	m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
	for (int i = 0; i < m_nTextures; i++) m_ppTextures[i] = NULL;
	for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';
}

CMaterial::~CMaterial()
{
	if (m_pShader) m_pShader->Release();

	if (m_nTextures > 0)
	{
		for (int i = 0; i < m_nTextures; i++) if (m_ppTextures[i]) m_ppTextures[i]->Release();
		delete[] m_ppTextures;

		if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;
	}
}

void CMaterial::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial::SetTexture(CTexture *pTexture, UINT nTexture) 
{ 
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->Release();
	m_ppTextures[nTexture] = pTexture; 
	if (m_ppTextures[nTexture]) m_ppTextures[nTexture]->AddRef();  
}

void CMaterial::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->ReleaseUploadBuffers();
	}
}

CShader *CMaterial::m_pWireFrameShader = NULL;
CShader *CMaterial::m_pSkinnedAnimationWireFrameShader = NULL;
CShader* CMaterial::m_pBoundingBoxShader = NULL;
void CMaterial::PrepareShaders(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_pWireFrameShader = new CWireFrameShader();
	m_pWireFrameShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pWireFrameShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pSkinnedAnimationWireFrameShader = new CSkinnedAnimationWireFrameShader();
	m_pSkinnedAnimationWireFrameShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pSkinnedAnimationWireFrameShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pBoundingBoxShader = new CBoundingBoxShader();
	m_pBoundingBoxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pBoundingBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CMaterial::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4AmbientColor, 16);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4DiffuseColor, 20);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4SpecularColor, 24);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4EmissiveColor, 28);

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nType, 32);

	for (int i = 0; i < m_nTextures; i++)
	{
		if (m_ppTextures[i]) m_ppTextures[i]->UpdateShaderVariable(pd3dCommandList, 0);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationCurve::CAnimationCurve(int nKeys)
{
	m_nKeys = nKeys;
	m_pfKeyTimes = new float[nKeys];
	m_pfKeyValues = new float[nKeys];
}

CAnimationCurve::~CAnimationCurve()
{
	if (m_pfKeyTimes) delete[] m_pfKeyTimes;
	if (m_pfKeyValues) delete[] m_pfKeyValues;
}

float CAnimationCurve::GetValueByLinearInterpolation(float fPosition)
{
	for (int k = 0; k < (m_nKeys - 1); k++)
	{
		if ((m_pfKeyTimes[k] <= fPosition) && (fPosition < m_pfKeyTimes[k+1]))
		{
			float t = (fPosition - m_pfKeyTimes[k]) / (m_pfKeyTimes[k+1] - m_pfKeyTimes[k]);
			return(m_pfKeyValues[k] * (1.0f - t) + m_pfKeyValues[k+1] * t);
		}
	}
	return(m_pfKeyValues[m_nKeys-1]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationLayer::CAnimationLayer() 
{ 
}

CAnimationLayer::~CAnimationLayer()
{
	for (int i = 0; i < m_nAnimatedBoneFrames; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (m_ppAnimationCurves[i][j]) delete m_ppAnimationCurves[i][j];
		}
	}
	if (m_ppAnimationCurves) delete[] m_ppAnimationCurves;

	if (m_ppAnimatedBoneFrameCaches) delete[] m_ppAnimatedBoneFrameCaches;
}

void CAnimationLayer::LoadAnimationKeyValues(int nBoneFrame, int nCurve, FILE *pInFile)
{
	int nAnimationKeys = ::ReadIntegerFromFile(pInFile);

	m_ppAnimationCurves[nBoneFrame][nCurve] = new CAnimationCurve(nAnimationKeys);

	::fread(m_ppAnimationCurves[nBoneFrame][nCurve]->m_pfKeyTimes, sizeof(float), nAnimationKeys, pInFile);
	::fread(m_ppAnimationCurves[nBoneFrame][nCurve]->m_pfKeyValues, sizeof(float), nAnimationKeys, pInFile);
}

XMFLOAT4X4 CAnimationLayer::GetSRT(int nBoneFrame, float fPosition, float fTrackWeight) 
{ 
	CGameObject *pBoneFrame = m_ppAnimatedBoneFrameCaches[nBoneFrame];
	XMFLOAT3 xmf3S = pBoneFrame->m_xmf3Scale;
	XMFLOAT3 xmf3R = pBoneFrame->m_xmf3Rotation;
	XMFLOAT3 xmf3T = pBoneFrame->m_xmf3Translation;

	if (m_ppAnimationCurves[nBoneFrame][0]) xmf3T.x = m_ppAnimationCurves[nBoneFrame][0]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][1]) xmf3T.y = m_ppAnimationCurves[nBoneFrame][1]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][2]) xmf3T.z = m_ppAnimationCurves[nBoneFrame][2]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][3]) xmf3R.x = m_ppAnimationCurves[nBoneFrame][3]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][4]) xmf3R.y = m_ppAnimationCurves[nBoneFrame][4]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][5]) xmf3R.z = m_ppAnimationCurves[nBoneFrame][5]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][6]) xmf3S.x = m_ppAnimationCurves[nBoneFrame][6]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][7]) xmf3S.y = m_ppAnimationCurves[nBoneFrame][7]->GetValueByLinearInterpolation(fPosition);
	if (m_ppAnimationCurves[nBoneFrame][8]) xmf3S.z = m_ppAnimationCurves[nBoneFrame][8]->GetValueByLinearInterpolation(fPosition);

	float fWeight = m_fWeight * fTrackWeight;
	XMMATRIX S = XMMatrixScaling(xmf3S.x * fWeight, xmf3S.y * fWeight, xmf3S.z * fWeight);
//	XMMATRIX R = XMMatrixRotationRollPitchYaw(xmf3R.x * fWeight, xmf3R.y * fWeight, xmf3R.z * fWeight);
	XMMATRIX R = XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(xmf3R.x * fWeight), XMMatrixRotationY(xmf3R.y * fWeight)), XMMatrixRotationZ(xmf3R.z * fWeight));
	XMMATRIX T = XMMatrixTranslation(xmf3T.x * fWeight, xmf3T.y * fWeight, xmf3T.z * fWeight);

	XMFLOAT4X4 xmf4x4Transform;
	XMStoreFloat4x4(&xmf4x4Transform, XMMatrixMultiply(XMMatrixMultiply(S, R), T));

	return(xmf4x4Transform);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSet::CAnimationSet(float fStartTime, float fEndTime, char* pstrName, int nType)
{
	m_fStartTime = fStartTime;
	m_fEndTime = fEndTime;
	m_fLength = fEndTime - fStartTime;
	m_nType = nType;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);
}

CAnimationSet::~CAnimationSet()
{
	if (m_pAnimationLayers) delete[] m_pAnimationLayers;

	if (m_pCallbackKeys) delete[] m_pCallbackKeys;
	if (m_pAnimationCallbackHandler) delete m_pAnimationCallbackHandler;
}

void CAnimationSet::HandleCallback()
{
	if (m_pAnimationCallbackHandler)
	{
		for (int i = 0; i < m_nCallbackKeys; i++)
		{
			if (::IsEqual(m_pCallbackKeys[i].m_fTime, m_fPosition, ANIMATION_CALLBACK_EPSILON))
			{
				if (m_pCallbackKeys[i].m_pCallbackData) m_pAnimationCallbackHandler->HandleCallback(m_pCallbackKeys[i].m_pCallbackData, m_fPosition);
				break;
			}
		}
	}
}

void CAnimationSet::SetPosition(float fElapsedPosition)
{
	switch (m_nType)
	{
		case ANIMATION_TYPE_LOOP:
		{
			m_fPosition += fElapsedPosition;
			if (m_fPosition >= m_fLength) m_fPosition = 0.0f;
			//m_fPosition = fmod(fTrackPosition, m_pfKeyFrameTimes[m_nKeyFrames-1]); // m_fPosition = fTrackPosition - int(fTrackPosition / m_pfKeyFrameTimes[m_nKeyFrames-1]) * m_pfKeyFrameTimes[m_nKeyFrames-1];
			//m_fPosition = fmod(fTrackPosition, m_fLength); //if (m_fPosition < 0) m_fPosition += m_fLength;
			//m_fPosition = fTrackPosition - int(fTrackPosition / m_fLength) * m_fLength;
			break;
		}
		case ANIMATION_TYPE_ONCE:
			if (m_fPosition < m_fLength)
				m_fPosition += fElapsedPosition;
			break;
		case ANIMATION_TYPE_PINGPONG:
			break;
	}
}

void CAnimationSet::Animate(float fElapsedPosition, float fTrackWeight)
{
	SetPosition(fElapsedPosition);

	for (int i = 0; i < m_nAnimationLayers; i++)
	{
		for (int j = 0; j < m_pAnimationLayers[i].m_nAnimatedBoneFrames; j++) 
		{
			m_pAnimationLayers[i].m_ppAnimatedBoneFrameCaches[j]->m_xmf4x4ToParent = m_pAnimationLayers[i].GetSRT(j, m_fPosition, fTrackWeight);
		}
	}
}

void CAnimationSet::SetCallbackKeys(int nCallbackKeys)
{
	m_nCallbackKeys = nCallbackKeys;
	m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSet::SetCallbackKey(int nKeyIndex, float fKeyTime, void *pData)
{
	m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationSet::SetAnimationCallbackHandler(CAnimationCallbackHandler *pCallbackHandler)
{
	m_pAnimationCallbackHandler = pCallbackHandler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_ppAnimationSets = new CAnimationSet*[nAnimationSets];
	for (int i = 0; i < m_nAnimationSets; i++) m_ppAnimationSets[i] = NULL;
}

CAnimationSets::~CAnimationSets()
{
	for (int i = 0; i < m_nAnimationSets; i++) if (m_ppAnimationSets[i]) delete m_ppAnimationSets[i];
	if (m_ppAnimationSets) delete[] m_ppAnimationSets;
}

void CAnimationSets::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	m_ppAnimationSets[nAnimationSet]->m_nCallbackKeys = nCallbackKeys;
	m_ppAnimationSets[nAnimationSet]->m_pCallbackKeys = new CALLBACKKEY[nCallbackKeys];
}

void CAnimationSets::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, void *pData)
{
	m_ppAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_fTime = fKeyTime;
	m_ppAnimationSets[nAnimationSet]->m_pCallbackKeys[nKeyIndex].m_pCallbackData = pData;
}

void CAnimationSets::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	m_ppAnimationSets[nAnimationSet]->SetAnimationCallbackHandler(pCallbackHandler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAnimationController::CAnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nAnimationTracks, CLoadedModelInfo *pModel)
{
	m_nAnimationTracks = nAnimationTracks;
    m_pAnimationTracks = new CAnimationTrack[nAnimationTracks];

	if (nAnimationTracks > 0) {
		m_pAnimationSets = pModel->m_pAnimationSets;
		m_pAnimationSets->AddRef();
	}
	m_nSkinnedMeshes = pModel->m_nSkinnedMeshes;
	m_ppSkinnedMeshes = new CSkinnedMesh*[m_nSkinnedMeshes];
	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i] = pModel->m_ppSkinnedMeshes[i];

	m_ppd3dcbSkinningBoneTransforms = new ID3D12Resource*[m_nSkinnedMeshes];
	m_ppcbxmf4x4MappedSkinningBoneTransforms = new XMFLOAT4X4*[m_nSkinnedMeshes];

	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Map(0, NULL, (void **)&m_ppcbxmf4x4MappedSkinningBoneTransforms[i]);
	}
}

CAnimationController::~CAnimationController()
{
	if (m_pAnimationTracks) delete[] m_pAnimationTracks;

	if (m_pAnimationSets) m_pAnimationSets->Release();

	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppd3dcbSkinningBoneTransforms[i]->Unmap(0, NULL);
		m_ppd3dcbSkinningBoneTransforms[i]->Release();
	}
	if (m_ppd3dcbSkinningBoneTransforms) delete[] m_ppd3dcbSkinningBoneTransforms;
	if (m_ppcbxmf4x4MappedSkinningBoneTransforms) delete[] m_ppcbxmf4x4MappedSkinningBoneTransforms;

	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CAnimationController::SetCallbackKeys(int nAnimationSet, int nCallbackKeys)
{
	if (m_pAnimationSets) m_pAnimationSets->SetCallbackKeys(nAnimationSet, nCallbackKeys);
}

void CAnimationController::SetCallbackKey(int nAnimationSet, int nKeyIndex, float fKeyTime, void *pData)
{
	if (m_pAnimationSets) m_pAnimationSets->SetCallbackKey(nAnimationSet, nKeyIndex, fKeyTime, pData);
}

void CAnimationController::SetAnimationCallbackHandler(int nAnimationSet, CAnimationCallbackHandler *pCallbackHandler)
{
	if (m_pAnimationSets) m_pAnimationSets->SetAnimationCallbackHandler(nAnimationSet, pCallbackHandler);
}

void CAnimationController::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetAnimationSet(nAnimationSet);
}

void CAnimationController::SetTrackEnable(int nAnimationTrack, bool bEnable)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetEnable(bEnable);
}

void CAnimationController::SetTrackPosition(int nAnimationTrack, float fPosition)
{
	if (m_pAnimationTracks) {
		m_pAnimationTracks[nAnimationTrack].SetPosition(fPosition);
		m_pAnimationSets->m_ppAnimationSets[m_pAnimationTracks[nAnimationTrack].m_nAnimationSet]->m_fPosition = fPosition;
	}
}

void CAnimationController::SetTrackSpeed(int nAnimationTrack, float fSpeed)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetSpeed(fSpeed);
}

void CAnimationController::SetTrackWeight(int nAnimationTrack, float fWeight)
{
	if (m_pAnimationTracks) m_pAnimationTracks[nAnimationTrack].SetWeight(fWeight);
}

void CAnimationController::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int i = 0; i < m_nSkinnedMeshes; i++)
	{
		m_ppSkinnedMeshes[i]->m_pd3dcbSkinningBoneTransforms = m_ppd3dcbSkinningBoneTransforms[i];
		m_ppSkinnedMeshes[i]->m_pcbxmf4x4MappedSkinningBoneTransforms = m_ppcbxmf4x4MappedSkinningBoneTransforms[i];
	}
}

void CAnimationController::AdvanceTime(float fTimeElapsed, CGameObject *pRootGameObject) 
{
	m_fTime += fTimeElapsed; 
	if (m_pAnimationTracks)
	{
		for (int i = 0; i < m_nAnimationTracks; i++)
		{
			if (m_pAnimationTracks[i].m_bEnable)
			{
				m_pAnimationSets->m_ppAnimationSets[m_pAnimationTracks[i].m_nAnimationSet]->Animate(fTimeElapsed * m_pAnimationTracks[i].m_fSpeed, m_pAnimationTracks[i].m_fWeight);
			}
		}

		pRootGameObject->UpdateTransform(NULL);

		for (int k = 0; k < m_nAnimationTracks; k++)
		{
			if (m_pAnimationTracks[k].m_bEnable) m_pAnimationSets->m_ppAnimationSets[m_pAnimationTracks[k].m_nAnimationSet]->HandleCallback();
		}
	}
}
void CAnimationController::SetAllTrackDisable()
{
	for (int i = 0; i < m_nAnimationTracks; ++i)
	{
		SetTrackEnable(i, false);
	}
}

void CAnimationController::SetTrackType(int nAnimationTrack, int nType)
{
	if (m_pAnimationTracks)
		m_pAnimationSets->m_ppAnimationSets[m_pAnimationTracks[nAnimationTrack].m_nAnimationSet]->SetType(nType);
}

bool CAnimationController::IsTrackFinish(int nAnimationTrack)
{
	return 	m_pAnimationSets->m_ppAnimationSets[m_pAnimationTracks[nAnimationTrack].m_nAnimationSet]->m_fPosition >=
		m_pAnimationSets->m_ppAnimationSets[m_pAnimationTracks[nAnimationTrack].m_nAnimationSet]->m_fLength;
}

float CAnimationController::GetTrackPosition(int nAnimationTrack)
{
	return m_pAnimationSets->m_ppAnimationSets[m_pAnimationTracks[nAnimationTrack].m_nAnimationSet]->m_fPosition;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CLoadedModelInfo::~CLoadedModelInfo()
{
	if (m_ppSkinnedMeshes) delete[] m_ppSkinnedMeshes;
}

void CLoadedModelInfo::PrepareSkinning()
{
	int nSkinnedMesh = 0;
	m_ppSkinnedMeshes = new CSkinnedMesh*[m_nSkinnedMeshes];
	m_pModelRootObject->FindAndSetSkinnedMesh(m_ppSkinnedMeshes, &nSkinnedMesh);

	for (int i = 0; i < m_nSkinnedMeshes; i++) m_ppSkinnedMeshes[i]->PrepareSkinning(m_pModelRootObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject::CGameObject(int nMaterials) : CGameObject()
{
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0)
	{
		m_ppMaterials = new CMaterial*[m_nMaterials];
		for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;
	}
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;

	if (m_pSkinnedAnimationController) delete m_pSkinnedAnimationController;
}

void CGameObject::AddRef() 
{ 
	m_nReferences++; 

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CGameObject::Release() 
{ 
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this; 
}

void CGameObject::SetChild(CGameObject *pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

CGameObject* CGameObject::SetBBObject(CCubeMesh* pBoundingBox)
{
	CGameObject* pBBObj = new CGameObject();
	strcpy_s(pBBObj->m_pstrFrameName, "BoundingBox");
	pBBObj->SetMesh(pBoundingBox);
	pBBObj->SetBoundingBoxShader();
	SetChild(pBBObj);

	return pBBObj;
}

CGameObject* CGameObject::SetBBObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 Center, XMFLOAT3 Extents)
{
	CGameObject* pBBObj = new CGameObject();
	strcpy_s(pBBObj->m_pstrFrameName, "BoundingBox");

	CCubeMesh* pBoundingBox = new CCubeMesh(pd3dDevice, pd3dCommandList, Extents.x*2, Extents.y*2, Extents.z*2);
	pBoundingBox->m_xmf3AABBCenter = Center;
	pBoundingBox->m_xmf3AABBExtents = Extents;

	pBBObj->SetMesh(pBoundingBox);
	pBBObj->SetBoundingBoxShader();
	pBBObj->Move(Center, 1);
	pBBObj->m_pMesh->m_xmf3AABBCenter = Center;
	pBBObj->m_pMesh->m_xmf3AABBExtents = Extents;
	SetChild(pBBObj,true);

	return pBBObj;
}

bool CGameObject::isCollide(CGameObject* pObject)
{
	BoundingOrientedBox bb = BoundingOrientedBox(
		FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBCenter,
		FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBExtents,
		XMFLOAT4(0, 0, 0, 1));

	bb.Transform(bb, XMLoadFloat4x4(&m_xmf4x4World));

	BoundingOrientedBox bbObject = BoundingOrientedBox(
		pObject->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBCenter,
		pObject->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBExtents,
		XMFLOAT4(0, 0, 0, 1));
	bbObject.Transform(bbObject, XMLoadFloat4x4(&pObject->m_xmf4x4World));

	if (bb.Contains(bbObject) != DirectX::DISJOINT||bb.Intersects(bbObject)) {
		return true;
	}
	return false;
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader)
{
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	m_ppMaterials[0] = new CMaterial(0);
	m_ppMaterials[0]->SetShader(pShader);
}

void CGameObject::SetShader(int nMaterial, CShader *pShader)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void CGameObject::SetWireFrameShader()
{
	if (m_nMaterials < 1 || m_ppMaterials[0] == NULL) {		
		m_nMaterials = 1;
		m_ppMaterials = new CMaterial * [m_nMaterials];
		m_ppMaterials[0] = NULL;
		CMaterial* pMaterial = new CMaterial(0);

		pMaterial->SetWireFrameShader();
		SetMaterial(0, pMaterial);
	}
	else {
		m_ppMaterials[0]->SetWireFrameShader();

	}
}

void CGameObject::SetSkinnedAnimationWireFrameShader()
{
	if (m_nMaterials < 1 || m_ppMaterials[0] == NULL) {
		m_nMaterials = 1;
		m_ppMaterials = new CMaterial * [m_nMaterials];
		m_ppMaterials[0] = NULL;
		CMaterial* pMaterial = new CMaterial(0);

		pMaterial->SetSkinnedAnimationWireFrameShader();
		SetMaterial(0, pMaterial);
	}
	else {
		m_ppMaterials[0]->SetSkinnedAnimationWireFrameShader();
	}
}

void CGameObject::SetBoundingBoxShader()
{
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	m_ppMaterials[0] = NULL;
	CMaterial* pMaterial = new CMaterial(0);

	pMaterial->SetBoundingBoxShader();
	SetMaterial(0, pMaterial);
}

void CGameObject::SetMaterial(int nMaterial, CMaterial *pMaterial)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}

CSkinnedMesh *CGameObject::FindSkinnedMesh(char *pstrSkinnedMeshName)
{
	CSkinnedMesh *pSkinnedMesh = NULL;
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) 
	{
		pSkinnedMesh = (CSkinnedMesh *)m_pMesh;
		if(!strcmp(pSkinnedMesh->m_pstrMeshName, pstrSkinnedMeshName)) return(pSkinnedMesh);
	}

	if (m_pSibling) if (pSkinnedMesh = m_pSibling->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);
	if (m_pChild) if (pSkinnedMesh = m_pChild->FindSkinnedMesh(pstrSkinnedMeshName)) return(pSkinnedMesh);

	return(NULL);
}

void CGameObject::FindAndSetSkinnedMesh(CSkinnedMesh **ppSkinnedMeshes, int *pnSkinnedMesh)
{
	if (m_pMesh && (m_pMesh->GetType() & VERTEXT_BONE_INDEX_WEIGHT)) ppSkinnedMeshes[(*pnSkinnedMesh)++] = (CSkinnedMesh *)m_pMesh;

	if (m_pSibling) m_pSibling->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
	if (m_pChild) m_pChild->FindAndSetSkinnedMesh(ppSkinnedMeshes, pnSkinnedMesh);
}

CGameObject *CGameObject::FindFrame(char *pstrFrameName)
{
	CGameObject *pFrameObject = NULL;

	if (!strcmp(m_pstrFrameName, pstrFrameName)) return(this);

	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::SetActive(char *pstrFrameName, bool bActive)
{
	CGameObject *pFrameObject = FindFrame(pstrFrameName);
	if (pFrameObject) pFrameObject->m_bActive = bActive;
}

void CGameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CGameObject::SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackAnimationSet(nAnimationTrack, nAnimationSet);
}

void CGameObject::SetTrackAnimationPosition(int nAnimationTrack, float fPosition)
{
	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetTrackPosition(nAnimationTrack, fPosition);
}

void CGameObject::Animate(float fTimeElapsed)
{
	OnPrepareRender();

	if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->AdvanceTime(fTimeElapsed, this);

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (!strcmp(m_pstrFrameName, "BoundingBox") && !gbShowBoundingBox) {
		if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
		if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
		return;
	}

	if (m_bActive)
	{
		if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

		if (m_pMesh)
		{
			UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

			if (m_nMaterials > 0)
			{
				for (int i = 0; i < m_nMaterials; i++)
				{
					if (m_ppMaterials[i])
					{
						if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
						m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
					}
					m_pMesh->Render(pd3dCommandList, i);
				}
			}
		}
	}

	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}

void CGameObject::RenderShadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!strcmp(m_pstrFrameName, "BoundingBox")) {
		return;
	}

	if (m_bActive)
	{
		if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->UpdateShaderVariables(pd3dCommandList);

		if (m_pMesh)
		{
			UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
			if (m_nMaterials > 0)
			{
				for (int i = 0; i < m_nMaterials; i++)
				{
					if (m_ppMaterials[i])
					{
						if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
						m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
					}
					m_pMesh->Render(pd3dCommandList, i);
				}
			}
		}
	}

	if (m_pSibling) m_pSibling->RenderShadow(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->RenderShadow(pd3dCommandList, pCamera);
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
/*
	XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	if (!strcmp(m_pstrFrameName, "L_shoulder")) xmf4Color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &xmf4Color, 16);
*/
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial)
{
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (int i = 0; i < m_nMaterials; i++)
	{
		if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	}

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetParentLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4ToParent._31, m_xmf4x4ToParent._32, m_xmf4x4ToParent._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed,
		m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 +
		vDirection.z * fSpeed);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

#define _WITH_DEBUG_FRAME_HIERARCHY

UINT ReadUnsignedIntegerFromFile(FILE *pInFile)
{
	UINT nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(UINT), 1, pInFile); 
	return(nValue);
}

int ReadIntegerFromFile(FILE *pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile); 
	return(nValue);
}

float ReadFloatFromFile(FILE *pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile); 
	return(fValue);
}

int ReadStringFromFile(FILE *pInFile, char *pstrToken)
{
	int nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(int), 1, pInFile);
	if (nStrLength > 64 || nStrLength < 0)
		return -1;
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile); 
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

static bool isSkinDeformation = false;
CGameObject *CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameObject *pParent, FILE *pInFile, CShader *pShader, int *pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = ::ReadIntegerFromFile(pInFile);

	string str;
	CGameObject* pGameObject = new CGameObject();
	::ReadStringFromFile(pInFile, pstrToken);
	{
		str = pstrToken;
		int i = str.find("__");
		if (i != -1) {
			str = i > 0 ? str.substr(0, i) : str;
			//str = str.substr(0, i);
			::ZeroMemory(pstrToken, 64);
			memcpy(pstrToken, str.c_str(), str.length());
		}
		memcpy(pGameObject->m_pstrFrameName, pstrToken, strlen(pstrToken));
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("FrameName: %hs\n"), pGameObject->m_pstrFrameName);
		OutputDebugString(pstrDebug);
	}

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Transform>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(XMFLOAT4X4), 1, pInFile);

			nReads = (UINT)::fread(&pGameObject->m_xmf3Scale, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&pGameObject->m_xmf3Rotation, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&pGameObject->m_xmf3Translation, sizeof(XMFLOAT3), 1, pInFile);


			//XMMATRIX S = XMMatrixScaling(pGameObject->m_xmf3Scale.x, pGameObject->m_xmf3Scale.y, pGameObject->m_xmf3Scale.z);
			//XMMATRIX R = XMMatrixRotationRollPitchYaw(pGameObject->m_xmf3Rotation.x, pGameObject->m_xmf3Rotation.y, pGameObject->m_xmf3Rotation.z);
			////XMMATRIX R = XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(xmf3R.x * fWeight), XMMatrixRotationY(xmf3R.y * fWeight)), XMMatrixRotationZ(xmf3R.z * fWeight));
			//XMMATRIX T = XMMatrixTranslation(pGameObject->m_xmf3Translation.x, pGameObject->m_xmf3Translation.y, pGameObject->m_xmf3Translation.z);

			//XMFLOAT4X4 xmf4x4Transform;
			//XMStoreFloat4x4(&xmf4x4Transform, XMMatrixMultiply(XMMatrixMultiply(S, R), T));
			//pGameObject->UpdateTransform(&xmf4x4Transform);

		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			int i;
			for (i = 0; i < CMap::m_nObjectInstance; ++i) {
				if (!strcmp(pGameObject->m_pstrFrameName, CMap::m_ppObjectInstance[i]->m_pstrFrameName)) {
					CGameObject* pInst = CMap::m_ppObjectInstance[i];
					pGameObject->SetMesh(pInst->m_pMesh);
					SkipMeshFromFile(pInFile);
					break;
				}
			}
			if (i == CMap::m_nObjectInstance) {
				CStandardMesh* pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
				pMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);
				pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

				pGameObject->SetMesh(pMesh);
			}
			isSkinDeformation = false;
		}
		else if (!strcmp(pstrToken, "<SkinDeformations>:"))
		{
			if (pnSkinnedMeshes) (*pnSkinnedMeshes)++;

			CSkinnedMesh* pSkinnedMesh = new CSkinnedMesh(pd3dDevice, pd3dCommandList);
			pSkinnedMesh->LoadSkinDeformationsFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pSkinnedMesh->CreateShaderVariables(pd3dDevice, pd3dCommandList);

			::ReadStringFromFile(pInFile, pstrToken); //<Mesh>:
			if (!strcmp(pstrToken, "<Mesh>:")) pSkinnedMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);

			pGameObject->SetMesh(pSkinnedMesh);
			isSkinDeformation = true;

		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			TCHAR pstrDebug[256] = { 0 };
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
			if (isSkinDeformation) {
				//_stprintf_s(pstrDebug, 256, _T("[SkinnedAnim] (m_nMaterials: %d)\n"), (int)pGameObject->m_nMaterials);

				/**/pGameObject->SetSkinnedAnimationWireFrameShader();
			}
			else {
				//_stprintf_s(pstrDebug, 256, _T("[WireFrame] (m_nMaterials: %d)\n"), (int)pGameObject->m_nMaterials);

				/**/pGameObject->SetWireFrameShader();
			}
			//OutputDebugString(pstrDebug);

		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<Frame>:"))
					{
						CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader, pnSkinnedMeshes);
						if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
						TCHAR pstrDebug[256] = { 0 };
						_stprintf_s(pstrDebug, 256, _T("[%d] (Frame: %p) (Parent: %p)\n"), i, pChild, pGameObject);
						OutputDebugString(pstrDebug);
#endif
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			if (gbInstancing) {
				int i;
				for (i = 0; i < CMap::m_nObjectInstance; ++i) {
					if (!strcmp(CMap::m_ppObjectInstance[i]->m_pstrFrameName, str.c_str())) {
						break;
					}
				}
				if (i == CMap::m_nObjectInstance)
					CMap::m_ppObjectInstance[CMap::m_nObjectInstance++] = pGameObject;
			}
			break;
		}
	}
	return(pGameObject);
}


CGameObject* CGameObject::LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);
	
	char pstrToken[64] = { '\0' };
	CGameObject* pGameObject = NULL;

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>"))
			{
				for (; ; )
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<Frame>:"))
					{
						pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, NULL);
					}
					else if (!strcmp(pstrToken, "</Hierarchy>"))
					{
						break;
					}
				}
			}
		}
		else
		{
			break;
		}
	}
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pGameObject);
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader)
{
	char pstrToken[64] = { '\0' };

	int nMaterial = 0;
	BYTE nStrLength = 0;
	{
		for (int i = 0; i < CMap::m_nObjectInstance; ++i) {
			if (!strcmp(m_pstrFrameName, CMap::m_ppObjectInstance[i]->m_pstrFrameName)) {
				CGameObject* pObject = CMap::m_ppObjectInstance[i];
				m_nMaterials = pObject->m_nMaterials;
				m_ppMaterials = new CMaterial * [m_nMaterials];
				for (int j = 0; j < m_nMaterials; ++j) {
					m_ppMaterials[j] = NULL;
					SetMaterial(j, pObject->m_ppMaterials[j]);
				}
				SkipMaterialsFromFile(pInFile);
				return;
			}
		}		
	}

	UINT nReads = (UINT)::fread(&m_nMaterials, sizeof(int), 1, pInFile);
	
	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	CMaterial* pMaterial = NULL;
	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Material>:"))
		{
			nMaterial = ::ReadIntegerFromFile(pInFile);
			::ReadStringFromFile(pInFile, pstrToken);
		}
		else if (!strcmp(pstrToken, "<TextureProperties>:"))
		{
			
			for (int i = ::ReadIntegerFromFile(pInFile); i >0 ; --i)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Property>:"))
				{
					int n = ::ReadIntegerFromFile(pInFile);
					::ReadStringFromFile(pInFile, pstrToken);
					::ReadStringFromFile(pInFile, pstrToken);
					if (strcmp(pstrToken,"Null")) {

						::ReadStringFromFile(pInFile, pstrToken);
						if (!strcmp(pstrToken, "<Textures>:"))
						{
							int nTextures = ::ReadIntegerFromFile(pInFile);

							if (nTextures > 0) {
								pMaterial = new CMaterial(nTextures);

								for (int j = 0; j < nTextures; ++j)
								{
									::ReadStringFromFile(pInFile, pstrToken);
									if (!strcmp(pstrToken, "<Texture>:"))
									{
										int nTexture = ::ReadIntegerFromFile(pInFile);

										TCHAR pstrDebug[256] = { 0 };
										::ReadStringFromFile(pInFile, pstrToken); // Texture Name

										_stprintf_s(pstrDebug, 256, _T("Texture Name: %hs\n"), pstrToken);
										OutputDebugString(pstrDebug);

										CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
										if (::ReadIntegerFromFile(pInFile) == 0)
										{
											::ReadStringFromFile(pInFile, pstrToken); // File Name
											char pstrFilePath[64] = { '\0' };
											strcpy_s(pstrFilePath, 64, "Model/Textures/");
											strcpy_s(pstrFilePath + 15, 64 - 15, pstrToken);
											strcpy_s(pstrFilePath + 15 + strlen(pstrToken) - 3, 64 - 15 - strlen(pstrToken), "dds");

											wchar_t textureName[64];
											size_t nConverted = 0;

											mbstowcs_s(&nConverted, textureName, 64, pstrFilePath, _TRUNCATE);

											pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, textureName, 0);
											CScene::CreateShaderResourceViews(pd3dDevice, pTexture, false);
											pTexture->SetGraphicsSrvRootArgument(0, 3, 0);
											pMaterial->SetTexture(pTexture, j);

											_stprintf_s(pstrDebug, 256, _T("File Name: %hs\n"), pstrFilePath);
											OutputDebugString(pstrDebug);

										}
									}
								}

								SetMaterial(nMaterial, pMaterial);
							}
						}
					}
				}
				else if (!strcmp(pstrToken, "</TextureProperties>:"))
				{
					break;
				}
			}
		}
		else if (!strcmp(pstrToken, "<Lambert>:"))
		{
			//Ambient
			m_ppMaterials[0]->m_xmf4AmbientColor.x = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4AmbientColor.y = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4AmbientColor.z = ::ReadFloatFromFile(pInFile);
			//Diffuse
			m_ppMaterials[0]->m_xmf4DiffuseColor.x = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4DiffuseColor.y = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4DiffuseColor.z = ::ReadFloatFromFile(pInFile);
			//Emissive
			m_ppMaterials[0]->m_xmf4EmissiveColor.x = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4EmissiveColor.y = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4EmissiveColor.z = ::ReadFloatFromFile(pInFile);

			//Transparency Factor
			::ReadFloatFromFile(pInFile);

		}
		else if (!strcmp(pstrToken, "<Phong>:"))
		{
			//Ambient
			m_ppMaterials[0]->m_xmf4AmbientColor.x = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4AmbientColor.y = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4AmbientColor.z = ::ReadFloatFromFile(pInFile);
			//Diffuse
			m_ppMaterials[0]->m_xmf4DiffuseColor.x = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4DiffuseColor.y = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4DiffuseColor.z = ::ReadFloatFromFile(pInFile);
			//Specular
			m_ppMaterials[0]->m_xmf4SpecularColor.x = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4SpecularColor.y = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4SpecularColor.z = ::ReadFloatFromFile(pInFile);
			//Emissive
			m_ppMaterials[0]->m_xmf4EmissiveColor.x = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4EmissiveColor.y = ::ReadFloatFromFile(pInFile);
			m_ppMaterials[0]->m_xmf4EmissiveColor.z = ::ReadFloatFromFile(pInFile);

			//Transparency Factor
			::ReadFloatFromFile(pInFile);
			//Shininess
			m_ppMaterials[0]->m_fSpecularHighlight = ::ReadFloatFromFile(pInFile);
			//Reflection Factor
			m_ppMaterials[0]->m_fGlossyReflection = ::ReadFloatFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}
}

void CGameObject::SkipFrameHierarchyFromFile(FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Transform>:"))
		{
			XMFLOAT4X4 a;
			XMFLOAT3 b;
			nReads = (UINT)::fread(&a, sizeof(XMFLOAT4X4), 1, pInFile);

			nReads = (UINT)::fread(&b, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&b, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&b, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Mesh>:"))
		{
			SkipMeshFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Materials>:"))
		{
			SkipMaterialsFromFile(pInFile);

		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
}
void CGameObject::SkipMaterialsFromFile(FILE* pInFile)
{
	char pstrToken[64] = { '\0' };

	int nMaterial = 0;
	BYTE nStrLength = 0;
	void* buffer;

	UINT nReads = (UINT)::fread(&buffer, sizeof(int), 1, pInFile);
	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Material>:"))
		{
			::ReadStringFromFile(pInFile, pstrToken);
		}
		else if (!strcmp(pstrToken, "<TextureProperties>:"))
		{

			for (int i = ::ReadIntegerFromFile(pInFile); i > 0; --i)
			{
				::ReadStringFromFile(pInFile, pstrToken);
				if (!strcmp(pstrToken, "<Property>:"))
				{
					::ReadIntegerFromFile(pInFile);
					::ReadStringFromFile(pInFile, pstrToken);
					::ReadStringFromFile(pInFile, pstrToken);
					if (strcmp(pstrToken, "Null")) {

						::ReadStringFromFile(pInFile, pstrToken);
						if (!strcmp(pstrToken, "<Textures>:"))
						{
							int nTextures = ::ReadIntegerFromFile(pInFile);

							if (nTextures > 0) {
								for (int j = 0; j < nTextures; ++j)
								{
									::ReadStringFromFile(pInFile, pstrToken);
									if (!strcmp(pstrToken, "<Texture>:"))
									{
										::ReadIntegerFromFile(pInFile);
										::ReadStringFromFile(pInFile, pstrToken);
										if (::ReadIntegerFromFile(pInFile) == 0)
										{
											::ReadStringFromFile(pInFile, pstrToken);

										}
									}
								}
							}
						}
					}
				}
				else if (!strcmp(pstrToken, "</TextureProperties>:"))
				{
					break;
				}
			}
		}
		else if (!strcmp(pstrToken, "<Lambert>:"))
		{
			
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			
			 ::ReadFloatFromFile(pInFile);
			 ::ReadFloatFromFile(pInFile);
			 ::ReadFloatFromFile(pInFile);

			//Transparency Factor
			::ReadFloatFromFile(pInFile);

		}
		else if (!strcmp(pstrToken, "<Phong>:"))
		{
			
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);
			::ReadFloatFromFile(pInFile);

			//Transparency Factor
			::ReadFloatFromFile(pInFile);
			
			::ReadFloatFromFile(pInFile);
			
			::ReadFloatFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "</Materials>"))
		{
			break;
		}
	}
}

void CGameObject::SkipMeshFromFile(FILE* pInFile)
{

	char pstrToken[64] = { '\0' };
	BYTE nStrLength = 0;
	UINT nReads = 0, m_nVertices;
	void* buffer = NULL;
	XMFLOAT3* bufCP = NULL , * bufN = NULL, * bufT = NULL, * bufBT = NULL;
	XMFLOAT2* bufUV = NULL;
	UINT* ibuf = NULL;
	::ReadStringFromFile(pInFile, pstrToken);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&buffer, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&buffer, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<ControlPoints>:"))
		{
			m_nVertices = ::ReadUnsignedIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				bufCP = new XMFLOAT3[m_nVertices];
				nReads = (UINT)::fread(bufCP, sizeof(XMFLOAT3), m_nVertices, pInFile);
				delete bufCP;
			}
		}
		else if (!strcmp(pstrToken, "<UVs>:"))
		{
			int nUVsPerVertex = ::ReadIntegerFromFile(pInFile);
			if (nUVsPerVertex > 0) {
				int nUVs = nUVsPerVertex * m_nVertices;

				bufUV = new XMFLOAT2[nUVs];
				nReads = (UINT)::fread(bufUV, sizeof(XMFLOAT2), nUVs, pInFile);
				delete bufUV;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			int nNormalsPerVertex = ::ReadIntegerFromFile(pInFile);
			if (nNormalsPerVertex > 0) {
				int nNormals = nNormalsPerVertex * m_nVertices;

				bufN = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(bufN, sizeof(XMFLOAT3), nNormals, pInFile);
				delete bufN;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			int nTangentsPerVertex = ::ReadIntegerFromFile(pInFile);
			if (nTangentsPerVertex > 0) {
				int nTangents = nTangentsPerVertex * m_nVertices;

				bufT = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(bufT, sizeof(XMFLOAT3), nTangents, pInFile);
				delete bufT;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			int nBiTangentsPerVertex = ::ReadIntegerFromFile(pInFile);
			if (nBiTangentsPerVertex > 0) {
				int nBiTangents = nBiTangentsPerVertex * m_nVertices;

				bufBT = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(bufBT, sizeof(XMFLOAT3), nBiTangents, pInFile);
				delete bufBT;
			}
		}
		else if (!strcmp(pstrToken, "<Polygons>:"))
		{
			int nPolygons = ::ReadIntegerFromFile(pInFile);
			for (; ; )
			{
				::ReadStringFromFile(pInFile, pstrToken);

				if (!strcmp(pstrToken, "<SubIndices>:"))
				{
					::ReadIntegerFromFile(pInFile);
					int m_nSubMeshes = ::ReadIntegerFromFile(pInFile);
					if (m_nSubMeshes == 0) m_nSubMeshes = 1;

					for (int i = 0; i < m_nSubMeshes; i++)
					{
						::ReadStringFromFile(pInFile, pstrToken);

						if (!strcmp(pstrToken, "<SubIndex>:"))
						{
							::ReadIntegerFromFile(pInFile);
							int n = ::ReadIntegerFromFile(pInFile);
							if (n > 0)
							{
								ibuf = new UINT[n];
								nReads = (UINT)::fread(ibuf, sizeof(UINT),n, pInFile);
								delete ibuf;
							}
						}
					}
				}
				else if (!strcmp(pstrToken, "</Polygons>"))
				{
					break;
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

//D3D12_GPU_DESCRIPTOR_HANDLE CGameObject::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nRootParameter, bool bAutoIncrement)
//{
//	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
//	if (pTexture)
//	{
//		int nTextures = pTexture->GetTextures();
//		int nTextureType = pTexture->GetTextureType();
//		for (int i = 0; i < nTextures; i++)
//		{
//			ID3D12Resource* pShaderResource = pTexture->GetTexture(i);
//			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
//			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
//			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
//			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
//
//			pTexture->SetGraphicsSrvRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
//			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
//		}
//	}
//	return(d3dSrvGPUDescriptorHandle);
//}
//
//void CGameObject::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
//{
//	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
//	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
//	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//	d3dDescriptorHeapDesc.NodeMask = 0;
//	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);
//
//	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
//	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
//	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
//}

void CGameObject::PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

void CGameObject::LoadAnimationFromFile(FILE *pInFile, CLoadedModelInfo *pLoadedModel)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nAnimationSets = 0;

	for ( ; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<AnimationSets>:"))
		{
			nAnimationSets = ::ReadIntegerFromFile(pInFile);
			pLoadedModel->m_pAnimationSets = new CAnimationSets(nAnimationSets);
		}
		else if (!strcmp(pstrToken, "<AnimationSet>:"))
		{
			int nAnimationSet = ::ReadIntegerFromFile(pInFile);

			::ReadStringFromFile(pInFile, pstrToken); //Animation Set Name

			TCHAR pstrDebug[256] = { 0 };
			_stprintf_s(pstrDebug, 256, _T("Animation Name: %hs\n"), pstrToken);
			OutputDebugString(pstrDebug);

			float fStartTime = ::ReadFloatFromFile(pInFile);
			float fEndTime = ::ReadFloatFromFile(pInFile);

			pLoadedModel->m_pAnimationSets->m_ppAnimationSets[nAnimationSet] = new CAnimationSet(fStartTime, fEndTime, pstrToken);
			CAnimationSet *pAnimationSet = pLoadedModel->m_pAnimationSets->m_ppAnimationSets[nAnimationSet];

			::ReadStringFromFile(pInFile, pstrToken);
			if (!strcmp(pstrToken, "<AnimationLayers>:"))
			{
				pAnimationSet->m_nAnimationLayers = ::ReadIntegerFromFile(pInFile);
				pAnimationSet->m_pAnimationLayers = new CAnimationLayer[pAnimationSet->m_nAnimationLayers];

				for (int i = 0; i < pAnimationSet->m_nAnimationLayers; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<AnimationLayer>:"))
					{
						int nAnimationLayer = ::ReadIntegerFromFile(pInFile);
						CAnimationLayer *pAnimationLayer = &pAnimationSet->m_pAnimationLayers[nAnimationLayer];

						pAnimationLayer->m_nAnimatedBoneFrames = ::ReadIntegerFromFile(pInFile);

						pAnimationLayer->m_ppAnimatedBoneFrameCaches = new CGameObject *[pAnimationLayer->m_nAnimatedBoneFrames];
						pAnimationLayer->m_ppAnimationCurves = new CAnimationCurve *[pAnimationLayer->m_nAnimatedBoneFrames][9];

						pAnimationLayer->m_fWeight = ::ReadFloatFromFile(pInFile);

						for (int j = 0; j < pAnimationLayer->m_nAnimatedBoneFrames; j++)
						{
							::ReadStringFromFile(pInFile, pstrToken);
							if (!strcmp(pstrToken, "<AnimationCurve>:"))
							{
								int nCurveNode = ::ReadIntegerFromFile(pInFile); //j

								for (int k = 0; k < 9; k++) pAnimationLayer->m_ppAnimationCurves[j][k] = NULL;

								::ReadStringFromFile(pInFile, pstrToken);
								pAnimationLayer->m_ppAnimatedBoneFrameCaches[j] = pLoadedModel->m_pModelRootObject->FindFrame(pstrToken);

								for ( ; ; )
								{
									::ReadStringFromFile(pInFile, pstrToken);
									if (!strcmp(pstrToken, "<TX>:")) pAnimationLayer->LoadAnimationKeyValues(j, 0, pInFile);
									else if(!strcmp(pstrToken, "<TY>:")) pAnimationLayer->LoadAnimationKeyValues(j, 1, pInFile);
									else if(!strcmp(pstrToken, "<TZ>:")) pAnimationLayer->LoadAnimationKeyValues(j, 2, pInFile);
									else if(!strcmp(pstrToken, "<RX>:")) pAnimationLayer->LoadAnimationKeyValues(j, 3, pInFile);
									else if(!strcmp(pstrToken, "<RY>:")) pAnimationLayer->LoadAnimationKeyValues(j, 4, pInFile);
									else if(!strcmp(pstrToken, "<RZ>:")) pAnimationLayer->LoadAnimationKeyValues(j, 5, pInFile);
									else if(!strcmp(pstrToken, "<SX>:")) pAnimationLayer->LoadAnimationKeyValues(j, 6, pInFile);
									else if(!strcmp(pstrToken, "<SY>:")) pAnimationLayer->LoadAnimationKeyValues(j, 7, pInFile);
									else if(!strcmp(pstrToken, "<SZ>:")) pAnimationLayer->LoadAnimationKeyValues(j, 8, pInFile);
									else if(!strcmp(pstrToken, "</AnimationCurve>"))
									{
										break;
									}
								}
							}
						}
						::ReadStringFromFile(pInFile, pstrToken); //</AnimationLayer>
					}
				}
				::ReadStringFromFile(pInFile, pstrToken); //</AnimationLayers>
			}
			::ReadStringFromFile(pInFile, pstrToken); //</AnimationSet>
		}
		else if (!strcmp(pstrToken, "</AnimationSets>"))
		{
			break;
		}
	}
}

CLoadedModelInfo *CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName, CShader *pShader)
{
	FILE *pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo *pLoadedModel = new CLoadedModelInfo();
	pLoadedModel->m_pModelRootObject = new CGameObject();
	strcpy_s(pLoadedModel->m_pModelRootObject->m_pstrFrameName, "RootNode");

	char pstrToken[64] = { '\0' };

	for ( ; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>"))
			{
				for ( ; ; )
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<Frame>:"))
					{
						CGameObject *pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, pShader, &pLoadedModel->m_nSkinnedMeshes);
						if (pChild) pLoadedModel->m_pModelRootObject->SetChild(pChild);
					}
					else if (!strcmp(pstrToken, "</Hierarchy>"))
					{
						break;
					}
				}
			}
			else if (!strcmp(pstrToken, "<Animation>"))
			{
				CGameObject::LoadAnimationFromFile(pInFile, pLoadedModel);
				pLoadedModel->PrepareSkinning();
			}
			else if (!strcmp(pstrToken, "</Animation>"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pLoadedModel->m_pModelRootObject, NULL);
#endif

	::fclose(pInFile);

	return(pLoadedModel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int texture) : CGameObject(1)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh *pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);


	CTexture* pWaterTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pWaterTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/Water.dds", 0);
	CTexture* pWaterNormalTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pWaterNormalTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/Water_Normal.dds", 0);

	CScene::CreateShaderResourceViews(pd3dDevice, pWaterTexture, false);
	pWaterTexture->SetGraphicsSrvRootArgument(0, 5, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pWaterNormalTexture, false);
	pWaterNormalTexture->SetGraphicsSrvRootArgument(0, 6, 0);


	CTexture *pTerrainBaseTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

	switch (texture) {
	case 0:
		pTerrainBaseTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Terrain/Desert_Floor_Texture.dds", 0);
		break;
	case 1:
		pTerrainBaseTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Terrain/Grass_02.dds", 0);
		break;
	case 2:
		pTerrainBaseTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Terrain/Snow.dds", 0);
		break;
	}

	CTexture *pTerrainDetailTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTerrainDetailTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Terrain/Detail_Texture_7.dds", 0);

	CTerrainShader *pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pTerrainBaseTexture, false);
	pTerrainBaseTexture->SetGraphicsSrvRootArgument(0, 13, 0);
	CScene::CreateShaderResourceViews(pd3dDevice, pTerrainDetailTexture, false);
	pTerrainDetailTexture->SetGraphicsSrvRootArgument(0, 14, 0);

	CMaterial *pTerrainMaterial = new CMaterial(4);
	pTerrainMaterial->SetTexture(pTerrainBaseTexture, 0);
	pTerrainMaterial->SetTexture(pTerrainDetailTexture, 1);
	pTerrainMaterial->SetTexture(pWaterTexture, 2);
	pTerrainMaterial->SetTexture(pWaterNormalTexture, 3);
	pTerrainMaterial->SetShader(pTerrainShader);

	SetMaterial(0, pTerrainMaterial);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(1)
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 2.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"SkyBox/SkyBox_0.dds", 0);

	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CScene::CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, false);
	pSkyBoxTexture->SetGraphicsSrvRootArgument(0, 10, 0);

	CMaterial *pSkyBoxMaterial = new CMaterial(1);
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);
	
	CGameObject::Render(pd3dCommandList, pCamera);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//

CMap::CMap(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, vector<int> arrange, void* pContext)
{
	//CLoadedModelInfo* pDesert_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Collision.bin", NULL);
	//CLoadedModelInfo* pDesert_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Passable.bin", NULL);
	//CLoadedModelInfo* pDesert_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Passable.bin", NULL);

	//CLoadedModelInfo* pForest_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Collision.bin", NULL);
	//CLoadedModelInfo* pForest_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Passable.bin", NULL);
	//CLoadedModelInfo* pForest_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Passable.bin", NULL);

	//CLoadedModelInfo* pSnow_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Collision.bin", NULL);
	//CLoadedModelInfo* pSnow_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Passable.bin", NULL);
	//CLoadedModelInfo* pSnow_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Passable.bin", NULL);
	////SetChild(pDesert_Test->m_pModelRootObject, true);

	////CLoadedModelInfo* pDesert_Test[3] = { NULL };
	////for(int i =0; i < 3; i++)
	////	pDesert_Test[i] = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Passable.bin", NULL);

	////for (int i = 0; i < 3; i++)
	////{
	////	m_ppMaps[i] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Test[i]);
	////	m_ppMaps[i]->SetPosition(0.0f, 0.0f, 2048.0f * (i+1));
	////	printf("%d번째 위치 : x - %f  y - %f  z - %f", i, m_ppMaps[i]->GetPosition().x, m_ppMaps[i]->GetPosition().y, m_ppMaps[i]->GetPosition().z);
	////}
	//m_ppMaps[0] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Collision, true);
	//m_ppMaps[1] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Steppable);
	//m_ppMaps[2] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Passable);

	//m_ppMaps[3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Collision, true);
	//m_ppMaps[4] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Steppable);
	//m_ppMaps[5] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Passable);

	//m_ppMaps[3]->SetPosition(-2048.0f, 0.0f, 0.0f);
	//m_ppMaps[4]->SetPosition(-2048.0f, 0.0f, 0.0f);
	//m_ppMaps[5]->SetPosition(-2048.0f, 0.0f, 0.0f);

	//m_ppMaps[6] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Collision, true);
	//m_ppMaps[7] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Steppable);
	//m_ppMaps[8] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Passable);

	//m_ppMaps[6]->SetPosition(2048.0f, 60.0f, 0.0f);
	//m_ppMaps[7]->SetPosition(2048.0f, 60.0f, 0.0f);
	//m_ppMaps[8]->SetPosition(2048.0f, 60.0f, 0.0f);

	//m_ppMaps[9] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Collision, true);
	//m_ppMaps[10] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Steppable);
	//m_ppMaps[11] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Passable);

	//m_ppMaps[9]->SetPosition(0.0f, 60.0f, 2048.0f);
	//m_ppMaps[10]->SetPosition(0.0f, 60.0f, 4096.0f);
	//m_ppMaps[11]->SetPosition(0.0f, 60.0f, 6144.0f);

	m_nMaps = 27;
	m_ppMaps = new CGameObject * [m_nMaps];


	m_nObjectInstance = 0;
	m_ppObjectInstance = new CGameObject * [108];
	gbInstancing = true;

	for (int i = 0; i < 3; i++)
	{
		CLoadedModelInfo* pDesert_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Collision.bin", NULL);
		CLoadedModelInfo* pDesert_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Passable.bin", NULL);
		CLoadedModelInfo* pDesert_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Passable.bin", NULL);

		m_ppMaps[0 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Collision, true);
		m_ppMaps[1 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Steppable);
		m_ppMaps[2 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Passable);

		m_ppMaps[0 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
		m_ppMaps[1 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
		m_ppMaps[2 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	}

	for (int i = 3; i < 6; i++)
	{
		CLoadedModelInfo* pForest_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Collision.bin", NULL);
		CLoadedModelInfo* pForest_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Passable.bin", NULL);
		CLoadedModelInfo* pForest_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Passable.bin", NULL);

		m_ppMaps[0 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Collision, true);
		m_ppMaps[1 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Steppable);
		m_ppMaps[2 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Passable);

		m_ppMaps[0 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
		m_ppMaps[1 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
		m_ppMaps[2 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	}

	for (int i = 6; i < 9; i++)
	{
		CLoadedModelInfo* pSnow_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Collision.bin", NULL);
		CLoadedModelInfo* pSnow_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Passable.bin", NULL);
		CLoadedModelInfo* pSnow_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Passable.bin", NULL);

		m_ppMaps[0 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Collision, true);
		m_ppMaps[1 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Steppable);
		m_ppMaps[2 + i * 3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Passable);

		m_ppMaps[0 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 60.0f, 2048.0f * arrange[2 * i + 1]);
		m_ppMaps[1 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 60.0f, 2048.0f * arrange[2 * i + 1]);
		m_ppMaps[2 + i * 3]->SetPosition(2048.0f * arrange[2 * i], 60.0f, 2048.0f * arrange[2 * i + 1]);
	}
	gbInstancing = false;


	//for (int i = 0; i < (arrange.size() / 2); i++)
	//{
	//	m_ppMaps[3 * i + 0]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	//	m_ppMaps[3 * i + 1]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	//	m_ppMaps[3 * i + 2]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	//}

	//for (int i = 0; i < 3; ++i) {
	//	for (int j = 0; j < 3; ++j) {
	//		int n = i * 3 + j;
	//		if (n % 3 == 0) {
	//			m_ppMaps[n] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pMapForest, 0);
	//			m_ppMaps[n]->SetPosition(i * 500.0f, 0, j * 500.0f);
	//		}
	//		else if (n % 3 == 1) {
	//			m_ppMaps[n] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pMapDesert, 0);
	//			m_ppMaps[n]->SetPosition(i * 500.0f, 0, j * 500.0f);
	//		}
	//		else {
	//			m_ppMaps[n] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pMapSnow, 0);
	//			m_ppMaps[n]->SetPosition(i * 500.0f, 0, j * 500.0f);
	//		}
	//		//m_ppMaps[n]->SetScale(10,10,10);
	//	}
	//}


}

CMap::~CMap()
{
}

void CMap::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nMaps; ++i)
		m_ppMaps[i]->Render(pd3dCommandList, pCamera);
}

void CMap::RenderShadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::RenderShadow(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nMaps; ++i)
		m_ppMaps[i]->RenderShadow(pd3dCommandList, pCamera);
}

void CMap::CheckCollision(CPlayer* pPlayer)
{
	CGameObject* pObject;
	for (int i = 0; i < 9; ++i) {
		pObject = m_ppMaps[3 * i]->FindFrame("RootNode")->m_pChild->m_pChild;
		while (true) {
			if (pPlayer->isCollide(pObject)) {
				XMFLOAT3 d = Vector3::Subtract(pPlayer->GetPosition(), pObject->GetPosition());
				pPlayer->Move(Vector3::ScalarProduct(d, 50.25f, true), true);

				cout << "Map Collision - " << pObject->m_pstrFrameName << endl;
				return;
			}
			if (pObject->m_pSibling)
				pObject = pObject->m_pSibling;
			else
				return;
		}
	}
}

void CMap::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nMaps; ++i)
		m_ppMaps[i]->ReleaseUploadBuffers();
}

void CMap::Release()
{
	for (int i = 0; i < m_nMaps; ++i)
		if(m_ppMaps[i])
			m_ppMaps[i]->Release();
}

CGameObject* CMap::GetMap(int idx) const
{
	return m_ppMaps[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

CPlayerObject::CPlayerObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks)
{
	CLoadedModelInfo* pPlayerModel = pModel;
	if (!pPlayerModel) pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Player_1Hsword.bin", NULL);

	SetChild(pPlayerModel->m_pModelRootObject, true);
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pPlayerModel);

	
	strcpy_s(m_pstrFrameName, "Player");

	//Rotate(-90.0f, 0.0f, 0.0f);
	//SetScale(0.2,0.2,0.2);
}

CPlayerObject::~CPlayerObject()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

CMapObject::CMapObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel,bool bCollide)
{
	CLoadedModelInfo* pMapModel = pModel;
	if (!pMapModel) pMapModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Tree.bin", NULL);

	SetChild(pMapModel->m_pModelRootObject, true);

	CGameObject* pObject = FindFrame("RootNode")->m_pChild->m_pChild;	

	while (bCollide) {
		BoundingBox bb = BoundingBox(pObject->m_pMesh->m_xmf3AABBCenter, pObject->m_pMesh->m_xmf3AABBExtents);
		pObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 0), bb.Extents);
		//pObject->UpdateTransform();
		if (pObject->m_pSibling)
			pObject = pObject->m_pSibling;
		else
			break;
	}

	strcpy_s(m_pstrFrameName, "Map");
	Rotate(0, 90.f, 0);

	//SetScale(20.5f, 10.25f, 20.5f);


	
	//Rotate(-90.0f, 0.0f, 0.0f);
}

CMapObject::~CMapObject()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 


void CBullet::Animate(float fElapsedTime) {
	//Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fElapsedTime);

	if (m_fSpeed != 0.0f) Move(m_xmf3MovingDirection, m_fSpeed * fElapsedTime);
	//if (m_xmf3MovingDirection.y <= m_xmf3Gravity.y)
	//	std::cout << "---------------------------------------------" << endl;
	m_xmf3MovingDirection = Vector3::Add(m_xmf3MovingDirection, m_xmf3Gravity, fElapsedTime);

	Rotate(-90.f,0,0);
	XMFLOAT3 look = GetLook();
	Rotate(90.f, 0, 0);


	m_fRotationX = acos(Vector3::DotProduct(m_xmf3MovingDirection, look) / (Vector3::Length(look) * Vector3::Length(m_xmf3MovingDirection)));
	//std::cout << m_fRotationX << std::endl;
	if (EPSILON <= m_fRotationX)
		Rotate(m_fRotationX / PI * 180, 0, 0);
}

void CBullet::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}


CDragon::CDragon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks,void* pContext, int nAnimationCount) :CMonster()
{
	CLoadedModelInfo* pDragonModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Monster/Dragon.bin", NULL);

	SetChild(pDragonModel->m_pModelRootObject, true);

	m_nAnimations = nAnimationTracks;
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pDragonModel);

	strcpy_s(m_pstrFrameName, "Dragon");


	CGameObject* pObject = pDragonModel->m_pModelRootObject->FindFrame("Polygonal_Dragon");

	BoundingBox bb = BoundingBox(pObject->m_pMesh->m_xmf3AABBCenter, pObject->m_pMesh->m_xmf3AABBExtents);
	/*CCubeMesh* pBoundingBox = new CCubeMesh(pd3dDevice, pd3dCommandList, bb.Extents.x * 2, bb.Extents.y * 2, bb.Extents.z * 2);

	SetBBObject(pBoundingBox);*/

	SetBBObject(pd3dDevice, pd3dCommandList,
		XMFLOAT3(0, bb.Extents.y-350, -50),					// Center
		XMFLOAT3(80, bb.Extents.y - 250, bb.Extents.z-50));	// Extents

	//BoundingBox detect = BoundingBox(pObject->m_pMesh->m_xmf3AABBCenter, Vector3::ScalarProduct(pObject->m_pMesh->m_xmf3AABBExtents,1.5f,false));
	//SetBBObject(pd3dDevice, pd3dCommandList,
	//	XMFLOAT3(0, 0, 0),					// Center
	//	detect.Extents);	// Extents
	//

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	SetPosition(300.f, pTerrain->GetHeight(300.f, 300.f), 300.f);
	MoveUp(pObject->m_pMesh->m_xmf3AABBExtents.y * 0.2f);
	SetScale(0.5f, 0.5f, 0.5f);
	Rotate(-90.0f, 20.0f, 0.0f);

	m_nTrackOffSet = nAnimationCount;
	InitAnimation();

	delete pDragonModel;
}

CDragon::~CDragon()
{
}

void CDragon::Update(float fTimeElapsed)
{
	CMonster::Update(fTimeElapsed);
	if (m_pSkinnedAnimationController->IsTrackFinish(MonsterState::Die )) {
		SetIdle();
		SetActive("BoundingBox", false);
		SetActive("Polygonal_Dragon", false);
	}
}

void CDragon::Attack()
{
	if (m_iState == Idle || m_iState == MonsterState::Move) {
		ChangeState(nDragon_BiteAttack);
	}
	else if (m_pSkinnedAnimationController->IsTrackFinish(nDragon_BiteAttack)) {
		m_pSkinnedAnimationController->SetTrackPosition(nDragon_BiteAttack, 0);
	}
}

void CDragon::InitAnimation()
{
	CMonster::InitAnimation();
	m_pSkinnedAnimationController->SetTrackType(nDragon_BiteAttack, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackType(nDragon_ProjectileAttack, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackType(nDragon_BreathAttack, ANIMATION_TYPE_ONCE);

#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nMonster_Die, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nMonster_Die, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nMonster_Die, pAnimationCallbackHandler);
#endif

	m_iState = Idle;
	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackPosition(m_iState, 0);
	m_pSkinnedAnimationController->SetTrackEnable(m_iState, true);
}

void CDragon::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	CMonster::Move(vDirection, fSpeed);

	if (m_pSkinnedAnimationController->IsTrackFinish(nDragon_BiteAttack)) {
		ChangeState(MonsterState::Move);
	}
}

CWolf::CWolf(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks, void* pContext, int nAnimationCount) :CMonster()
{
	CLoadedModelInfo* pWolfModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Monster/Wolf.bin", NULL);
	
	SetChild(pWolfModel->m_pModelRootObject, true);

	m_nAnimations = nAnimationTracks;
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pWolfModel);
	strcpy_s(m_pstrFrameName, "Wolf");


	CGameObject* pObject = pWolfModel->m_pModelRootObject->FindFrame("Polygonal_Wolf");

	BoundingBox bb = BoundingBox(pObject->m_pMesh->m_xmf3AABBCenter, pObject->m_pMesh->m_xmf3AABBExtents);
	/*CCubeMesh* pBoundingBox = new CCubeMesh(pd3dDevice, pd3dCommandList, bb.Extents.x * 2, bb.Extents.y * 2, bb.Extents.z * 2);

	SetBBObject(pBoundingBox)->MoveForward(-bb.Extents.z);*/

	SetBBObject(pd3dDevice, pd3dCommandList,
		XMFLOAT3(0, 20, -bb.Extents.z),									// Center
		XMFLOAT3(bb.Extents.x, bb.Extents.y - 20, bb.Extents.z));	// Extents

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	SetPosition(400.f, pTerrain->GetHeight(400.f, 400.f), 400.f);
	MoveUp(pObject->m_pMesh->m_xmf3AABBExtents.y * 0.5f);
	SetScale(0.5f, 0.5f, 0.5f);
	Rotate(-90.0f, -40.0f, 0.0f);

	m_nTrackOffSet = nAnimationCount;
	InitAnimation();

	delete pWolfModel;
}

CWolf::~CWolf()
{
}

void CWolf::Update(float fTimeElapsed)
{
	CMonster::Update(fTimeElapsed);
	if (m_pSkinnedAnimationController->IsTrackFinish(MonsterState::Die )) {
		SetIdle();
		SetActive("BoundingBox", false);
		SetActive("Polygonal_Wolf", false);
	}
}

void CWolf::Attack()
{
	if (m_iState == Idle || m_iState == MonsterState::Move) {
		ChangeState(nWolf_BiteAttack);
	}
	else if (m_pSkinnedAnimationController->IsTrackFinish(nWolf_BiteAttack)) {
		m_pSkinnedAnimationController->SetTrackPosition(nWolf_BiteAttack, 0);
	}
}

void CWolf::InitAnimation()
{
	CMonster::InitAnimation();

	m_pSkinnedAnimationController->SetTrackType(nWolf_BiteAttack, ANIMATION_TYPE_ONCE);
#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nMonster_Die, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nMonster_Die, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nMonster_Die, pAnimationCallbackHandler);
#endif

	m_iState = Idle;
	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackPosition(m_iState, 0);
	m_pSkinnedAnimationController->SetTrackEnable(m_iState, true);
}

void CWolf::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	CMonster::Move(vDirection, fSpeed);

	if (m_pSkinnedAnimationController->IsTrackFinish(nWolf_BiteAttack)) {
		ChangeState(MonsterState::Move);
	}
}

CMetalon::CMetalon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks, void* pContext, int nAnimationCount) :CMonster()
{
	CLoadedModelInfo* pMetalonModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Monster/Metalon.bin", NULL);

	SetChild(pMetalonModel->m_pModelRootObject, true);

	m_nAnimations = nAnimationTracks;
	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, pMetalonModel);

	strcpy_s(m_pstrFrameName, "Metalon");

	CGameObject* pObject = pMetalonModel->m_pModelRootObject->FindFrame("Polygonal_Metalon");

	BoundingBox bb = BoundingBox(pObject->m_pMesh->m_xmf3AABBCenter, pObject->m_pMesh->m_xmf3AABBExtents);

	/*CCubeMesh* pBoundingBox = new CCubeMesh(pd3dDevice, pd3dCommandList, bb.Extents.x * 2, bb.Extents.y * 2, bb.Extents.z * 2);

	SetBBObject(pBoundingBox);*/

	pObject->SetBBObject(pd3dDevice, pd3dCommandList,
		XMFLOAT3(0, 20, -bb.Extents.z),													// Center
		XMFLOAT3(bb.Extents.x, bb.Extents.y-20, bb.Extents.z));	// Extents

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	SetPosition(500.f, pTerrain->GetHeight(500.f, 500.f), 500.f);
	MoveUp(pObject->m_pMesh->m_xmf3AABBExtents.y*0.1f);
	SetScale(0.1f, 0.1f, 0.1f);
	Rotate(-90.0f, 0.0f, 0.0f);

	m_nTrackOffSet = nAnimationCount;
	InitAnimation();

	delete pMetalonModel;
}

CMetalon::~CMetalon()
{
}

void CMetalon::Update(float fTimeElapsed)
{
	CMonster::Update(fTimeElapsed);
	if (m_pSkinnedAnimationController->IsTrackFinish(MonsterState::Die )) {
		SetIdle();
		SetActive("Polygonal_Metalon", false);
		SetActive("BoundingBox", false);
	}
}

void CMetalon::Attack()
{
	if (m_iState == Idle || m_iState == MonsterState::Move) {
		ChangeState(nMetalon_CastSpell);
	}
	else if (m_pSkinnedAnimationController->IsTrackFinish(nMetalon_CastSpell)) {
		m_pSkinnedAnimationController->SetTrackPosition(nMetalon_CastSpell, 0);
	}
}

void CMetalon::InitAnimation()
{
	CMonster::InitAnimation();

	m_pSkinnedAnimationController->SetTrackType(nMetalon_CastSpell, ANIMATION_TYPE_ONCE);
#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nMonster_Die, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nMonster_Die, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nMonster_Die, pAnimationCallbackHandler);
#endif
	m_iState = Idle;
	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackPosition(m_iState, 0);
	m_pSkinnedAnimationController->SetTrackEnable(m_iState, true);
}

void CMetalon::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	CMonster::Move(vDirection, fSpeed);

	if (m_pSkinnedAnimationController->IsTrackFinish(nMetalon_CastSpell)) {
		ChangeState(MonsterState::Move);
	}
}

CMonster::CMonster()
{
	m_nAnimations = 3;
	m_iHp = 100;
	m_iAtkStat = 10;
	m_iDefStat = 0;
	m_iState = Idle;
}

CMonster::~CMonster()
{
}

void CMonster::TakeDamage(int iDamage)
{
	CGameObject::TakeDamage(iDamage);
	if (m_iHp > 0)
		m_iState = MonsterState::TakeDamage;
	else {
		m_iState = MonsterState::Die;
	}
	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackPosition(m_iState , 0);
	m_pSkinnedAnimationController->SetTrackEnable(m_iState , true);
}

void CMonster::Update(float fTimeElapsed)
{
	if (m_pSkinnedAnimationController->IsTrackFinish(MonsterState::TakeDamage )) {
		SetIdle();
	}
}

void CMonster::SetIdle()
{
	ChangeState(Idle);
}

void CMonster::ChangeState(int nState)
{
	m_pSkinnedAnimationController->SetTrackEnable(m_iState, false);
	m_pSkinnedAnimationController->SetTrackPosition(m_iState, 0);
	m_iState = nState;
	m_pSkinnedAnimationController->SetTrackPosition(m_iState, 0);
	m_pSkinnedAnimationController->SetTrackEnable(m_iState, true);
}

void CMonster::Attack()
{
}

void CMonster::InitAnimation()
{
	//m_nTrackOffSet *= m_nAnimations;

	for (int i = 0; i < m_nAnimations; ++i)
		m_pSkinnedAnimationController->SetTrackAnimationSet(i , i);

	m_pSkinnedAnimationController->SetTrackType(nMonster_Die, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackType(nMonster_TakeDamage, ANIMATION_TYPE_ONCE);
}

void CMonster::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	if(m_iState == MonsterState::Move)
		CGameObject::Move(vDirection, fSpeed);
}
