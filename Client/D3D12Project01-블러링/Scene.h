//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

#define MAX_LIGHTS			16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define NO_FOG 0.0f
#define LINEAR_FOG 1.0f
#define EXP_FOG 2.0f
#define EXP2_FOG 3.0f

struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
	int						m_nLights;
};

struct FOG
{
	XMFLOAT4 gcFogColor;
	XMFLOAT4 gvFogParameter; //(Mode, Start, Range, Density)
};

class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature* CreateRootSignature(ID3D12Device* pd3dDevice, D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags, UINT nRootParameters, D3D12_ROOT_PARAMETER* pd3dRootParameters, UINT nStaticSamplerDescs, D3D12_STATIC_SAMPLER_DESC* pd3dStaticSamplerDescs);
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);

	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	ID3D12RootSignature* GetComputeRootSignature() { return(m_pd3dComputeRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, const D3D12_CPU_DESCRIPTOR_HANDLE* pRtvCPUDescHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* pDsvCPUDescHandle,ID3D12Resource* pd3dSwapChainBackBuffer, CCamera* pCamera = NULL);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void ReleaseUploadBuffers();

	CPlayer								*m_pPlayer = NULL;

public:
	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;
	ID3D12RootSignature					*m_pd3dComputeRootSignature = NULL;

	int									m_nGameObjects = 0;
	CGameObject							**m_ppGameObjects = NULL;

	int									m_nShaders = 0;
	CShader								**m_ppShaders = NULL;

	CHeightMapTerrain					*m_pTerrain = NULL;

	CSkyBox								*m_pSkyBox = NULL;

	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;


	XMFLOAT4							m_xmf4FogColor;
	XMFLOAT4							m_xmf4FogParameter;

	ID3D12Resource						*m_pd3dcbFog = NULL;
	FOG									*m_pcbMappedFog = NULL;

	CTexture2							*m_pTextureRenderTarget = NULL;

	CBlurringShader						*m_pHorzBlurShader = NULL;
	CBlurringShader						*m_pVertBlurShader = NULL;
	CTextureToFullScreenShader			*m_pScreenShader = NULL;

	ID3D12DescriptorHeap* m_pd3dCbvSrvUavDescriptorHeap = NULL;
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dUavCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dUavCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGPUDescriptorNextHandle;

public:
	void Shot(float fTimeElapsed);

	void CheckCollision();
	void Update(float fTimeElapsed);

	void CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);

	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nDescriptorHeapIndex);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats);
	void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nIndex);
	void CreateUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nDescriptorHeapIndex);
	void CreateUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture2* pTexture, UINT nIndex);


#define idxObjectsShader 1
#define idxBulletsShader 2
};
