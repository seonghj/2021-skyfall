//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
//#include "protocol.h"
#include "CPacket.h"

ID3D12DescriptorHeap *CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dUavGPUDescriptorNextHandle;

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
	m_pLights[0].m_fRange = 5000.0f;
	/*m_pLights[0].m_xmf4Ambient = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);*/
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(-100.0f, 500.0f, -100.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.4f, -1.0f, 0.4f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = false;
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
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 1.0f);
	m_pLights[3].m_bEnable = false;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 1200.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(50.0f, 1000.0f, 30.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, -1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, vector<vector<int>> arrange)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CreateCbvSrvUavDescriptorHeaps(pd3dDevice, 1000, 5000,2); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	BuildDefaultLightsAndMaterials();

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3Scale(8.0f, 4.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);

	//m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/Desert.raw"), 257, 257, xmf3Scale, xmf4Color, 0);
	//m_pForestTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/Forest.raw"), 257, 257, xmf3Scale, xmf4Color, 1);
	//m_pSnowTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/Snow.raw"), 257, 257, xmf3Scale, xmf4Color, 2);
	//m_pTerrain->SetPosition(2048.0f * arrange[0], 0.0f, 2048.0f * arrange[1]);
	//m_pForestTerrain->SetPosition(2048.0f * arrange[2], 0.0f, 2048.0f * arrange[3]);
	//m_pSnowTerrain->SetPosition(2048.0f * arrange[4], 125.0f, 2048.0f * arrange[5]);
	//m_pForestTerrain->SetPosition(-2048.0f, 0.0f, 0.0f);
	//m_pSnowTerrain->SetPosition(2048.0f, 125.0f, 0.0f);

	for (int i = 0; i < 9; i++)
	{
		if (i < 3)
		{
			m_ppTerrain[i] = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/Desert_0807.raw"), 257, 257, xmf3Scale, xmf4Color, 0);
		}
		else if (i >= 3 && i < 6)
		{
			m_ppTerrain[i] = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/Forest_0807.raw"), 257, 257, xmf3Scale, xmf4Color, 1);
		}
		else if (i >= 6)
		{
			m_ppTerrain[i] = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/Snow_0807.raw"), 257, 257, xmf3Scale, xmf4Color, 2);
		}
		m_ppTerrain[i]->SetPosition(2048.0f * arrange[i][0], 0.0f, 2048.0f * arrange[i][1]);
		m_ppTerrain[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		if (i >= 6) m_ppTerrain[i]->MoveUp(125.f);
	}

	{
		m_nGameObjects = 15;
		m_ppGameObjects = new CMonster * [m_nGameObjects];

		CLoadedModelInfo* pDragonModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Monster/Dragon.bin", NULL);
		CLoadedModelInfo* pWolfModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Monster/Wolf.bin", NULL);
		CLoadedModelInfo* pMetalonModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Monster/Metalon.bin", NULL);

		m_ppGameObjects[0] = new CDragon(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pDragonModel, 16, (void**)m_ppTerrain, 4);
		m_ppGameObjects[0]->m_nkey = 0;
		for (int i = 1; i < 8; ++i) {
			CLoadedModelInfo* pModel = new CLoadedModelInfo(*pWolfModel);
			m_ppGameObjects[i] = new CWolf(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pModel, 11, (void**)m_ppTerrain, 4);
			m_ppGameObjects[i]->m_nkey = i;
			delete pModel;
		}

		for (int i = 8; i < m_nGameObjects; ++i) {
			CLoadedModelInfo* pModel = new CLoadedModelInfo(*pMetalonModel);
			m_ppGameObjects[i] = new CMetalon(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pModel, 11, (void**)m_ppTerrain, 4);
			m_ppGameObjects[i]->m_nkey = i;
			delete pModel;
		}
		delete pDragonModel;
		delete pWolfModel;
		delete pMetalonModel;
	}

	{
		m_nUIs = 5;
		m_ppUIObjects = new CUIObject * [m_nUIs];
		m_ppUIObjects[0] = new CUIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/Textures/UI_HP_FILL.dds", -0.85, -0.97, 0.85, -0.90, 0.8);
		m_ppUIObjects[0]->SetAlpha(0.8f);
		m_ppUIObjects[0]->SethPercent(1.f);

		m_ppUIObjects[1] = new CUIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/Textures/Player_Die_UI.dds", -1, -1, 1, 1, 0.8);
		m_ppUIObjects[1]->SetAlpha(0.0f);
		m_ppUIObjects[1]->SethPercent(1.f);

		m_ppUIObjects[2] = new CUIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/Textures/Player_Win_UI.dds", -1, -1, 1, 1, 0.8);
		m_ppUIObjects[2]->SetAlpha(0.0f);
		m_ppUIObjects[2]->SethPercent(1.f);

		m_ppUIObjects[3] = new CUIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/Textures/Player_Die_UI.dds", -0.85, -0.885, 0.85, -0.835, 0.8);
		m_ppUIObjects[3]->SetAlpha(0.8f);
		m_ppUIObjects[3]->SethPercent(1.f);
		m_ppUIObjects[3]->SetInfo(UI_STAMINA);

		m_ppUIObjects[4] = new CUIObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/Textures/Player_Die_UI.dds", -1, -1, 1, 1, 0.8);
		m_ppUIObjects[4]->SetAlpha(0.0f);
		m_ppUIObjects[4]->SethPercent(1.f);
		m_ppUIObjects[4]->SetInfo(UI_BLOOD);
	}

	m_pMap = new CMap(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, arrange);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::AddWeapon(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CLoadedModelInfo* p1HSwordModel = new CLoadedModelInfo(*CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_1Hsword.bin", NULL));
	CLoadedModelInfo* pBowModel = new CLoadedModelInfo(*CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_Bow.bin", NULL));
	CLoadedModelInfo* p2HSwordModel = new CLoadedModelInfo(*CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_2Hsword.bin", NULL));
	CLoadedModelInfo* p2HSpearModel = new CLoadedModelInfo(*CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_2HSpear.bin", NULL));

	for (int i = 0; i < 4; ++i) {
		m_ppWeapons[i] = new CGameObject();
		m_ppWeapons[i]->SetScale(0.6f, 0.6f, 0.6f);
		m_ppWeapons[i]->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 0), XMFLOAT3(80, 60, 40))->Rotate(0, 0, 20.f);
		m_ppWeapons[i]->SetHpBar(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 60, 0), XMFLOAT2(40, 10))->SetMaxHp(100);
		m_ppWeapons[i]->FindFrame("HpBar")->m_iHp = 0;
		m_ppWeapons[i]->SetActive("HpBar", false);
	}
	CGameObject* pSword = p1HSwordModel->m_pModelRootObject->FindFrame("Sword_Pivot");
	CGameObject* pBow = pBowModel->m_pModelRootObject->FindFrame("Bow_Pivot");
	CGameObject* pSword2 = p2HSwordModel->m_pModelRootObject->FindFrame("Long_Sword_Pivot");
	CGameObject* pSpear = p2HSpearModel->m_pModelRootObject->FindFrame("Spear_Pivot");

	m_ppWeapons[0]->SetChild(pSword);
	m_ppWeapons[1]->SetChild(pBow);
	m_ppWeapons[2]->SetChild(pSword2);
	m_ppWeapons[3]->SetChild(pSpear);

	XMFLOAT3 pos = m_ppTerrain[2]->GetPosition();
	m_ppWeapons[0]->SetPosition(Vector3::Add(pos, XMFLOAT3(1135, 190, 1370)));
	m_ppWeapons[1]->SetPosition(Vector3::Add(pos, XMFLOAT3(1135, 170, 1430)));
	m_ppWeapons[2]->SetPosition(Vector3::Add(pos, XMFLOAT3(1135, 200, 1490)));
	m_ppWeapons[3]->SetPosition(Vector3::Add(pos, XMFLOAT3(1135, 200, 1550)));
	m_ppWeapons[0]->Rotate(0, 0.f, -20.f);
	m_ppWeapons[1]->Rotate(0, 0.f, -20.f);
	m_ppWeapons[2]->Rotate(0, 0.f, -20.f);
	m_ppWeapons[3]->Rotate(0, 0.f, -20.f);


	if (p1HSwordModel) delete p1HSwordModel;
	if (pBowModel) delete pBowModel;
	if (p2HSwordModel) delete p2HSwordModel;
	if (p2HSpearModel) delete p2HSpearModel;

}

void CScene::AddPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CLoadedModelInfo* p1HSwordModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_1Hsword.bin", NULL);
	CLoadedModelInfo* pBowModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_Bow.bin", NULL);
	CLoadedModelInfo* p2HSwordModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_2Hsword.bin", NULL);
	CLoadedModelInfo* p2HSpearModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Player/Player_2HSpear.bin", NULL);

	for (int i = 0; i < MAX_PLAYER; ++i) {
		CLoadedModelInfo* pModel1 = new CLoadedModelInfo(*p1HSwordModel);
		CLoadedModelInfo* pModel2 = new CLoadedModelInfo(*pBowModel);
		CLoadedModelInfo* pModel3 = new CLoadedModelInfo(*p2HSwordModel);
		CLoadedModelInfo* pModel4 = new CLoadedModelInfo(*p2HSpearModel);
		m_m1HswordPlayer[i] = new C1HswordPlayer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, p1HSwordModel, (void**)m_ppTerrain);
		m_mBowPlayer[i] = new CBowPlayer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pModel2, (void**)m_ppTerrain);
		m_m2HswordPlayer[i] = new C2HswordPlayer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pModel3, (void**)m_ppTerrain);
		m_m2HspearPlayer[i] = new C2HspearPlayer(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pModel4, (void**)m_ppTerrain);
		if (i % 3 == 0)
			m_mPlayer[i] = m_mBowPlayer[i];
		if (i % 3 == 1)
			m_mPlayer[i] = m_m2HswordPlayer[i];
		if (i % 3 == 2)
			m_mPlayer[i] = m_m2HspearPlayer[i];
		m_mPlayer[i]->m_nkey = i;
		delete pModel1;
		delete pModel2;
		delete pModel3;
		delete pModel4;
	}

	if (p1HSwordModel) delete p1HSwordModel;
	if (pBowModel) delete pBowModel;
	if (p2HSwordModel) delete p2HSwordModel;
	if (p2HSpearModel) delete p2HSpearModel;
}

void CScene::InitPlayers()
{
	//m_m1HswordPlayer[0]->SetPosition(XMFLOAT3(10, 124, 25));
	//m_mBowPlayer[0]->SetPosition(XMFLOAT3(70, 124, 50));
	//m_mBowPlayer[0]->SetPlace(0);
	//m_m2HswordPlayer[0]->SetPosition(XMFLOAT3(130, 124, 50));
	//m_m2HspearPlayer[0]->SetPosition(XMFLOAT3(190, 124, 25));

}

void CScene::MovePlayer(int player_num, XMFLOAT3 pos)
{
	/*if (m_mPlayer[player_num]->GetGround())
	{
		if (m_mPlayer[player_num]->GetRunning()) {
			m_mPlayer[player_num]->m_pSkinnedAnimationController->SetAllTrackDisable();
			m_mPlayer[player_num]->m_pSkinnedAnimationController->SetTrackEnable(2, true);
		}
		else {
			m_mPlayer[player_num]->m_pSkinnedAnimationController->SetAllTrackDisable();
			m_mPlayer[player_num]->m_pSkinnedAnimationController->SetTrackEnable(11, true);
		}
	}*/

	m_mPlayer[player_num]->SetPosition(pos);
}

void CScene::AnimatePlayer(int id, int animation_num)
{
	switch (animation_num)
	{
	case 0:	// LbuttonDown
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		//cout << "0" << endl;
		break;
	case 1:	// LbuttonUp
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		//cout << "1" << endl;
		break;
	case 2: // RbuttonDown
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(2, true);
		break;
	case 3:
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(3, true);
		//cout << "3" << endl;
		break;
	case 4:	// LbuttonDown
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(4, true);
		//cout << "0" << endl;
		break;
	case 5:	// LbuttonUp
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(5, true);
		//cout << "1" << endl;
		break;
	case 6: // RbuttonDown
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(6, true);
		break;
	case 7:
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(7, true);
		//cout << "3" << endl;
		break;
	case 8:
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackPosition(8, 0);
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(8, true);
		//cout << "6" << endl;
		break;
	case 9:
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackPosition(9, 0);
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(9, true);
		//cout << "6" << endl;
		break;
	case 10:
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackPosition(10, 0);
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(10, true);
		break;
	case 11:
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackPosition(11, 0);
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(11, true);
		/*if (!strcmp(m_mPlayer[id]->m_pstrFrameName, "Player_Bow")) {
			m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
			m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		}*/
		break;
	case 12:
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackPosition(12, 0);
		m_mPlayer[id]->m_pSkinnedAnimationController->SetAllTrackDisable();
		m_mPlayer[id]->m_pSkinnedAnimationController->SetTrackEnable(12, true);
		break;
	}
	//	}
	//}
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

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
	if (m_pForestTerrain) delete m_pForestTerrain;
	if (m_pSnowTerrain) delete m_pSnowTerrain;
	if (m_pSkyBox) delete m_pSkyBox;
	if (m_ppTerrain) delete m_ppTerrain;

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	for (int i = 0; i < MAX_PLAYER; i++) {
		if (m_mBowPlayer[i]) {
			m_mBowPlayer[i]->Release();
		}
	}
	for (int i = 0; i < MAX_PLAYER; i++) {
		if (m_m1HswordPlayer[i]) {
			m_m1HswordPlayer[i]->Release();
		}
	}
	for (int i = 0; i < MAX_PLAYER; i++) {
		if (m_m2HswordPlayer[i]) {
			m_m2HswordPlayer[i]->Release();
		}
	}
	for (int i = 0; i < MAX_PLAYER; i++) {
		if (m_m2HspearPlayer[i]) {
			m_m2HspearPlayer[i]->Release();
		}
	}

	if (m_pMap)
	{
		m_pMap->Release();
		delete m_pMap;
	}
	if (m_ppUIObjects) {
		for (int i = 0; i < m_nUIs; ++i)
			m_ppUIObjects[i]->Release();
		delete[] m_ppUIObjects;
	}
	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[11];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtTextureSkin
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtWaterTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtWaterNormalTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 3; //t3: gtxtUI
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 10; //t10: empty
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 11; //t11: empty
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
	pd3dDescriptorRanges[10].BaseShaderRegister = 0; //t0: gtxtShadowMap
	pd3dDescriptorRanges[10].RegisterSpace = 0;
	pd3dDescriptorRanges[10].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[21];

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

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[15].Descriptor.ShaderRegister = 0; //Shadow Transform
	pd3dRootParameters[15].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[10]);
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[17].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[17].Descriptor.ShaderRegister = 3; //Framework Info
	pd3dRootParameters[17].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[17].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[18].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[18].Descriptor.ShaderRegister = 5; //Fog Info
	pd3dRootParameters[18].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[18].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[19].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[19].Descriptor.ShaderRegister = 6; //UI Info
	pd3dRootParameters[19].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[19].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[20].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[20].Descriptor.ShaderRegister = 9; //Terrain Info
	pd3dRootParameters[20].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[20].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[3];

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

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 0;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256�� ���
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);

}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
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
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	if (m_pForestTerrain) m_pForestTerrain->ReleaseUploadBuffers();
	if (m_pSnowTerrain) m_pSnowTerrain->ReleaseUploadBuffers();
	if (m_pMap) m_pMap->ReleaseUploadBuffers();
	for (int i = 0; i < 9; i++)
		if (m_ppTerrain[i]) m_ppTerrain[i]->ReleaseUploadBuffers();

	//for (auto p : m_mPlayer) {
	//	p.second->ReleaseUploadBuffers();
	//}

	//Object들을 각 개체별로 인스턴싱 했기 때문에 ReleaseUploadBuffers를 객체당 한번만 호출

	//Dragon
	m_ppGameObjects[0]->ReleaseUploadBuffers();
	//Wolf
	m_ppGameObjects[1]->ReleaseUploadBuffers();
	//Metalon
	m_ppGameObjects[8]->ReleaseUploadBuffers();
	
	if (m_mBowPlayer[0]) {
		m_mBowPlayer[0]->ReleaseUploadBuffers();
	}
	if (m_m1HswordPlayer[0]) {
		m_m1HswordPlayer[0]->ReleaseUploadBuffers();
	}
	if (m_m2HswordPlayer[0]) {
		m_m2HswordPlayer[0]->ReleaseUploadBuffers();
	}
	if (m_m2HspearPlayer[0]) {
		m_m2HspearPlayer[0]->ReleaseUploadBuffers();
	}
	if (m_ppUIObjects)
		for (int i = 0; i < m_nUIs; ++i)
			m_ppUIObjects[i]->ReleaseUploadBuffers();

	
}


void CScene::CheckCollision(CPacket* pPacket)
{
	if (m_pMap)
		m_pMap->CheckCollision(m_pPlayer);

	if (m_iState == INROOM) {
		for (int i = 0; i < 4; ++i) {
			CGameObject* pHpBar = m_ppWeapons[i]->FindFrame("HpBar");
			if (m_ppWeapons[i]->isCollide(m_pPlayer)) {
				if (pHpBar->m_iMaxHp < ++pHpBar->m_iHp) {
					// i = PlayerType
					/* state 변경후 서버와 통신 알아서*/
					// m_iState = INGAME;
					//for (int i = 0; i < 4; ++i)
					//	m_ppWeapons[i]->Release();
					pPacket->Set_StartWeapon((PlayerType)i);
					//pPacket->Send_ready_packet((PlayerType)i);
				}
				pHpBar->m_bActive = true;
			}
			else {
				if (--pHpBar->m_iHp < 0) {
					pHpBar->m_iHp = 0;
					pHpBar->m_bActive = false;
				}
			}
		}
	}
	else if (m_iState == INGAME) {
		for (auto& a : m_mPlayer) {
			XMFLOAT3 d = Vector3::Subtract(m_pPlayer->GetPosition(), a->GetPosition());
			if (m_pPlayer != a && Vector3::Length(d) < 100) {
				// check attack& collision: m_pPlayer -> a
				m_pPlayer->CheckCollision(a, false);
			}
		}

		for (int i = 0; i < m_nGameObjects; ++i) {
			if (m_ppGameObjects[i]->GetHp() > 0) {
				if (m_pPlayer->CheckCollision(m_ppGameObjects[i])) {
					//m_ppUIObjects[0]->SetvPercent((float)m_pPlayer->m_iHp / (float)m_pPlayer->m_iMaxHp);
				}
				//m_ppUIObjects[0]->SetvPercent(m_pPlayer->GetHp() / m_pPlayer->m_iMaxHp);
				//if (i == 0) {
				//	if (m_ppGameObjects[i]->GetBehaviorActivate() == true)
				//		CheckBehavior(m_ppGameObjects[i]);
				//	//CheckBehavior(m_ppGameObjects[i]);
				//}
			}
		}
	}
}

void CScene::CheckBehavior(CMonster *pMonster)
{
	XMFLOAT3 subtract;
	float rotation;
	float range;
	subtract = Vector3::Subtract(m_pPlayer->GetPosition(), pMonster->GetPosition());
	subtract.y = 0;
	range = Vector3::Length(subtract);
	float distance = (pMonster->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBExtents.z - pMonster->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBCenter.z) / 2.0f;
	if (range < 200.0f)
	{
		//printf("center : %f\nextents : %f\n\n", pMonster->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBCenter.z, pMonster->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBExtents.z);
		subtract = Vector3::Normalize(subtract);
		//printf("range : %f\n", range);
		// 실제 몬스터의 look 벡터
		XMFLOAT3 look = Vector3::ScalarProduct(pMonster->GetUp(), -1);
		//printf(" x : %f / y : %f / z : %f\n", pMonster->GetUp().x, pMonster->GetUp().y, pMonster->GetUp().z);

		rotation = acosf(Vector3::DotProduct(subtract, look)) * 180 / PI;
		//printf("rotation : %f\n", rotation);

		// 플레이어 쪽으로 이동, 일정 거리 안까지 들어가면 공격, 이동 종료
		if (range <= distance && rotation <= 5) {
			pMonster->Attack();
			return;
		}
		else if (range > distance) {
			pMonster->Move(subtract, 3.f);
		}
		// 외적에 따라 가까운 방향으로 회전하도록
		XMFLOAT3 cross = Vector3::CrossProduct(subtract, look);

		/*rotation = Vector3::Angle(subtract, look);
		printf("rotation2 : %f\n", rotation);*/
		if (EPSILON <= rotation)
			pMonster->Rotate(0.0f, 0.0f, -cross.y * rotation / 10);
		//printf("%f, %f, %f\n", cross.y, rotation, -cross.y * rotation / 10);
	}
	else
		pMonster->SetBehaviorActivate(false);
	//printf("%d 번째 크기 : %f\n", i, Vector3::Length(Vector3::Subtract(m_ppGameObjects[i]->GetPosition(), m_pPlayer->GetPosition())));

}

void CScene::TakeDamage(bool isDamaged)
{
	if (isDamaged)
		m_ppUIObjects[4]->SetAlpha(1);
	else
		m_ppUIObjects[4]->SetAlpha(0);
}

void CScene::UpdateMap()
{
	for (int i = 0; i < m_pMap->m_nMaps; ++i) {
		if (m_ppTerrain[i / 3]->IsFalling() && m_ppTerrain[1 / 3]->GetTime() < 20) {
			CGameObject* pObject = m_pMap->GetMap(i)->FindFrame("RootNode")->m_pChild->m_pChild;
			while (true) {
				XMFLOAT3 pos = pObject->GetPosition();
				pos.y = m_ppTerrain[i / 3]->GetHeight(pos.x, pos.z);
				pObject->SetPosition(pos);

				if (pObject->m_pSibling)
					pObject = pObject->m_pSibling;
				else
					break;
			}
		}
	}
}

void CScene::Reset()
{
	for (int i = 0; i < 9; ++i)
		m_ppTerrain[i]->Reset();
	m_pMap->Reset();
	for (int i = 0; i < MAX_PLAYER; ++i) {
		m_mPlayer[i]->Reset();
	}
	for (int i = 0; i < 4; ++i)
	{
		m_ppWeapons[i]->FindFrame("HpBar")->m_iHp = 0;
	}
}


void CScene::CreateCbvSrvUavDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nConstantBufferViews);
	m_d3dUavCPUDescriptorNextHandle.ptr= m_d3dUavCPUDescriptorStartHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);
	m_d3dUavGPUDescriptorNextHandle.ptr= m_d3dUavGPUDescriptorStartHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvUavDescriptorIncrementSize * nShaderResourceViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvUavDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvUavDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}


D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	//d3dShaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;

	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
		case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
		case RESOURCE_TEXTURE2D_ARRAY:
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
			d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
			break;
		case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
			d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
			d3dShaderResourceViewDesc.Buffer.NumElements = 0;
			d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
			d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			break;
	}
	return(d3dShaderResourceViewDesc);
}


D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, DXGI_FORMAT format)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetTexture(i);
			if (pShaderResource) {
				D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
				d3dResourceDesc.Format = format;
				D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
				pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
				m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

				//pTexture->SetGraphicsSrvRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
				pTexture->SetSrvGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
				m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			}
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
			if (pShaderResource) {
				D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
				D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
				pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
				m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

				//pTexture->SetGraphicsSrvRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
				pTexture->SetSrvGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
				m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
			}
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}

D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc;
	d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		d3dUnorderedAccessViewDesc.Texture2D.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2D.PlaneSlice = 0;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		d3dUnorderedAccessViewDesc.Texture2DArray.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dUnorderedAccessViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		d3dUnorderedAccessViewDesc.Texture2DArray.PlaneSlice = 0;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dUnorderedAccessViewDesc.Format = d3dResourceDesc.Format;
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		d3dUnorderedAccessViewDesc.Buffer.FirstElement = 0;
		d3dUnorderedAccessViewDesc.Buffer.NumElements = 0;
		d3dUnorderedAccessViewDesc.Buffer.StructureByteStride = 0;
		d3dUnorderedAccessViewDesc.Buffer.CounterOffsetInBytes = 0;
		d3dUnorderedAccessViewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		break;
	}
	return(d3dUnorderedAccessViewDesc);
}
D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture* pTexture, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dUavGPUDescriptorHandle = m_d3dUavGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = GetUnorderedAccessViewDesc(d3dResourceDesc, nTextureType);
			pd3dDevice->CreateUnorderedAccessView(pShaderResource, NULL,&d3dUnorderedAccessViewDesc, m_d3dUavCPUDescriptorNextHandle);
			m_d3dUavCPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;

			//pTexture->SetComputeUavRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dUavGPUDescriptorNextHandle);
			pTexture->SetUavGpuDescriptorHandle(i, m_d3dUavGPUDescriptorNextHandle);
			m_d3dUavGPUDescriptorNextHandle.ptr += ::gnCbvSrvUavDescriptorIncrementSize;
		}
	}
	return(d3dUavGPUDescriptorHandle);
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
		case 'z': case 'Z':
			gbShowBoundingBox = !gbShowBoundingBox;
			break;

		/*case 'W': m_ppGameObjects[0]->MoveForward(+3.0f); break;
		case 'S': m_ppGameObjects[0]->MoveForward(-3.0f); break;
		case 'A': m_ppGameObjects[0]->MoveStrafe(-3.0f); break;
		case 'D': m_ppGameObjects[0]->MoveStrafe(+3.0f); break;
		case 'Q': m_ppGameObjects[0]->MoveUp(+3.0f); break;
		case 'R': m_ppGameObjects[0]->MoveUp(-3.0f); break;*/
		default:
			break;
		}
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
	m_fElapsedTime = fTimeElapsed;
	if (m_iState == INROOM) {
		m_pPlayer->Animate(fTimeElapsed);		
	}
	else if (m_iState == INGAME) {
		m_ppUIObjects[3]->SethPercent(m_pPlayer->GetStamina() / MAX_STAMINA);
		if (m_pPlayer->GetDamaged()) {
			m_ppUIObjects[4]->DecreaseAlpha(fTimeElapsed);
		}
		else
			TakeDamage(false);

		for (int i = 0; i < MAX_PLAYER; ++i) {
			if (m_mPlayer[i]) {
				m_mPlayer[i]->Animate(fTimeElapsed);
				m_mPlayer[i]->UpdateTransform();
			}
		}

		for (int i = 0; i < m_nGameObjects; i++)
		{
			if (m_ppGameObjects[i])
			{
				m_ppGameObjects[i]->Update(fTimeElapsed);
			}
		}
		for (int i = 0; i < 9; ++i) {
			if (m_ppTerrain[i]->IsFalling())
				m_ppTerrain[i]->UpdateTime(fTimeElapsed);
		}
	}

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetCamera()->GetLookVector();
	}
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool onlyTerrain)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < 9; i++) {
		if (m_ppTerrain[i]) {
			m_ppTerrain[i]->UpdateShaderVariables(pd3dCommandList);
			m_ppTerrain[i]->Render(pd3dCommandList, pCamera);
		}
	}
	if (m_pMap) m_pMap->Render(pd3dCommandList, pCamera);

	if (onlyTerrain) return;

	if (m_iState == SCENE::INROOM) {
		m_pPlayer->Render(pd3dCommandList, NULL);
		for(int i=0;i<4;++i)
			m_ppWeapons[i]->Render(pd3dCommandList, pCamera);
	}
	else if (m_iState == SCENE::INGAME) {

		for (int i = 0; i < m_nGameObjects; i++)
		{
			if (m_ppGameObjects[i] && !m_ppTerrain[m_ppGameObjects[i]->GetPlace()]->IsFalling())
			{
				m_ppGameObjects[i]->Animate(m_fElapsedTime);
				m_ppGameObjects[i]->UpdateTransform(NULL);
				m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
			}
		}

		for (int i = 0; i < MAX_PLAYER; ++i)
			if (m_mPlayer[i]) {
				m_mPlayer[i]->UpdateShaderVariables(pd3dCommandList);
				if (m_mPlayer[i]->GetPosition().y > 0)
					m_mPlayer[i]->Render(pd3dCommandList, pCamera);
			}

		for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->Render(pd3dCommandList, pCamera);

		for (int i = 0; i < m_nUIs; ++i) {
			if (m_ppUIObjects[i]) {
				m_ppUIObjects[i]->UpdateShaderVariables(pd3dCommandList);
				m_ppUIObjects[i]->Render(pd3dCommandList, pCamera);
			}
		}
	}
	else if (m_iState == SCENE::ENDGAME)
	{
		if (m_pPlayer->GetRate() == 1)
		{
			m_ppUIObjects[3]->UpdateShaderVariables(pd3dCommandList);
			m_ppUIObjects[3]->Render(pd3dCommandList, pCamera);
		}
		else if (m_pPlayer->GetRate() > 1)
		{
			m_ppUIObjects[2]->UpdateShaderVariables(pd3dCommandList);
			m_ppUIObjects[2]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CScene::RenderShadow(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

	if (m_pTerrain) m_pTerrain->RenderShadow(pd3dCommandList, pCamera);
	if (m_pForestTerrain) m_pForestTerrain->RenderShadow(pd3dCommandList, pCamera);
	if (m_pSnowTerrain) m_pSnowTerrain->RenderShadow(pd3dCommandList, pCamera);
	for (int i = 0; i < 9; i++)
		if (m_ppTerrain[i]) m_ppTerrain[i]->RenderShadow(pd3dCommandList, pCamera);

	if (m_pMap) m_pMap->RenderShadow(pd3dCommandList, pCamera);

	if (m_iState == SCENE::INROOM) {
		for (int i = 0; i < 4; ++i)
			m_ppWeapons[i]->RenderShadow(pd3dCommandList, pCamera);
	}
	else if (m_iState == SCENE::INGAME) {
		//if (m_pSkyBox) m_pSkyBox->RenderShadow(pd3dCommandList, pCamera);

		for (int i = 0; i < m_nGameObjects; i++)
		{
			if (m_ppGameObjects[i])
			{
				m_ppGameObjects[i]->Animate(m_fElapsedTime);
				m_ppGameObjects[i]->UpdateTransform(NULL);
				m_ppGameObjects[i]->RenderShadow(pd3dCommandList, pCamera);
			}
		}

		for (int i = 0; i < MAX_PLAYER; ++i)
			if (m_mPlayer[i]) m_mPlayer[i]->RenderShadow(pd3dCommandList, pCamera);
	}
}

void CScene::Set(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
}

