#include "Map.h"

void Map::init_Map(Server* s, Timer* t)
{
	m_pServer = s;
	m_pTimer = t;

	memset(&over, 0, sizeof(over));
	over.is_recv = true;
	over.dataBuffer.len = BUFSIZE;
	over.dataBuffer.buf = over.messageBuffer;
	over.type = 1;
	over.roomID = roomnum;

	// 맵생성 이벤트 전달
	map_block_set p;
	p.type = EventType::Mapset;
	p.size = sizeof(p);
	p.key = roomnum;
	over.dataBuffer.len = sizeof(p);
	memcpy(over.dataBuffer.buf, reinterpret_cast<char*>(&p), sizeof(p));
	DWORD Transferred = 0;
	BOOL ret = PostQueuedCompletionStatus(m_pServer->Gethcp(), Transferred
		,roomnum, &over.overlapped);

	game_time = 0;

	//hMove = CreateEvent(NULL, TRUE, TRUE, NULL);
	ismove = true;
	//cloud_move();
}

void Map::Set_map()
{
	int num_count[3] = { 0 };
	int n;

	map_block_set p;
	p.type = PacketType::SC_map_set;
	p.size = sizeof(p);
	p.key = roomnum;

	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<int> dis1(0, 2);
	std::uniform_int_distribution<int> dis2(1000, 1100);

	for (int i = 0; i < MAX_MAP_BLOCK; i++)
	{
		while (1)
		{
			int n = rand() % 3;
			if (num_count[n] < 3)
			{
				num_count[n]++;
				Map_type[i] = dis1(gen);
				p.block_type[i] = Map_type[i];
				break;
			}
		}
	}
	for (int i = 0; i < 9; i++)
	{
		atm[i] = dis2(gen);
		if (Map_type[i] == terrain::Desert)
			atm[i] -= 100;
		else if (Map_type[i] == terrain::Snowy_field)
			atm[i] += 100;
		isMap_block[i] = TRUE;
	}

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	BuildMap(m_pd3dDevice, m_pd3dCommandList);

	over.dataBuffer.len = sizeof(p);
	strcpy_s(over.messageBuffer, reinterpret_cast<char*>(&p));
	m_pServer->send_packet_to_allplayers(p.key, reinterpret_cast<char*>(&p));

	ismove = true;
	Set_wind();
	Set_cloudpos();
	//print_Map();

	cloud_move();
}

void Map::Set_wind()
{
	for (int i = 0; i < 12; i++)
	{
		if (i < 6)
		{
			if (i == 0 || i == 1)
				wind[i] = calc_windpower(atm[i], atm[i + 1]) * 3.f;
			else if (i == 2 || i == 3)
				wind[i] = calc_windpower(atm[i + 1], atm[i + 2]) * 3.f;
			else if (i == 4 || i == 5)
				wind[i] = calc_windpower(atm[i + 2], atm[i + 3] * 3.f);
		}
		else
		{
			wind[i] = -calc_windpower(atm[i - 6], atm[i - 3] * 3.f);
		}
	}
}

void Map::Set_cloudpos()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<int> dis(10, MAP_SIZE - 5000);
	Cloud.x = dis(gen);
	Cloud.y = dis(gen);
}

void Map::print_Map()
{
	printf("\n");
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			printf("%d: %f | ", Map_type[(3*i)+j], atm[(3 * i) + j]);
		}
		printf("\n");
	}
	printf("\n");
	for (int i = 0; i < 12; i++)
		printf("%d | %f\n", i, wind[i]);

	printf("\n");
	//printf("x: %f | y: %f\n", Cloud.x, Cloud.y);
}

float Map::calc_windpower(float a, float b)
{
	return (a - b) / 5;
}

void Map::cloud_move()
{;
	for (int i = 0; i < 3; i++)
	{
		if (MAP_BLOCK_SIZE * i < Cloud.y && Cloud.y <= MAP_BLOCK_SIZE * (i + 1))
		{
			if (0 < Cloud.x && Cloud.x <= MAP_BLOCK_SIZE * 1.5f)
			{
				Cloud.x += wind[i * 2];
				//printf("%d", i * 2);

			}
			else if (MAP_BLOCK_SIZE * 1.5f < Cloud.x && Cloud.x < MAP_BLOCK_SIZE * 3)
			{
				Cloud.x += wind[i * 2 + 1];
				//printf("%d", i * 2 + 1);

			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (MAP_BLOCK_SIZE * i < Cloud.x && Cloud.x <= MAP_BLOCK_SIZE * (i + 1))
		{
			if (0 < Cloud.y && Cloud.y <= MAP_BLOCK_SIZE * 1.5f)
			{
				Cloud.y += wind[i + 6];
				//printf("%d", i + 6);

			}
			else if (MAP_BLOCK_SIZE * 1.5f < Cloud.y && Cloud.y < MAP_BLOCK_SIZE * 3)
			{
				Cloud.y += wind[i + 9];
				//printf("%d", i + 9);

			}
		}
	}

	if (Cloud.x < 0 || Cloud.y < 0) {
		Set_cloudpos();
	}


	cloud_move_packet p;
	p.type = EventType::Cloud_move;
	p.size = sizeof(cloud_move_packet);
	p.key = roomnum;
	p.roomid = roomnum;
	p.x = Cloud.x;
	p.z = Cloud.y;
	m_pTimer->push_event(roomnum, OE_gEvent, 1000, reinterpret_cast<char*>(&p));
	
	/*++game_time;
	if (game_time % MAP_BREAK_TIME == 0)
		Map_collapse();*/
		/*DWORD Transferred = 0;
		BOOL ret = PostQueuedCompletionStatus(m_pServer->Gethcp(), Transferred
			, (ULONG_PTR) & (roomnum), (LPOVERLAPPED)&over.overlapped);
		ismove = true;*/
}

void Map::Map_collapse()
{
	srand((unsigned int)time(NULL));
	int num = 0;
	while (1)
	{
		num = rand() % 9;

		if (isMap_block[num] == TRUE)
		{
			isMap_block[num] = FALSE;
			break;
		}
	}

	printf("collapse block: %d\n", num);
	//printf("cloud x: %f | y: %f\n\n", Cloud.x, Cloud.y);
	atm[num] = 1000;
	Set_wind();

	/*if (num%2 == 0)
	{
		if (num == 0)
			wind[0] = 0, wind[6] = 0;
		else if (num == 2)
			wind[1] = 0, wind[8] = 0;
		else if (num == 4)
			wind[2] = 0, wind[3] = 0, wind[7] = 0, wind[10] = 0;
		else if (num == 6)
			wind[4] = 0, wind[9] = 0;
		else if (num == 8)
			wind[5] = 0, wind[11] = 0;
	}
	else
	{
		if (num == 1)
			wind[0] = 0, wind[1] = 0, wind[7] = 0;
		else if (num == 3)
			wind[2] = 0, wind[6] = 0, wind[9] = 0;
		else if (num == 5)
			wind[3] = 0, wind[8] = 0, wind[11] = 0;
		else if (num == 7)
			wind[4] = 0, wind[5] = 0, wind[10] = 0;
	}*/
	m_pServer->send_map_collapse_packet(num, roomnum);
	//print_Map();

	if (num == 9) {
		m_pServer->game_end(roomnum);
	}
}

void Map::CreateDirect3DDevice()
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

void Map::CreateCommandQueueAndList()
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


void Map::BuildMap(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	std::vector<int> arrange{ 0, 0, -1, 0, 1, 0 };

	XMFLOAT3 xmf3Scale(8.0f, 4.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, NULL, _T("Terrain/Desert.raw"), 257, 257, xmf3Scale, xmf4Color, 0);
	m_pForestTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, NULL, _T("Terrain/Forest.raw"), 257, 257, xmf3Scale, xmf4Color, 1);
	m_pSnowTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, NULL, _T("Terrain/Snow.raw"), 257, 257, xmf3Scale, xmf4Color, 2);
	m_pForestTerrain->SetPosition(-2048.0f, 0.0f, 0.0f);
	m_pSnowTerrain->SetPosition(2048.0f, 125.0f, 0.0f);


	m_pTerrain->SetPosition(2048.0f * arrange[0], 0.0f, 2048.0f * arrange[1]);
	m_pForestTerrain->SetPosition(2048.0f * arrange[2], 0.0f, 2048.0f * arrange[3]);
	m_pSnowTerrain->SetPosition(2048.0f * arrange[4], 125.0f, 2048.0f * arrange[5]);

	m_pCMap = new CMap(pd3dDevice, pd3dCommandList, NULL, arrange);
}
