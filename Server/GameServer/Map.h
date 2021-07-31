#pragma once
#include "stdafx.h"
#include "Server.h"
#include "protocol.h"
#include "Mesh.h"
#include "Object.h"
#include <tchar.h>


//struct OVER_EX;
class Server;
class Timer;

class Map{
public:
	Map() { }
	Map(const int& num):roomnum(num) { }
	~Map() {}

	OVER_EX     over;

	Vector2D Cloud;
	HANDLE hMove;

	int Map_type[MAX_MAP_BLOCK];

	float atm[9] = { 0 };
	float wind[12] = { 0 };

	bool isMap_block[9];

	bool ismove;

	void SetNum(int n) { roomnum = n; }

	void init_Map(Server* s, Timer* t);

	void Set_map();
	void Set_wind();
	void Set_cloudpos();

	void print_Map();

	float calc_windpower(float a, float b);
	void cloud_move();
	void Map_collapse();

	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	void BuildMap(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

private:
	int game_time;
	Server* m_pServer = NULL; 
	Timer* m_pTimer = NULL;
	int roomnum = -1;


	IDXGIFactory4* m_pdxgiFactory = NULL;
	IDXGISwapChain3* m_pdxgiSwapChain = NULL;
	ID3D12Device* m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;
	UINT						m_nDsvDescriptorIncrementSize;

	ID3D12CommandAllocator* m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue* m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;

	ID3D12Fence* m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;

	CHeightMapTerrain* m_pTerrain = NULL;
	CHeightMapTerrain* m_pForestTerrain = NULL;
	CHeightMapTerrain* m_pSnowTerrain = NULL;

	CMap* m_pCMap = NULL;
};