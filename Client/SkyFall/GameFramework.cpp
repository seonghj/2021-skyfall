﻿//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"

#define NO_FOG 0.0f
#define LINEAR_FOG 1.0f
#define EXP_FOG 2.0f
#define EXP2_FOG 3.0f

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pScene = NULL;
	m_pPlayer = NULL;
	m_pPacket = NULL;

	m_ptOldCursorPos.x = FRAME_BUFFER_WIDTH / 2;
	m_ptOldCursorPos.y = FRAME_BUFFER_HEIGHT / 2;
	_tcscpy_s(m_pszFrameRate, _T("SkyFall ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();
	CreateShaderVariables();
	CoInitialize(NULL);

	BuildObjects();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
#endif
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);

	IDXGIAdapter1* pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pd3dAdapter));
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	::gnCbvSrvUavDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::CreateShadowMap()
{
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN: {
		if (!strcmp(m_pPlayer->m_pstrFrameName, "Player_1Hsword"))
			m_pPacket->Send_attack_packet(PlayerAttackType::SWORD1HL);
		else if (!strcmp(m_pPlayer->m_pstrFrameName,"Player_Bow"))
			m_pPacket->Send_attack_packet(PlayerAttackType::BOWL);
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		if (!m_bRotateEnable) {
			m_ChargeTimer.Reset();
			m_ChargeTimer.Start();
			//m_pPlayer->LButtonDown();
		}
		break;
	}
	case WM_RBUTTONDOWN: {
		if (!strcmp(m_pPlayer->m_pstrFrameName, "Player_1Hsword"))
			m_pPacket->Send_attack_packet(PlayerAttackType::SWORD1HR);
		else if (!strcmp(m_pPlayer->m_pstrFrameName, "Player_Bow"))
			m_pPacket->Send_attack_packet(PlayerAttackType::BOWR);
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		if (!m_bRotateEnable) {
			m_ChargeTimer.Reset();
			m_ChargeTimer.Start();
		}
		break;
	}
	case WM_LBUTTONUP: {
		if (!strcmp(m_pPlayer->m_pstrFrameName, "Player_1Hsword"))
			m_pPacket->Send_stop_packet();
		::ReleaseCapture();
		m_ChargeTimer.Stop();
		m_pPlayer->LButtonUp();
		break;
	}
	case WM_RBUTTONUP: {
		if (!strcmp(m_pPlayer->m_pstrFrameName, "Player_1Hsword"))
			m_pPacket->Send_stop_packet();
		m_pPlayer->RButtonUp();
		CCamera* pCamera = m_pPlayer->GetCamera();
		m_pCamera = pCamera;
		m_DegreeX = 0;
		m_DegreeY = 0;
		break;
	}
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
		case WM_KEYUP:
			switch (wParam)
			{
				case VK_CONTROL:
					m_bRotateEnable = false;
					//m_pCamera->Rotate(-m_fPitch, -m_fYaw, 0);
					m_fPitch = 0;
					m_fYaw = 0;
					break;
				case VK_ESCAPE:
					::PostQuitMessage(0);
					break;
				case VK_RETURN:
					break;
				case VK_F1:
				case VK_F2:
				case VK_F3:
					m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
					break;
				case VK_F4: 
					m_pPacket->Send_swap_weapon_packet(PT_BOW);
					break;
				case VK_F5: 
					m_pPacket->Send_swap_weapon_packet(PT_SWORD1H);
					break;
				case VK_F6:
					m_bMouseHold = !m_bMouseHold;
					break;
				case VK_F9:
					ChangeSwapChainState();
					break;
				case VK_UP:
					XMFLOAT3 pos = m_pPlayer->GetPosition();
					m_pPlayer->SetPosition(XMFLOAT3(pos.x, 1000.0f, pos.z));
					m_pPlayer->SetGravity(XMFLOAT3(0.0f, -400.f, 0.0f));
					break;
				default:
					break;
			}
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_CONTROL:
				m_bRotateEnable = true;
				break;
			case VK_UP:
				XMFLOAT3 pos = m_pPlayer->GetPosition();
				m_pPlayer->SetPosition(XMFLOAT3(pos.x, 1000.0f, pos.z));
				m_pPlayer->SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
				break;
			case VK_HOME:
				m_pShadowMap->Plus();
				break;
			case VK_END:
				m_pShadowMap->Minus();
				break;
			}
			break;
		default:
			break;
	}
}

void CGameFramework::OnProcessingKeyboardMessageForLogIn(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:

		break;
	case WM_KEYDOWN:

		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::CheckCollision()
{
	m_pScene->CheckCollision();
	//m_pScene->CheckTarget();
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pScene = new CScene();

	m_vMapArrange = { { -1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1} };
	if (m_pScene) m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_vMapArrange);

	m_pd3dCommandList->SetGraphicsRootSignature(m_pScene->GetGraphicsRootSignature());
	CCamera* pCamera = new CCamera();
	pCamera->SetPosition(m_pScene->m_pLights[0].m_xmf3Position);
	pCamera->SetLookVector(m_pScene->m_pLights[0].m_xmf3Direction);
	pCamera->RegenerateViewMatrix();
	pCamera->GenerateProjectionMatrixOrtho(1.01f, 2000.0f, m_nWndClientWidth*2, m_nWndClientHeight*2);
	pCamera->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

	m_pShadowMap = new CShadowMap(m_nWndClientWidth, m_nWndClientHeight, pCamera);
	m_pShadowMap->CreateShader(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature());
	m_pShadowMap->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	m_pShadowMap->CreateShadowMap(m_pd3dDevice);

	CLoadedModelInfo* pSwordModel = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), "Model/Player/Player_1Hsword.bin", NULL);
	C1HswordPlayer* p1HswordPlayer = new C1HswordPlayer(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), pSwordModel, (void**)m_pScene->m_ppTerrain);
	
	m_p1HswordPlayer = p1HswordPlayer;
	if (pSwordModel) delete pSwordModel;

	m_pScene->AddPlayer(m_pd3dDevice, m_pd3dCommandList);
	m_pScene->m_pPlayer = m_p1HswordPlayer;
	m_pPlayer = m_pScene->m_pPlayer;
	for (int i = 0; i < 20; ++i)
		m_pScene->MovePlayer(i, XMFLOAT3(80.0f, 50.0f, 0.0f));

	m_pPlayer->SetPlace(4);
	m_pCamera = m_pPlayer->GetCamera();
	m_pPacket->m_pScene = m_pScene;
	m_pPacket->m_pFramework = this;
	m_pPacket->m_pPlayer = m_pPlayer;
	m_pPacket->m_pMap = m_pScene->m_pMap;

	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pScene) m_pScene->ReleaseUploadBuffers();
	if (p1HswordPlayer) p1HswordPlayer->ReleaseUploadBuffers();

	for (int i = 0; i < MAX_PLAYER; i++) {
		if (m_pScene->m_mBowPlayer[i]) {
			m_pScene->m_mBowPlayer[i]->ReleaseUploadBuffers();
		}
	}
	for (int i = 0; i < MAX_PLAYER; i++) {
		if (m_pScene->m_m1HswordPlayer[i]) {
			m_pScene->m_m1HswordPlayer[i]->ReleaseUploadBuffers();
		}
	}

	m_GameTimer.Reset();
	m_ChargeTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer) m_pPlayer->Release();
	/*if (m_p1HswordPlayer) m_p1HswordPlayer->Release();
	if (m_pBowPlayer) m_pBowPlayer->Release();*/

	if (m_pScene) {
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}
	if (m_pShadowMap) {
		m_pShadowMap->ReleaseObjects();
		delete m_pShadowMap;
	}
}

void CGameFramework::ProcessInput()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = true;
	if (GetKeyboardState(pKeysBuffer) && m_pScene) bProcessedByScene = m_pScene->ProcessInput(pKeysBuffer);
	if (!bProcessedByScene)
	{
		DWORD dwDirection = 0;

		// ���߿� ī�޶� �̵��� �����
		/*if (pKeysBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeysBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeysBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeysBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeysBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeysBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;*/
		if (m_pPlayer->GetGround()) {
			if (pKeysBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeysBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeysBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeysBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
			/*if (pKeysBuffer['Q'] & 0xF0) dwDirection |= DIR_UP;
			if (pKeysBuffer['E'] & 0xF0) dwDirection |= DIR_DOWN;*/


			if (pKeysBuffer[VK_SPACE] & 0xF0)
			{
				//m_pPlayer->SetJump(true);
				player_move_packet p;
				p.key = m_pPacket->Get_clientkey();
				p.dx = m_DegreeX;
				p.dy = m_DegreeY;
				//p.MoveType = dwDirection;
				p.size = sizeof(p);
				p.state = 1;
				p.MoveType = PlayerMove::JUMP;
				p.type = CS_player_move;
				m_pPacket->SendPacket(reinterpret_cast<char*>(&p));
				m_pPlayer->SetFriction(0.f);
			}
			else if (pKeysBuffer[VK_SHIFT] & 0xF0)
			{
				m_pPlayer->SetRunning(true);
			}
		}
		
		if (!(pKeysBuffer[VK_SHIFT] & 0xF0) && m_pPlayer->GetRunning())
		{
			m_pPlayer->SetRunning(false);
		}

		if (((pKeysBuffer[VK_LCONTROL] & 0xF0) && m_pCamera->GetMode() == THIRD_PERSON_CAMERA) ||
			((pKeysBuffer[VK_LBUTTON] & 0xF0) && m_pCamera->GetMode() == SPACESHIP_CAMERA)) {
			//m_pScene->Shot(fTimeElapsed, 300.f);
		}

		float cxDelta = 0.0f, cyDelta = 0.0f;
		if ((GetCapture() == m_hWnd || m_pCamera->GetMode() == FIRST_PERSON_CAMERA) || m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			if(!m_bMouseHold)
				SetCursor(NULL);
			POINT ptCursorPos;
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			if(!m_bMouseHold)
				SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if (m_pPlayer->GetAttack() && m_pCamera->GetMode() == THIRD_PERSON_CAMERA&&
			!strcmp(m_pPlayer->m_pstrFrameName,"Player_Bow"))
		{
			player_shot_packet p;
			p.key = m_pPacket->Get_clientkey();
			p.size = sizeof(p);
			p.type = CS_allow_shot;
			p.Look = m_pCamera->GetLookVector();
			p.fTimeElapsed = fTimeElapsed;
			p.ChargeTimer = m_ChargeTimer.GetTotalTime();
			m_pPacket->SendPacket(reinterpret_cast<char*>(&p));
			//printf("Look - X : %f Y : %f Z : %f\n", m_pCamera->GetLookVector().x, m_pCamera->GetLookVector().y, m_pCamera->GetLookVector().z);
		}

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (m_bRotateEnable) {
					m_fPitch += cyDelta;
					m_fYaw += cxDelta;
					//m_pCamera->Rotate(cyDelta, cxDelta, 0);
					m_pShadowMap->Rotate(cyDelta, cxDelta, 0);
				}
				else {
					m_DegreeX = cyDelta;
					m_DegreeY = cxDelta;
				}
			}
			if (dwDirection && (false == m_pPlayer->GetAttack())) {
				m_pPlayer->Move(dwDirection, 50.25f, true);

				//int Yaw = 0;
				//if (dwDirection & DIR_BACKWARD) {
				//	Yaw = 180;
				//	if (dwDirection & DIR_LEFT) {
				//		Yaw += 45;
				//	}
				//	else if (dwDirection & DIR_RIGHT) {
				//		Yaw -= 45;
				//	}
				//}
				//else if (dwDirection & DIR_FORWARD) {
				//	Yaw = 0.f;
				//	if (dwDirection & DIR_LEFT) {
				//		Yaw -= 45;
				//	}
				//	else if (dwDirection & DIR_RIGHT) {
				//		Yaw += 45;
				//	}
				//}
				//else if (dwDirection & DIR_RIGHT) {
				//	Yaw = 90;
				//}
				//else if (dwDirection & DIR_LEFT) {
				//	Yaw = -90;
				//}
				//if (m_pPlayer->m_iRotate != Yaw) {
				//	//m_pPlayer->RotatePlayer(Yaw - m_pPlayer->m_iRotate);
				//	m_pPlayer->m_iRotate = Yaw;
				//}
			}
		}
	}
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	if (m_pScene) {
		m_pScene->AnimateObjects(fTimeElapsed);

		//// 임시방편
		//if (m_pPlayer->GetPosition().x < 0) {
		//	m_pPlayer->SetPlayerUpdatedContext(m_pScene->m_pForestTerrain);
		//}
		//else if (m_pPlayer->GetPosition().x < 2048) {
		//	m_pPlayer->SetPlayerUpdatedContext(m_pScene->m_pTerrain);
		//}
		//else {
		//	m_pPlayer->SetPlayerUpdatedContext(m_pScene->m_pSnowTerrain);
		//}
		XMFLOAT3 pos = m_pPlayer->GetPosition();
		int nPlace = m_pPlayer->GetPlace();
		if (pos.x < m_vMapArrange[nPlace][0] * 2048 && nPlace % 3>0) {
			m_pPlayer->SetPlace(nPlace - 1);
		}
		else if (pos.x > (m_vMapArrange[nPlace][0] + 1) * 2048 && nPlace % 3 < 2) {
			m_pPlayer->SetPlace(nPlace + 1);
		}

		if (pos.z < m_vMapArrange[nPlace][1] * 2048&&nPlace>2) {
			m_pPlayer->SetPlace(nPlace - 3);
		}
		else if (pos.z > (m_vMapArrange[nPlace][1] + 1) * 2048 && nPlace < 6) {
			m_pPlayer->SetPlace(nPlace + 3);
		}
	}
	/*if(m_p1HswordPlayer)
		m_p1HswordPlayer->Animate(fTimeElapsed);
	if (m_pBowPlayer)
		m_pBowPlayer->Animate(fTimeElapsed);*/
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	//m_nSwapChainBufferIndex = (m_nSwapChainBufferIndex + 1) % m_nSwapChainBuffers;

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP
void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(60.0f);


	ProcessInput();
	CheckCollision();
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
	CCamera* pCamera = m_pShadowMap->GetCamera();
	pCamera->SetPosition(XMFLOAT3(m_pPlayer->GetPosition().x, 0, m_pPlayer->GetPosition().z));
	pCamera->Move(XMFLOAT3(-500, 200, -500));
	pCamera->RegenerateViewMatrix();
    AnimateObjects();

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pShadowMap->Set(m_pd3dCommandList);
	m_pScene->Set(m_pd3dCommandList);

	UpdateShaderVariables();
	m_pShadowMap->UpdateShaderVariable(m_pd3dCommandList);

	m_pShadowMap->Render(m_pd3dCommandList, NULL);
	if (m_pScene) m_pScene->RenderShadow(m_pd3dCommandList, m_pShadowMap->GetCamera());
	m_pPlayer->RenderShadow(m_pd3dCommandList, NULL);

	//if (m_pBowPlayer) m_pBowPlayer->RenderShadow(m_pd3dCommandList, m_pCamera);
	//if (m_p1HswordPlayer) m_p1HswordPlayer->RenderShadow(m_pd3dCommandList, m_pCamera);

	m_pShadowMap->Reset(m_pd3dCommandList);

	::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	//D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	//::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	//d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	//d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	m_pShadowMap->UpdateShaderVariables(m_pd3dCommandList);
	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif
	/*if (m_pBowPlayer) m_pBowPlayer->Render(m_pd3dCommandList, m_pCamera);
	if (m_p1HswordPlayer) m_p1HswordPlayer->Render(m_pd3dCommandList, m_pCamera);*/


	::SynchronizeResourceTransition(m_pd3dCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	/*d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);*/

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	//	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	MoveToNextFrame();

	XMFLOAT3 NowPosition = m_pPlayer->GetPosition();
	if (m_BeforePosition.x != NowPosition.x || m_BeforePosition.y != NowPosition.y || m_BeforePosition.z != NowPosition.z
		|| m_DegreeX != 0.0f || m_DegreeY != 0.0f)
	{
		/*printf("N: %f, %f, %f | B: %f, %f, %f\n", NowPosition.x, NowPosition.y, NowPosition.z,
			m_BeforePosition.x, m_BeforePosition.y, m_BeforePosition.z);*/
		if (false == m_pPlayer->GetAttack()) {
			player_pos_packet p;
			p.key = m_pPacket->Get_clientkey();
			p.Position = NowPosition;
			p.dx = m_DegreeX;
			p.dy = m_DegreeY;

			//p.MoveType = dwDirection;
			p.size = sizeof(p);
			p.state = 1;
			if (m_BeforePosition.x == NowPosition.x && m_BeforePosition.y == NowPosition.y && m_BeforePosition.z == NowPosition.z) {
				p.MoveType = PlayerMove::STAND;
				m_pPlayer->SetStanding(true);
			}
			else {
				if (m_pPlayer->GetJump() == true || m_pPlayer->GetGround() == false)
					p.MoveType = PlayerMove::JUMP;
				else
					p.MoveType = m_pPlayer->GetRunning();
				m_pPlayer->SetStanding(false);
			}
			p.type = CS_player_pos;
			
			if (m_pPacket->canmove == TRUE)
				m_pPacket->SendPacket(reinterpret_cast<char*>(&p));

			m_BeforePosition = NowPosition;

			m_DegreeX = 0.0f;
			m_DegreeY = 0.0f;

		}
	}
	else {
		if (false == m_pPlayer->GetStanding()) {
			m_pPlayer->SetStanding(true);
			player_stop_packet sp;
			sp.key = m_pPacket->Get_clientkey();
			sp.size = sizeof(sp);
			sp.type = PacketType::CS_player_stop;
			sp.Position = m_pPlayer->GetPosition();
			m_pPacket->SendPacket(reinterpret_cast<char*>(&sp));
		}
	}

	m_GameTimer.GetFrameRate(m_pszFrameRate + 9, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::CreateShaderVariables()
{
	//FRAMEWORK
	UINT ncbElementBytes = ((sizeof(CB_FRAMEWORK_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbFrameworkInfo = ::CreateBufferResource(m_pd3dDevice, m_pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbFrameworkInfo->Map(0, NULL, (void**)&m_pcbMappedFrameworkInfo);

	//FOG
	ncbElementBytes = ((sizeof(CB_FOG_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbFog = ::CreateBufferResource(m_pd3dDevice, m_pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbFog->Map(0, NULL, (void**)&m_pcbMappedFog);

	m_pcbMappedFog->gcFogColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.2f);
	m_pcbMappedFog->gvFogParameter = XMFLOAT4(/*EXP_FOG*/NO_FOG, 1.f, 300.f, 0.001f);
}

void CGameFramework::UpdateShaderVariables()
{
	m_pcbMappedFrameworkInfo->m_fCurrentTime = m_GameTimer.GetTotalTime();
	m_pcbMappedFrameworkInfo->m_fElapsedTime = m_GameTimer.GetTimeElapsed();

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbFrameworkInfo->GetGPUVirtualAddress();
	m_pd3dCommandList->SetGraphicsRootConstantBufferView(17, d3dGpuVirtualAddress);


	D3D12_GPU_VIRTUAL_ADDRESS d3dcbFogGpuVirtualAddress = m_pd3dcbFog->GetGPUVirtualAddress();
	m_pd3dCommandList->SetGraphicsRootConstantBufferView(18, d3dcbFogGpuVirtualAddress); //Fog

}

void CGameFramework::ReleaseShaderVariables()
{
	if (m_pd3dcbFrameworkInfo)
	{
		m_pd3dcbFrameworkInfo->Unmap(0, NULL);
		m_pd3dcbFrameworkInfo->Release();
	}
	if (m_pd3dcbFog)
	{
		m_pd3dcbFog->Unmap(0, NULL);
		m_pd3dcbFog->Release();
	}
}
