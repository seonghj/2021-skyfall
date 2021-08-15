#pragma once


#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "CPacket.h"

struct CB_FRAMEWORK_INFO
{
	float					m_fCurrentTime;
	float					m_fElapsedTime;
};

struct CB_FOG_INFO
{
	XMFLOAT4 gcFogColor;
	XMFLOAT4 gvFogParameter; //(Mode, Start, Range, Density)
	XMFLOAT2 gvFogPos;
};

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void CreateShadowMap();

	void ChangeSwapChainState();

    void BuildObjects();
    void ReleaseObjects();

    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void CreateShaderVariables();
	void UpdateShaderVariables();
	void ReleaseShaderVariables();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessageForLogIn(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void Set_m_pPacket(CPacket* t) { m_pPacket = t; };
	void CheckCollision();
	void SetCloud(float x, float z) { m_pcbMappedFog->gvFogPos = XMFLOAT2(x, z); }
	void Restart();
	void MouseHold(bool b) { m_bMouseHold = b; }
	void StartGame();
	void ShowLoginWindow();
	void ShowLobbyWindow();
	void ShowRoomWindow();
	void ShowAccountWindow(bool* p_open);
	void ShowCreateRoomWindow();
	void ShowError(const char* str);

	XMFLOAT3					m_BeforePosition;
	float						m_DegreeX;
	float						m_DegreeY;
	float						m_DegreeZ;

	CTerrainPlayer*				m_pPlayer = NULL;
	CCamera*					m_pCamera = NULL;
	CScene*						m_pScene = NULL;

	unordered_map<int, char[20]>	rooms;

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
        
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;
	UINT						m_nDsvDescriptorIncrementSize;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;
	CGameTimer					m_ChargeTimer;

	CTerrainPlayer				*m_ppOtherPlayer[OTHER_PLAYER_NUM];

	CPacket						*m_pPacket = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	bool						m_bRotateEnable = 0;
	float						m_fPitch = 0;
	float						m_fYaw = 0;

	CBowPlayer					*m_pBowPlayer;
	C1HswordPlayer				*m_p1HswordPlayer;

	bool						m_bMouseHold = false; 
	vector<vector<int>>			m_vMapArrange;


	ID3D12Resource				*m_pd3dcbFrameworkInfo = NULL;
	CB_FRAMEWORK_INFO			*m_pcbMappedFrameworkInfo = NULL;

	ID3D12Resource				*m_pd3dcbFog = NULL;
	CB_FOG_INFO					*m_pcbMappedFog = NULL;

	DWORD						dwDirection = 0;
	BOOL						PressDirButton = false;

	//Font
	unique_ptr<SpriteBatch>		m_pSprite;
	unique_ptr<SpriteFont>		m_pFont;
	unique_ptr<DescriptorHeap> m_resourceDescriptors;
	unique_ptr<GraphicsMemory> m_graphicsMemory;

	void DrawTimer();

	enum Descriptors
	{
		SegoeFont,
		ImGui,
		Count = 256
	};

	//imgui
	void CreateFontAndGui();
	char m_bufID[11];
	char m_bufPW[21];
	bool m_bShowAccountWindow = false;
	bool m_bShowCreateRoomWindow = false;
	bool m_bError = false;
	vector<string> m_vRooms;
	string m_ErrorMsg;

	//miniMap
	void RenderMiniMap() const;
	void BuildMiniMap();
	void UpdateMiniMap();
	CUIObject* m_pMiniMap = NULL;
	CTexture* m_pMiniMapTexture = NULL;
	CCamera* m_pMiniMapCamera = NULL;

	//Shadow
	CShadowMap* m_pShadowMap;
	void BuildShadowMap();
	void UpdateShadowMap();

};

