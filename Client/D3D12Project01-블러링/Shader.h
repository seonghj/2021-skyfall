//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"



class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int									m_nReferences = 0;

protected:

	ID3D12PipelineState					**m_ppd3dPipelineStates = NULL;
	int									m_nPipelineStates = 0;

	ID3D12DescriptorHeap				*m_pd3dCbvSrvUavDescriptorHeap = NULL;

	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;
	ID3D12RootSignature					*m_pd3dComputeRootSignature = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dUavCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dUavGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dUavCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dUavGPUDescriptorNextHandle;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateDomainShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateHullShader(ID3DBlob** ppd3dShaderBlob);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob=NULL);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World) { }

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void ReleaseUploadBuffers() { }

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }

	void CreateCbvSrvUavDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex);
	void CreateUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex);
	void CreateUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetUavCPUDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavGPUDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardShader : public CShader
{
public:
	CStandardShader();
	virtual ~CStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CObjectsShader : public CStandardShader
{
public:
	CObjectsShader();
	virtual ~CObjectsShader();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	
	void SetTerrain(void* pContext);

protected:
	CGameObject						**m_ppObjects = 0;
	int								m_nObjects = 0;
	int								m_nCheckPoint = 0;
	int								m_direction = 1;
	CShader*						m_pExpShader;

public:
	CGameObject* GetGameObject(int idx) { return m_ppObjects[idx]; }
	int GetnObjects() { return m_nObjects; }
	void AnimateExp(XMFLOAT3& xmf3Target);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CPlayerShader : public CShader
{
public:
	CPlayerShader();
	virtual ~CPlayerShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual D3D12_BLEND_DESC CreateBlendState();

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CTexturedShader : public CShader
{
public:
	CTexturedShader();
	virtual ~CTexturedShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	virtual void AnimateObjects(float fTimeElapsed);

	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE d3dPrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

protected:
	CWaterGridMesh *m_pWaterGridMesh = NULL;
	CTexture* m_pWaterTexture = 0;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CBillboardObjectsShader : public CShader
{
public:
	CBillboardObjectsShader();
	virtual ~CBillboardObjectsShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** ppd3dShaderBlob);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL);
	virtual void ReleaseObjects();
	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
protected:
	CGeometryBillboardMesh* m_pGeometryBillboardMesh = 0;
	CTexture* m_pBillboardTexture = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CTerrainShader : public CShader
{
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);


};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CTerrainTessellationShader : public CShader
{
public:
	CTerrainTessellationShader();
	virtual ~CTerrainTessellationShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateDomainShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateHullShader(ID3DBlob** ppd3dShaderBlob);

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CBulletsShader : public CShader
{
public:
	CBulletsShader();
	virtual ~CBulletsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

protected:
	CMesh * m_pBulletMesh = 0;
	int m_nBullets = 0;
	CBullet** m_ppBullets = 0;
	float m_time = 0;
	void DeleteBullet(const int& idx);
public:
	void CheckMap(BoundingBox& MapBB, void* pContext);
	bool CheckObject(BoundingBox& BBWorld);
	void Shot(CPlayer* pPlayer, float fTimeElapsed, float fSpeed);
	int GetnBullets() const { return m_nBullets; } 
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CExplosionShader :public CShader
{
public:
	CExplosionShader();
	virtual ~CExplosionShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);

protected:
	CTexture* m_pExpTexture = 0;
	CExplosionBillboardMesh* m_pExpMesh = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CComputeShader : public CShader
{
public:
	CComputeShader();
	virtual ~CComputeShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups);

	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups);
	void OnPrepare(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
	UINT							m_cxThreadGroups = 0;
	UINT							m_cyThreadGroups = 0;
	UINT							m_czThreadGroups = 0;
};

class CBlurringShader : public CComputeShader
{
public:
	CBlurringShader();
	virtual ~CBlurringShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);


	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	/*virtual void Dispatch(ID3D12GraphicsCommandList* pd3dCommandList, UINT cxThreadGroups, UINT cyThreadGroups, UINT czThreadGroups);*/
};

class CHorzBlurShader :public CBlurringShader
{
public:
	CHorzBlurShader();
	virtual ~CHorzBlurShader();
	
public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);

};

class CVertBlurShader :public CBlurringShader
{
public:
	CVertBlurShader();
	virtual ~CVertBlurShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


class CTextureToFullScreenShader : public CShader
{
public:
	CTextureToFullScreenShader();
	virtual ~CTextureToFullScreenShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void ReleaseShaderVariables();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CShadowMapShader : public CShader
{
public:
	CShadowMapShader(ID3D12Device* device, UINT width, UINT height);
	virtual ~CShadowMapShader();

	UINT Width()const;
	UINT Height()const;
	ID3D12Resource* Resource();
	D3D12_GPU_DESCRIPTOR_HANDLE Srv()const;
	D3D12_CPU_DESCRIPTOR_HANDLE Dsv()const;

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	void BuildDescriptors(
		D3D12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		D3D12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		D3D12_CPU_DESCRIPTOR_HANDLE hCpuDrv);

	void OnResize(UINT newWidth, UINT newHeight);

private:
	void BuildDescriptors();
	void BuildResource();

private:

	ID3D12Device* m_pd3dDevice = nullptr;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	D3D12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE mhCpuDrv;

	ComPtr<ID3D12Resource> m_pShadowMap = nullptr;
};
