//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 4;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 1000.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(30.0f, 30.0f, 30.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights[3].m_bEnable = true;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(50.0f, 30.0f, 30.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	m_pd3dComputeRootSignature = CreateComputeRootSignature(pd3dDevice);

	BuildDefaultLightsAndMaterials();

	//Set Fog
	m_xmf4FogColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.2f);
	m_xmf4FogParameter = XMFLOAT4(NO_FOG, 1.f, 300.f, 0.01f);
	gbFog = false;

	CreateCbvSrvUavDescriptorHeaps(pd3dDevice, 0, 2, 1);

	m_pTextureRenderTarget = new CTexture2(2, RESOURCE_TEXTURE2D, 0, 1, 1, 1);

	D3D12_CLEAR_VALUE d3dRtbClearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } };
	m_pTextureRenderTarget->CreateTexture(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dRtbClearValue, RESOURCE_TEXTURE2D, 0);
	m_pTextureRenderTarget->CreateTexture(pd3dDevice, 800, 600, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, NULL, RESOURCE_TEXTURE2D, 1);


	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);

	

	D3D12_RENDER_TARGET_VIEW_DESC RtvDesc;
	RtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	RtvDesc.Texture2D.MipSlice = 0;
	RtvDesc.Texture2D.PlaneSlice = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pd3dDevice->CreateRenderTargetView(m_pTextureRenderTarget->GetResource(0), &RtvDesc, d3dRtvCPUDescriptorHandle);

	CreateShaderResourceViews(pd3dDevice, m_pTextureRenderTarget, 0);
	CreateUnorderedAccessView(pd3dDevice, m_pTextureRenderTarget, 1);

	m_pTextureRenderTarget->SetComputeSrvRootParameterIndex(0, 0, 0);
	m_pTextureRenderTarget->SetComputeUavRootParameterIndex(0, 1, 1);
	m_pTextureRenderTarget->SetGraphicsSrvRootParameterIndex(0, 18, 1);


	m_pHorzBlurShader = new CHorzBlurShader();
	m_pHorzBlurShader->CreateShader(pd3dDevice, m_pd3dComputeRootSignature, 256, 600, 1);

	m_pVertBlurShader = new CVertBlurShader();
	m_pVertBlurShader->CreateShader(pd3dDevice, m_pd3dComputeRootSignature, 800, 256, 1);

	m_pScreenShader = new CTextureToFullScreenShader();
	m_pScreenShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);



	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Image/terrain.raw"),
		TERRAIN_WIDTH, TERRAIN_LENGTH, 9, 9, c_xmf3Scale, xmf4Color);

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	m_nShaders = 4;
	m_ppShaders = new CShader*[m_nShaders];

	CObjectsShader *pObjectsShader = new CObjectsShader();
	pObjectsShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, NULL);
	pObjectsShader->SetTerrain(m_pTerrain);
	CBillboardObjectsShader* pBillboardShader = new CBillboardObjectsShader();
	pBillboardShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	pBillboardShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pTerrain);

	CBulletsShader* pBulletShader = new CBulletsShader();
	pBulletShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pBulletShader->BuildObjects(pd3dDevice, pd3dCommandList, NULL);

	// 물을 제일 마지막에
	CTexturedShader* pTextureShader = new CTexturedShader();
	pTextureShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pTextureShader->BuildObjects(pd3dDevice, pd3dCommandList,m_pTerrain);

	m_ppShaders[0] = pBillboardShader;
	m_ppShaders[1] = pObjectsShader;
	m_ppShaders[2] = pBulletShader;
	m_ppShaders[3] = pTextureShader;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dCbvSrvUavDescriptorHeap) m_pd3dCbvSrvUavDescriptorHeap->Release();

	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dComputeRootSignature) m_pd3dComputeRootSignature->Release();

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;

	if (m_pScreenShader)
	{
		m_pScreenShader->ReleaseShaderVariables();
		m_pScreenShader->ReleaseObjects();
		m_pScreenShader->Release();
	}
	if (m_pHorzBlurShader)
	{
		m_pHorzBlurShader->ReleaseShaderVariables();
		m_pHorzBlurShader->Release();
	}
	if (m_pVertBlurShader)
	{
		m_pVertBlurShader->ReleaseShaderVariables();
		m_pVertBlurShader->Release();
	}
}

ID3D12RootSignature* CScene::CreateRootSignature(ID3D12Device* pd3dDevice, D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags, UINT nRootParameters, D3D12_ROOT_PARAMETER* pd3dRootParameters, UINT nStaticSamplerDescs, D3D12_STATIC_SAMPLER_DESC* pd3dStaticSamplerDescs)
{
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = nRootParameters;
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = nStaticSamplerDescs;
	d3dRootSignatureDesc.pStaticSamplers = pd3dStaticSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3D12RootSignature* pd3dRootSignature = NULL;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dRootSignature);
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[17];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtDetailAlbedoTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtDetailNormalTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[10].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[10].NumDescriptors = 1;
	pd3dDescriptorRanges[10].BaseShaderRegister = 3; //t3: gtxtTerrainAlphaTexture
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[11].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[11].NumDescriptors = 7;
	pd3dDescriptorRanges[11].BaseShaderRegister = 14; //t14: gtxtBillboardTextures[7]
	pd3dDescriptorRanges[11].RegisterSpace = 0;
	pd3dDescriptorRanges[11].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[12].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[12].NumDescriptors = 1;
	pd3dDescriptorRanges[12].BaseShaderRegister = 21; //t21: gtxtWaterBaseTexture
	pd3dDescriptorRanges[12].RegisterSpace = 0;
	pd3dDescriptorRanges[12].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[13].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[13].NumDescriptors = 1;
	pd3dDescriptorRanges[13].BaseShaderRegister = 22; //t22:  gtxtWaterDetailTexture
	pd3dDescriptorRanges[13].RegisterSpace = 0;
	pd3dDescriptorRanges[13].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[14].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[14].NumDescriptors = 1;
	pd3dDescriptorRanges[14].BaseShaderRegister = 23; //t23: gtxtWaterAlphaTexture
	pd3dDescriptorRanges[14].RegisterSpace = 0;
	pd3dDescriptorRanges[14].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[15].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[15].NumDescriptors = 1;
	pd3dDescriptorRanges[15].BaseShaderRegister = 0; //t0: gtxtExplosionTexture
	pd3dDescriptorRanges[15].RegisterSpace = 0;
	pd3dDescriptorRanges[15].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[16].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[16].NumDescriptors = 1;
	pd3dDescriptorRanges[16].BaseShaderRegister = 25; //t25: gtxtOutput
	pd3dDescriptorRanges[16].RegisterSpace = 0;
	pd3dDescriptorRanges[16].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[19];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[11].DescriptorTable.NumDescriptorRanges = 3;
	pd3dRootParameters[11].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[12].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[12].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[11]);
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 3;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[12]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[14].Descriptor.ShaderRegister = 3; //Framework Info
	pd3dRootParameters[14].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[15].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[15].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[15]);
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[16].Constants.Num32BitValues = 1;
	pd3dRootParameters[16].Constants.ShaderRegister = 5; //time
	pd3dRootParameters[16].Constants.RegisterSpace = 0;
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[17].Constants.ShaderRegister = 6; //Fog
	pd3dRootParameters[17].Constants.RegisterSpace = 0;
	pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[18].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[18].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[18].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[16]);
	pd3dRootParameters[18].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#else
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 7;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtStandardTextures[7] //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#endif

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	ID3D12RootSignature* pd3dGraphicsRootSignature = CreateRootSignature(pd3dDevice, d3dRootSignatureFlags, _countof(pd3dRootParameters), pd3dRootParameters, _countof(pd3dSamplerDescs), pd3dSamplerDescs);
	
	/*D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	char* pErrorString = NULL;
	if (pd3dErrorBlob)
		pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();*/

	return(pd3dGraphicsRootSignature);
}

ID3D12RootSignature* CScene::CreateComputeRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 24; //t24: gtxtRenderTarget
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 0; //u0: RWTexture2D
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture2D
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //RWTexture2D
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 1;
	pd3dRootParameters[2].Constants.ShaderRegister = 7; //velocity
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	ID3D12RootSignature* pd3dComputeRootSignature = CreateRootSignature(pd3dDevice, d3dRootSignatureFlags, _countof(pd3dRootParameters), pd3dRootParameters, 0, NULL);

	return(pd3dComputeRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	//LIGHTS
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);

	//FOG
	ncbElementBytes = ((sizeof(FOG) + 255) & ~255); //256의 배수
	m_pd3dcbFog = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbFog->Map(0, NULL, (void**)&m_pcbMappedFog);

}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	//LIGHTS
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));

}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
	if (m_pd3dcbFog)
	{
		m_pd3dcbFog->Unmap(0, NULL);
		m_pd3dcbFog->Release();
	}
	if (m_pTextureRenderTarget)m_pTextureRenderTarget->Release();
}

void CScene::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pcbMappedFog->gvFogParameter.x != m_xmf4FogParameter.x || m_pcbMappedFog->gvFogParameter.z != m_xmf4FogParameter.z || m_pcbMappedFog->gvFogParameter.w != m_xmf4FogParameter.w) {
		//FOG
		::memcpy(&m_pcbMappedFog->gcFogColor, &m_xmf4FogColor, sizeof(XMFLOAT4));
		::memcpy(&m_pcbMappedFog->gvFogParameter, &m_xmf4FogParameter, sizeof(XMFLOAT4));

		D3D12_GPU_VIRTUAL_ADDRESS d3dcbFogGpuVirtualAddress = m_pd3dcbFog->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(17, d3dcbFogGpuVirtualAddress); //Fog
	}

	if (m_pd3dComputeRootSignature)pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);
	float vel = Vector3::Length(m_pPlayer->GetVelocity());
	float bias = 5.f;
	if(vel >50)
		bias = max(10.f - vel / 10.f, 0.f);
	pd3dCommandList->SetComputeRoot32BitConstants(2, 1, &bias, 0);
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain)m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();

	if(m_pScreenShader) m_pScreenShader->ReleaseUploadBuffers();
	if (m_pVertBlurShader) m_pVertBlurShader->ReleaseUploadBuffers();
	if (m_pHorzBlurShader) m_pHorzBlurShader->ReleaseUploadBuffers();

	if (m_pTextureRenderTarget) m_pTextureRenderTarget->ReleaseUploadBuffers();

}

void CScene::Shot(float fTimeElapsed)
{
	CBulletsShader* pShader = (CBulletsShader*)m_ppShaders[idxBulletsShader];
	pShader->Shot(m_pPlayer,fTimeElapsed);
}




bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case'0':
			if (m_xmf4FogParameter.x == LINEAR_FOG)
				m_xmf4FogParameter.z -= 10.f;
			else
				m_xmf4FogParameter.w += 0.001f;

			break;
		case'9':
			if (m_xmf4FogParameter.x == LINEAR_FOG)
				m_xmf4FogParameter.z += 10.f;
			else
				m_xmf4FogParameter.w /= 2;
			break;
		case VK_F4:
			if (m_xmf4FogParameter.x != LINEAR_FOG)
				m_xmf4FogParameter.x = LINEAR_FOG;
			break;
		case VK_F5:
			if (m_xmf4FogParameter.x != EXP_FOG)
				m_xmf4FogParameter.x = EXP_FOG;
			break;
		case VK_F6:
			if (m_xmf4FogParameter.x != EXP2_FOG)
				m_xmf4FogParameter.x = EXP2_FOG;
			break;
		case VK_F7:
			if (m_xmf4FogParameter.x != NO_FOG) {
				m_xmf4FogParameter.x = NO_FOG;
				gbFog = false;
			}
			break;

		default:
			break;
		}
		if (m_xmf4FogParameter.x != NO_FOG)
			gbFog = true;
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed, NULL);
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->UpdateTransform(NULL);

	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_ppShaders[idxObjectsShader]) {
		CObjectsShader* pShader = (CObjectsShader*)m_ppShaders[idxObjectsShader];
		pShader->AnimateExp(m_pPlayer->GetCamera()->GetPosition());
	}

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, const D3D12_CPU_DESCRIPTOR_HANDLE* pRtvCPUDescHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* pDsvCPUDescHandle, ID3D12Resource* pd3dSwapChainBackBuffer, CCamera *pCamera)
{

	UpdateShaderVariables(pd3dCommandList);

	//if (m_pd3dCbvSrvUavDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvUavDescriptorHeap);


	::SynchronizeResourceTransition(pd3dCommandList, m_pTextureRenderTarget->GetResource(0), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	FLOAT Color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, Color, 0, NULL);


	pd3dCommandList->ClearDepthStencilView(*pDsvCPUDescHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, pDsvCPUDescHandle);

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	m_pPlayer->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < m_nShaders; i++) 
		if (m_ppShaders[i]) m_ppShaders[i]->Render(pd3dCommandList, pCamera);
	::SynchronizeResourceTransition(pd3dCommandList, m_pTextureRenderTarget->GetResource(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

	::SynchronizeResourceTransition(pd3dCommandList, m_pTextureRenderTarget->GetResource(1), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//블러링

	if (m_pd3dCbvSrvUavDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvUavDescriptorHeap);

	if (m_pd3dComputeRootSignature) pd3dCommandList->SetComputeRootSignature(m_pd3dComputeRootSignature);
	if (m_pTextureRenderTarget) m_pTextureRenderTarget->UpdateComputeShaderVariables(pd3dCommandList);
	
	m_pHorzBlurShader->Dispatch(pd3dCommandList);
	m_pVertBlurShader->Dispatch(pd3dCommandList);

	::SynchronizeResourceTransition(pd3dCommandList, m_pTextureRenderTarget->GetResource(1), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList,CCamera* pCamera)
{
	if (m_pd3dCbvSrvUavDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvUavDescriptorHeap);

	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	m_pTextureRenderTarget->UpdateGraphicsShaderVariables(pd3dCommandList);

	m_pScreenShader->Render(pd3dCommandList, pCamera);
}





void CScene::CheckCollision()
{
	//맵<->플레이어 충돌검사
	BoundingBox MapBB = m_pTerrain->m_ppMeshes[0]->GetBBModel();
	CCamera* pCamera = m_pPlayer->GetCamera();
	if (!m_pPlayer->IsInMap(MapBB) && pCamera->GetMode() != FIRST_PERSON_CAMERA) {
		XMFLOAT3 pos = m_pPlayer->GetPosition();
		if (pos.x < MapBB.Center.x - MapBB.Extents.x) pos.x = MapBB.Center.x - MapBB.Extents.x;
		else if (pos.x > MapBB.Center.x + MapBB.Extents.x) pos.x = MapBB.Center.x + MapBB.Extents.x;
		if (pos.y < MapBB.Center.y - MapBB.Extents.y) pos.y = MapBB.Center.y - MapBB.Extents.y;
		else if (pos.y > MapBB.Center.y + MapBB.Extents.y) pos.y = MapBB.Center.y + MapBB.Extents.y;
		if (pos.z < MapBB.Center.z - MapBB.Extents.z) pos.z = MapBB.Center.z - MapBB.Extents.z;
		else if (pos.z > MapBB.Center.z + MapBB.Extents.z) pos.z = MapBB.Center.z + MapBB.Extents.z;
		m_pPlayer->SetPosition(pos);
	}
	//맵<->총알 충돌검사
	CBulletsShader* pBShader = (CBulletsShader*)m_ppShaders[idxBulletsShader];
	pBShader->CheckMap(MapBB,m_pTerrain);
	
	CObjectsShader* pOShader = (CObjectsShader*)m_ppShaders[idxObjectsShader];
	for (int i = 0; i < pBShader->GetnBullets(); ++i) {
		CGameObject* pObject = pOShader->GetGameObject(0);
		BoundingBox BBWorld = pObject->m_pChild->m_pChild->XMBBModel();
		BBWorld.Transform(BBWorld, XMLoadFloat4x4(&pObject->m_xmf4x4World));
		if (!pObject->m_bCollision && pBShader->CheckObject(BBWorld)) {
			pObject->m_bCollision = true;
		}
	}
}

void CScene::Update(float fTimeElapsed)
{

	CObjectsShader* pShader = (CObjectsShader*)m_ppShaders[idxObjectsShader];

	CheckCollision();
	if (pShader->GetGameObject(0)->m_bCollision) {
		pShader->GetGameObject(0)->Explode(fTimeElapsed);
	}
}
void CScene::CreateUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nDescriptorHeapIndex)
{
	m_d3dUavCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dUavGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	UINT nTextureType = pTexture->GetTextureType();
	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
		if (pShaderResource)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = pTexture->GetUnorderedAccessViewDesc(i);
			pd3dDevice->CreateUnorderedAccessView(pShaderResource, NULL, &d3dUnorderedAccessViewDesc, m_d3dUavCPUDescriptorNextHandle);
			m_d3dUavCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			pTexture->SetUavGpuDescriptorHandle(i, m_d3dUavGPUDescriptorNextHandle);
			m_d3dUavGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
}

void CScene::CreateUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nIndex)
{
	ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
	if (pShaderResource)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = pTexture->GetUnorderedAccessViewDesc(nIndex);
		pd3dDevice->CreateUnorderedAccessView(pShaderResource, NULL, &d3dUnorderedAccessViewDesc, m_d3dUavCPUDescriptorNextHandle);
		m_d3dUavCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		pTexture->SetUavGpuDescriptorHandle(nIndex, m_d3dUavGPUDescriptorNextHandle);
		m_d3dUavGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
	}
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nDescriptorHeapIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvUavDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	UINT nTextureType = pTexture->GetTextureType();
	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
		if (pShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			pTexture->SetSrvGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats)
{
	for (int i = 0; i < nResources; i++)
	{
		if (ppd3dResources[i])
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
			d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			d3dShaderResourceViewDesc.Format = pdxgiSrvFormats[i];
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			pd3dDevice->CreateShaderResourceView(ppd3dResources[i], &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
}

void CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nIndex)
{
	ID3D12Resource* pShaderResource = pTexture->GetResource(nIndex);
	if (pShaderResource)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(nIndex);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
		m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		pTexture->SetSrvGpuDescriptorHandle(nIndex, m_d3dSrvGPUDescriptorNextHandle);
		m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
	}
}

void CScene::CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvUavDescriptorHeap);

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dUavCPUDescriptorStartHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);
	m_d3dUavGPUDescriptorStartHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle;
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle;
	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;
	m_d3dUavCPUDescriptorNextHandle = m_d3dUavCPUDescriptorStartHandle;
	m_d3dUavGPUDescriptorNextHandle = m_d3dUavGPUDescriptorStartHandle;
}