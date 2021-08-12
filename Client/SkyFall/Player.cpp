//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "CPacket.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_iSpeedJump = 1000;
	m_isJump = false;
	m_isGround = true;
	m_isRunning = false;
	m_isStanding = true;
	m_isAttack = false;

	SetMaxHp(100);

	m_iAtkStat = 10;
	m_iDefStat = 0;
	m_fHitCool = 1.f;

	m_ppPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, +fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
#ifdef _WITH_LEFT_HAND_COORDINATES
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, +fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
#else
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, +fDistance);
#endif
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, +fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity.x = 0;
		m_xmf3Velocity.z = 0;
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
		if (m_isRunning)
		{
			m_xmf3Velocity.x *= 3.3;
			m_xmf3Velocity.z *= 3.3;
		}
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
#ifndef _WITH_LEFT_HAND_COORDINATES
	x = -x; y = -y; z = -z;
#endif
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	if(m_fHitCool>0)
		m_fHitCool -= fTimeElapsed;

	// jump
	if (GetJump() && GetGround())
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Up, m_iSpeedJump);
		SetJump(false);
	}
	else
		SetJump(false);

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity, fTimeElapsed);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;

	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (m_xmf3Velocity.y > 0 && fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_ppPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) {
		//m_pCamera->Update(m_xmf3Position, fTimeElapsed);
		if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
		//m_pCamera->SetLookAt(m_xmf3Position);
	}

	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	float VY = m_xmf3Velocity.y;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	m_xmf3Velocity.y = VY;
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(m_pCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(m_pCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(m_pCamera);
			break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	UpdateTransform(NULL);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{


	CGameObject::Render(pd3dCommandList, pCamera);

}



bool CPlayer::CheckCollision(CGameObject* pObject, bool isMonster)
{
	// Player - pObject
	if (isCollide(pObject)) {
		if (m_fHitCool <= 0 && isMonster) {
			m_fHitCool = 1.f;
			TakeDamage(pObject->GetAtkStat());
			cout << "damage : "  << pObject->GetAtkStat() << endl;
		}

		XMFLOAT3 d = Vector3::Subtract(m_xmf3Position, pObject->GetPosition());
		Move(Vector3::ScalarProduct(d, 50.f, true), true);

		return true;

		// 여기는 플레이어가 몬스터한테 맞는 부분
		//pObject->SetBehaviorActivate(true);
		//cout << "Monster Collision - " << pObject->m_pstrFrameName << endl;
	}
	return false;
}

void CPlayer::CheckMap(CGameObject* pMap)
{
	
}

void CPlayer::RotatePlayer(int iYaw)
{
	XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(iYaw));
	m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
	m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void *pCallbackData, float fTrackPosition)
{
   _TCHAR *pWavName = (_TCHAR *)pCallbackData; 
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s\n"), pWavName);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
   PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
   PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}

CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void **ppContext)
{

	strcpy_s(m_pstrFrameName, "Player_Basic");

	if (!pModel)
		pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Player_Bow.bin", NULL);
	CLoadedModelInfo* pPlayerModel = pModel;
	SetChild(pPlayerModel->m_pModelRootObject, true);

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0,0,30), XMFLOAT3(7,7,30));

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 9, pPlayerModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Idle, nBasic_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Walk, nBasic_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Run, nBasic_Run);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunBack, nBasic_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunLeft, nBasic_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunRight, nBasic_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_TakeDamage, nBasic_TakeDamage);


	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Jump, nBasic_Jump);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Jump, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Death, nBasic_Death);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Death, ANIMATION_TYPE_ONCE);


#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nBasic_Walk, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nBasic_Walk, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler *pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nBasic_Walk, pAnimationCallbackHandler);
#endif
	
	SetPlayerUpdatedContext(ppContext);
	//SetCameraUpdatedContext(pContext);
	SetPlace(2);
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)ppContext[m_nPlace];
	XMFLOAT3 pos = pTerrain->GetPosition();
	pos.x += 1270;
	pos.z += 1480;
	SetPosition(XMFLOAT3(pos.x, pTerrain->GetHeight(pos.x, pos.z), pos.z));

	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
	//SetJump(true);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	//SetPosition(XMFLOAT3(310.0f, pTerrain->GetHeight(310.0f, 595.0f), 595.0f));	


	//SetScale(XMFLOAT3(1.2f, 1.2f, 1.2f));
}

CTerrainPlayer::~CTerrainPlayer()
{
}

void CTerrainPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
	//m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixRotationX(-90.0f), m_xmf4x4ToParent);
	CGameObject::Rotate(-90.f, 0, 0);
}

CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	XMFLOAT3 pos = m_xmf3Position;

	//SetFriction(2.5f);
	SetGravity(XMFLOAT3(0.0f, -400.f, 0.0f));
	SetMaxVelocityXZ(1000.0f);
	SetMaxVelocityY(200.f);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:			
			/*SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);*/
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 50.0f, 0.0f));
			m_pCamera->SetPosition(m_xmf3Position);
			m_pCamera->Move(XMFLOAT3(0, 50, 0));
			pos.y += 40;
			pos = Vector3::Add(pos, Vector3::ScalarProduct(Vector3::Normalize(m_xmf3Look), 50));
			m_pCamera->SetLookAt(pos);
			m_pCamera->GenerateProjectionMatrix(1.0f, 1000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			/*SetFriction(125.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);*/
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			/*SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 50.0f, -70.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);*/
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 110.0f, -50.0f));
			m_pCamera->SetPosition(Vector3::Subtract(m_xmf3Position, Vector3::ScalarProduct(Vector3::Normalize(m_xmf3Look), Vector3::Length(m_pCamera->GetOffset()))));
			//m_pCamera->SetPosition(Vector3::Subtract(pos,Vector3::ScalarProduct(GetLook(),10,false)));
			pos.y += 50;
			m_pCamera->SetLookAt(pos);
			m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
			break;
		default:
			break;
	}
	//cout << "x: " << m_pCamera->GetPosition().x << ",z : " << m_pCamera->GetPosition().y << ", y: " << m_pCamera->GetPosition().z << endl;
	//m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_ppPlayerUpdatedContext[m_nPlace];
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	XMFLOAT3 xmf3TerrainPosition = pTerrain->GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x - xmf3TerrainPosition.x, xmf3PlayerPosition.z - xmf3TerrainPosition.z, bReverseQuad) + xmf3TerrainPosition.y;
	
	//SetFriction(1000.f);
	if (xmf3PlayerPosition.y <= fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
		if (!GetGround())
		{
			SetFriction(1000.f);
			SetGround(true);
		}
	}
	else if (GetGround())
	{
		SetGround(false);
	}
}

void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad);
	if (xmf3CameraPosition.y <= fHeight)
	{
		float d = fHeight - xmf3CameraPosition.y;
		xmf3CameraPosition.y = fHeight;
		XMFLOAT3 LookAtPos = m_pCamera->GetLookAtPosition();
		LookAtPos.y += d;
		m_pCamera->SetLookAtPosition(LookAtPos);
		m_pCamera->SetPosition(xmf3CameraPosition);
		/*if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}*/
	}
}

void CTerrainPlayer::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
}


#ifdef _WITH_SOUND_CALLBACK
void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection&&m_isGround)
	{
		if (m_isRunning) {
			if (dwDirection & DIR_FORWARD) {
				m_pSkinnedAnimationController->SetAllTrackDisable();
				m_pSkinnedAnimationController->SetTrackEnable(nBasic_Run, true);
			}
			else if (dwDirection & DIR_BACKWARD) {
				m_pSkinnedAnimationController->SetAllTrackDisable();
				m_pSkinnedAnimationController->SetTrackEnable(nBasic_RunBack, true);
			}
			else if (dwDirection & DIR_LEFT) {
				m_pSkinnedAnimationController->SetAllTrackDisable();
				m_pSkinnedAnimationController->SetTrackEnable(nBasic_RunLeft, true);
			}
			else if (dwDirection & DIR_RIGHT) {
				m_pSkinnedAnimationController->SetAllTrackDisable();
				m_pSkinnedAnimationController->SetTrackEnable(nBasic_RunRight, true);
			}
		}
		else {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Walk, true);
		}
	}

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CTerrainPlayer::Update(float fTimeElapsed)
{
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (m_isJump) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackPosition(nBasic_Jump, 0);
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Jump, true);
		}
		else if (::IsZero(fLength)&&m_isGround&&m_isDamaged == false)
		{
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
		}
	}
	CPlayer::Update(fTimeElapsed);
}
#endif

CBowPlayer::CBowPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext)
{
	strcpy_s(m_pstrFrameName, "Player_Bow");
	if(!pModel)
		pModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Player_Bow.bin", NULL);
	CLoadedModelInfo* pPlayerModel = pModel;

	SetChild(pPlayerModel->m_pModelRootObject, true);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 30), XMFLOAT3(7, 7, 30));
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 12, pPlayerModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Idle, nBasic_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Death, nBasic_Death);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Death, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Jump, nBasic_Jump);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Jump, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Walk, nBasic_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Run, nBasic_Run);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunBack, nBasic_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunLeft, nBasic_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunRight, nBasic_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_TakeDamage, nBasic_TakeDamage);
	m_pSkinnedAnimationController->SetTrackType(nBasic_TakeDamage, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nShotHold, nShotHold);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nShotRelease, nShotRelease);
	m_pSkinnedAnimationController->SetTrackType(nShotRelease, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nShotReady, nShotReady);
	m_pSkinnedAnimationController->SetTrackType(nShotReady, ANIMATION_TYPE_ONCE);



#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nBasic_Walk, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nBasic_Walk, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nBasic_Walk, pAnimationCallbackHandler);
#endif

	SetPlayerUpdatedContext(ppContext);
	//SetCameraUpdatedContext(pContext);
	SetJump(true);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	SetPlace(4);
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)ppContext[m_nPlace];
	XMFLOAT3 pos = pTerrain->GetPosition();
	pos.x += 100 + rand() % 1900;
	pos.z += 100 + rand() % 1900;
	SetPosition(XMFLOAT3(pos.x, pTerrain->GetHeight(pos.x, pos.z), pos.z));

	m_isRelease = false;

	pWeapon = pPlayerModel->m_pModelRootObject->FindFrame("Bow_Main");

	BoundingBox bb = BoundingBox(pWeapon->m_pMesh->m_xmf3AABBCenter, pWeapon->m_pMesh->m_xmf3AABBExtents);

	pWeapon->SetBBObject(pd3dDevice, pd3dCommandList,
		XMFLOAT3(0,0,-bb.Center.z/4),
		XMFLOAT3(bb.Extents.x, bb.Extents.y, bb.Extents.z));


	m_ppBullets = new CBullet * [MAX_BULLET];

	CGameObject* pArrow = FindFrame("Arrow_Obj");/*CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Arrow.bin", NULL);*/
	CMesh* pMesh = pArrow->m_pMesh;
	//CMesh* pMesh = CBullet::m_pArrow->m_pMesh;

	for (int i = 0; i < MAX_BULLET; ++i)
	{
		m_ppBullets[i] = new CBullet(pMesh);

		m_ppBullets[i]->SetBBObject(pd3dDevice, pd3dCommandList,
			XMFLOAT3(0, pMesh->m_xmf3AABBExtents.y + 10, 0),	// Center
			XMFLOAT3(2, 5, 2));									// Extents
		m_ppBullets[i]->SetWireFrameShader();
	}
}

CBowPlayer::~CBowPlayer()
{
}

void CBowPlayer::Update(float fTimeElapsed)
{
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (m_isRelease&&m_pSkinnedAnimationController->IsTrackFinish(nShotRelease)) {
			m_isRelease = false;
		}
		if (m_isDamaged && m_pSkinnedAnimationController->IsTrackFinish(8))
			m_isDamaged = false;
		if (m_isJump) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackPosition(nBasic_Jump, 0);
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Jump, true);
		}
		else if (m_isCharging) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nShotReady, true);
		}
		else if (::IsZero(fLength) && m_isGround && !m_isRelease && !m_isDamaged)
		{
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
		}
	}
	CPlayer::Update(fTimeElapsed);
}

void CBowPlayer::SetAttack(bool shoot)
{
	m_isAttack = shoot;

	if(!shoot)
		m_pSkinnedAnimationController->SetTrackPosition(nShotReady, 0);
}

void CBowPlayer::RButtonDown()
{
}

void CBowPlayer::RButtonUp()
{
	if (m_pCamera->GetMode() == FIRST_PERSON_CAMERA)
		m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	else
		m_pCamera = ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
}

void CBowPlayer::LButtonDown()
{
	if (!m_isCharging && !m_isAttack) {
		SetCharging(true);
		SetActive("Arrow_Obj", true);
	}
}

void CBowPlayer::LButtonUp()
{
	if (m_isCharging) {
		SetAttack(true);
		SetCharging(false);
		SetActive("Arrow_Obj", false);
		m_isRelease = true;
		//m_pSkinnedAnimationController->SetTrackEnable(nShotRelease, true);
	}
}


bool CBowPlayer::CheckCollision(CGameObject* pObject, bool isMonster)
{
	// Bullet - pObject
	if (m_ppBullets)
		for (int i = 0; i < m_nBullets; ++i) {
			if (m_ppBullets[i]->isCollide(pObject)) {
				cout << "Bullet Collision - " << pObject->m_pstrFrameName << ": Hp = " << pObject->GetHp() << endl;
				DeleteBullet(i);
				//pObject->TakeDamage(m_iAtkStat);
				if (isMonster)
					m_pPacket->Send_mon_damaged_packet(pObject->m_nkey, 3);
				else
					m_pPacket->Send_damage_to_player_packet(pObject->m_nkey, 3);
			}
			if(m_ppBullets[i]->GetPosition().y<=0)
				DeleteBullet(i--);
		}

	// Player - pObject
	return CPlayer::CheckCollision(pObject,isMonster);
}

void CBowPlayer::Shot(float fTimeElapsed, float fSpeed)
{
	CGameObject* pBow = FindFrame("Bow_Main");

	//m_ppBullets[m_nBullets]->m_xmf4x4World = pBow->m_xmf4x4World;

	XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
	xmf4x4Scale._11 = 0.5;
	xmf4x4Scale._22 = 0.5;
	xmf4x4Scale._33 = 0.5;
	m_ppBullets[m_nBullets]->m_xmf4x4ToParent =  Matrix4x4::Multiply(xmf4x4Scale, m_xmf4x4ToParent);
	m_ppBullets[m_nBullets]->SetPosition(pBow->GetPosition());
	m_ppBullets[m_nBullets]->m_xmf3MovingDirection = GetCamera()->GetLookVector();
	m_ppBullets[m_nBullets]->SetSpeed(fSpeed);
	m_ppBullets[m_nBullets++]->Rotate(90.f, 0, 0);
	//m_ppBullets[m_nBullets++]->Move(GetCamera()->GetLookVector(), 10);
}

void CBowPlayer::Shot(float fTimeElapsed, float fSpeed, XMFLOAT3 Look)
{
	CGameObject* pBow = FindFrame("Bow_Main");

	XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
	xmf4x4Scale._11 = 0.5;
	xmf4x4Scale._22 = 0.5;
	xmf4x4Scale._33 = 0.5;

	m_ppBullets[m_nBullets]->m_xmf4x4ToParent = Matrix4x4::Multiply(xmf4x4Scale, m_xmf4x4ToParent);
	m_ppBullets[m_nBullets]->SetPosition(pBow->GetPosition());
	m_ppBullets[m_nBullets]->m_xmf3MovingDirection = Look;
	printf("%f, %f, %f\n", Look.x, Look.y, Look.z);

	m_ppBullets[m_nBullets]->SetSpeed(fSpeed);
	m_ppBullets[m_nBullets++]->Move(Look, 10);
}

void CBowPlayer::DeleteBullet(const int& idx)
{
	for (int i = idx; i < m_nBullets - 1; ++i) {
		m_ppBullets[i] = m_ppBullets[i + 1];
	}
	--m_nBullets;
}

void CBowPlayer::Animate(float fTimeElapsed)
{
	CTerrainPlayer::Animate(fTimeElapsed);

	for (int i = 0; i < m_nBullets; ++i) {
		m_ppBullets[i]->Animate(fTimeElapsed);
		if (m_ppBullets[i]->GetPosition().y <= 0) {
			DeleteBullet(i--);
		}
	}
}

void CBowPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera = m_pCamera;
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == FIRST_PERSON_CAMERA) {
		CGameObject* pObject = FindFrame("Bow_Pivot");
		pObject->Render(pd3dCommandList, pCamera);
		FindFrame("Arrow_Pivot")->Render(pd3dCommandList, pCamera);
	}
	else
		CGameObject::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nBullets; ++i) {
		m_ppBullets[i]->Render(pd3dCommandList, pCamera);
	}
}

C1HswordPlayer::C1HswordPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext)
{
	strcpy_s(m_pstrFrameName, "Player_1Hsword");
	CLoadedModelInfo* pPlayerModel = pModel;
	SetChild(pPlayerModel->m_pModelRootObject, true);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 30), XMFLOAT3(7, 7, 30));
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 13, pPlayerModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Idle, nBasic_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Death, nBasic_Death);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Death, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Jump, nBasic_Jump);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Jump, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Walk, nBasic_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Run, nBasic_Run);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunBack, nBasic_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunLeft, nBasic_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunRight, nBasic_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_TakeDamage, nBasic_TakeDamage);
	m_pSkinnedAnimationController->SetTrackType(nBasic_TakeDamage, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack1, nAttack1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack2, nAttack2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack3, nAttack3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack4, nAttack4);

	m_pSkinnedAnimationController->SetTrackType(nAttack1, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackType(nAttack2, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackSpeed(nAttack1, 1.5f);
	m_pSkinnedAnimationController->SetTrackSpeed(nAttack2, 1.5f);

#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nBasic_Walk, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nBasic_Walk, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nBasic_Walk, pAnimationCallbackHandler);
#endif

	SetPlayerUpdatedContext(ppContext);
	//SetCameraUpdatedContext(pContext);


	SetJump(true);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pWeapon = pPlayerModel->m_pModelRootObject->FindFrame("Sword_Blade");

	BoundingBox bb = BoundingBox(pWeapon->m_pMesh->m_xmf3AABBCenter, pWeapon->m_pMesh->m_xmf3AABBExtents);

	pWeapon->SetBBObject(pd3dDevice, pd3dCommandList,
		XMFLOAT3(0,0,-bb.Center.z/4),
		XMFLOAT3(bb.Extents.x, bb.Extents.y, bb.Extents.z));

	SetPlace(4);
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)ppContext[m_nPlace];
	XMFLOAT3 pos = pTerrain->GetPosition();
	pos.x += 100 + rand() % 1900;
	pos.z += 100 + rand() % 1900;
	SetPosition(XMFLOAT3(pos.x, pTerrain->GetHeight(pos.x, pos.z), pos.z));
}

C1HswordPlayer::~C1HswordPlayer()
{
}


void C1HswordPlayer::Update(float fTimeElapsed)
{
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (m_isDamaged && m_pSkinnedAnimationController->IsTrackFinish(8))
			m_isDamaged = false;

		if (m_isJump) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackPosition(nBasic_Jump, 0);
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Jump, true);
		}
		else if (m_isAttack) {
			float d = m_pSkinnedAnimationController->GetTrackPosition(nAttack1 + m_nAttack);

			if (abs(d-0.2f)<=fTimeElapsed) {
				m_bHit = true;
			}
			if (m_pSkinnedAnimationController->IsTrackFinish(nAttack1 + m_nAttack))	{
				if (m_nAttack <= 1) {
					m_pSkinnedAnimationController->SetTrackPosition(nAttack1 + m_nAttack, 0);
					m_nAttack = m_nAttack == 0 ? 1 : 0;
					m_pSkinnedAnimationController->SetTrackPosition(nAttack1 + m_nAttack, 0);
				}
				if (m_pSkinnedAnimationController->GetTrackPosition(nAttack1 + m_nAttack) == 0)
					m_pPacket->Send_attack_packet(m_nAttack);
			}

			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nAttack1 + m_nAttack, true);
		}
		else if (::IsZero(fLength) && m_isGround && !m_isDamaged)
		{
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
		}
	}
	CPlayer::Update(fTimeElapsed);
}

void C1HswordPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera = m_pCamera;
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == FIRST_PERSON_CAMERA) {
		CGameObject* pObject = FindFrame("Sword_Pivot");
		pObject->Render(pd3dCommandList, pCamera);
	}
	else
		CGameObject::Render(pd3dCommandList, pCamera);
}

void C1HswordPlayer::LButtonDown()
{
	if (m_isGround) {
		m_nAttack = 0;
		m_isAttack = true;
	}
}

void C1HswordPlayer::LButtonUp()
{
	if (m_isAttack) {
		m_isAttack = false;
		m_bHit = false;
		m_pSkinnedAnimationController->SetTrackPosition(nAttack1, 0);
		m_pSkinnedAnimationController->SetTrackPosition(nAttack2, 0);
	}
}

void C1HswordPlayer::RButtonDown()
{
	if (m_isGround) {
		if (m_isRunning)
			m_nAttack = 2;
		else
			m_nAttack = 3;

		m_isAttack = true;
	}
}

void C1HswordPlayer::RButtonUp()
{
	if (m_isAttack) {
		m_isAttack = false; 
		m_bHit = false;
		m_pSkinnedAnimationController->SetTrackPosition(nAttack3, 0);
		m_pSkinnedAnimationController->SetTrackPosition(nAttack4, 0);
	}
}


bool C1HswordPlayer::CheckCollision(CGameObject* pObject, bool isMonster)
{

	if (m_isAttack&& m_bHit) {
		if (pObject->isCollide(pWeapon)) {
			//pObject->TakeDamage(m_iAtkStat * (1.f + m_nAttack / 4.f));
			//cout << "Sword Collision - " << pObject->m_pstrFrameName << ": Hp = " << pObject->GetHp() << endl;
			m_bHit = false;
			if (isMonster) {
				m_pPacket->Send_mon_damaged_packet(pObject->m_nkey, m_nAttack);
				pObject->SetBehaviorActivate(true);
				//cout << "Monster Collision - " << pObject->m_pstrFrameName << endl;
				pObject->FindFrame("HpBar")->m_bActive = true;
			}
			else
				m_pPacket->Send_damage_to_player_packet(pObject->m_nkey, m_nAttack);

		}
	}
	// Player - pObject
	return CPlayer::CheckCollision(pObject, isMonster);
}

C2HswordPlayer::C2HswordPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext)
{
	strcpy_s(m_pstrFrameName, "Player_2Hsword");
	CLoadedModelInfo* pPlayerModel = pModel;
	SetChild(pPlayerModel->m_pModelRootObject, true);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 30), XMFLOAT3(7, 7, 30));
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 13, pPlayerModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Idle, nBasic_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Death, nBasic_Death);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Death, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Jump, nBasic_Jump);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Jump, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Walk, nBasic_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Run, nBasic_Run);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunBack, nBasic_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunLeft, nBasic_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunRight, nBasic_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_TakeDamage, nBasic_TakeDamage);

	m_pSkinnedAnimationController->SetTrackType(nBasic_TakeDamage, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack1, nAttack1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack2, nAttack2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack3, nAttack3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack4, nAttack4);

	m_pSkinnedAnimationController->SetTrackType(nAttack1, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackType(nAttack2, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackSpeed(nAttack1, 1.5f);
	m_pSkinnedAnimationController->SetTrackSpeed(nAttack2, 1.5f);

#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nBasic_Walk, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nBasic_Walk, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nBasic_Walk, pAnimationCallbackHandler);
#endif

	SetPlayerUpdatedContext(ppContext);
	//SetCameraUpdatedContext(pContext);


	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pWeapon = pPlayerModel->m_pModelRootObject->FindFrame("Long_Sword_Blade");

	BoundingBox bb = BoundingBox(pWeapon->m_pMesh->m_xmf3AABBCenter, pWeapon->m_pMesh->m_xmf3AABBExtents);

	pWeapon->SetBBObject(pd3dDevice, pd3dCommandList,
		XMFLOAT3(0, -bb.Center.y / 4, 0),
		XMFLOAT3(bb.Extents.x, bb.Extents.y, bb.Extents.z));
}

C2HswordPlayer::~C2HswordPlayer()
{
}

C2HspearPlayer::C2HspearPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext)
{
	strcpy_s(m_pstrFrameName, "Player_2Hspear");
	CLoadedModelInfo* pPlayerModel = pModel;
	SetChild(pPlayerModel->m_pModelRootObject, true);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 30), XMFLOAT3(7, 7, 30));
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 13, pPlayerModel);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Idle, nBasic_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Death, nBasic_Death);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Death, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Jump, nBasic_Jump);
	m_pSkinnedAnimationController->SetTrackType(nBasic_Jump, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Walk, nBasic_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Run, nBasic_Run);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunBack, nBasic_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunLeft, nBasic_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunRight, nBasic_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_TakeDamage, nBasic_TakeDamage);
	m_pSkinnedAnimationController->SetTrackType(nBasic_TakeDamage, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack1, nAttack1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack2, nAttack2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack3, nAttack3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nAttack4, nAttack4);

	m_pSkinnedAnimationController->SetTrackType(nAttack1, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackType(nAttack2, ANIMATION_TYPE_ONCE);
	m_pSkinnedAnimationController->SetTrackSpeed(nAttack1, 1.5f);
	m_pSkinnedAnimationController->SetTrackSpeed(nAttack2, 1.5f);

#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nBasic_Walk, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nBasic_Walk, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nBasic_Walk, pAnimationCallbackHandler);
#endif

	SetPlayerUpdatedContext(ppContext);
	//SetCameraUpdatedContext(pContext);


	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);


	pWeapon = pPlayerModel->m_pModelRootObject->FindFrame("Spear_Blade");

	BoundingBox bb = BoundingBox(pWeapon->m_pMesh->m_xmf3AABBCenter, pWeapon->m_pMesh->m_xmf3AABBExtents);

	pWeapon->SetBBObject(pd3dDevice, pd3dCommandList,
		XMFLOAT3(0, -bb.Center.y/4, 0),
		XMFLOAT3(bb.Extents.x, bb.Extents.y, bb.Extents.z));
}

C2HspearPlayer::~C2HspearPlayer()
{
}
