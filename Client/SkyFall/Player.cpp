//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

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

	m_iHp = 0;
	m_fAtkStat = 0;
	m_fDefStat = 0;
	m_fHitCool = 0.f;

	m_pPlayerUpdatedContext = NULL;
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

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

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

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

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
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) {
		CGameObject::Render(pd3dCommandList, pCamera);
	}
}



void CPlayer::CheckCollision(CGameObject* pObject)
{
	// Player - pObject
	if (m_fHitCool <= 0 && isCollide(pObject)) {
		m_fHitCool = 0.f;
		XMFLOAT3 d = Vector3::Subtract(m_xmf3Position, pObject->GetPosition());
		Move(Vector3::ScalarProduct(d, 50.25f, true), true);

		cout << "충돌 - " << pObject->m_pstrFrameName << endl;
	}
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

CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
	strcpy_s(m_pstrFrameName, "Player_Basic");

	CLoadedModelInfo *pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Player_Bow.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0,0,30), XMFLOAT3(10,10,30));
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 12, pPlayerModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Idle, nBasic_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Walk, nBasic_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_Run, nBasic_Run);
	/*m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunBack, nBasic_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunLeft, nBasic_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_RunRight, nBasic_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBasic_TakeDamage, nBasic_TakeDamage);*/


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

	SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);

	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
	if (pPlayerModel) delete pPlayerModel;

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
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
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
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
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
			XMFLOAT3 pos = m_xmf3Position;
			pos.y += 50;
			m_pCamera->SetLookAt(pos);
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
			break;
		default:
			break;
	}
	//m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
	
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
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Run, true);
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
		else if (::IsZero(fLength)&&m_isGround)
		{
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBasic_Idle, true);
		}
	}
	CPlayer::Update(fTimeElapsed);
}
#endif

CBowPlayer::CBowPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	strcpy_s(m_pstrFrameName, "Player_Bow");
	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Player_Bow.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 30), XMFLOAT3(10, 10, 30));
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 12, pPlayerModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_Idle, nBow_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_Walk, nBow_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_Run, nBow_Run);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_RunBack, nBow_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_RunLeft, nBow_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_RunRight, nBow_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_ShotHold, nBow_ShotHold);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_ShotRelease, nBow_ShotRelease);
	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_TakeDamage, nBow_TakeDamage);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_ShotReady, nBow_ShotReady);
	m_pSkinnedAnimationController->SetTrackType(nBow_ShotReady, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_Jump, nBow_Jump);
	m_pSkinnedAnimationController->SetTrackType(nBow_Jump, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(nBow_Death, nBow_Death);
	m_pSkinnedAnimationController->SetTrackType(nBow_Death, ANIMATION_TYPE_ONCE);


#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(nBow_Walk, 1);
	m_pSkinnedAnimationController->SetCallbackKey(nBow_Walk, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(nBow_Walk, pAnimationCallbackHandler);
#endif

	SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);
	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackEnable(nBow_Idle, true);

	if (pPlayerModel) delete pPlayerModel;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	//SetPosition(XMFLOAT3(310.0f, pTerrain->GetHeight(310.0f, 595.0f), 595.0f));


	m_ppBullets = new CBullet * [MAX_BULLET];


	CGameObject* pArrow = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Arrow.bin", NULL);
	CMesh* pMesh = pArrow->m_pMesh;


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



void CBowPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection && m_isGround)
	{
		if (m_isRunning) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBow_Run, true);
		}
		else {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBow_Walk, true);
		}
	}

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CBowPlayer::Update(float fTimeElapsed)
{
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (m_isJump) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackPosition(nBow_Jump, 0);
			m_pSkinnedAnimationController->SetTrackEnable(nBow_Jump, true);
		}
		else if (m_isCharging) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBow_ShotReady, true);
		}
		else if (::IsZero(fLength) && m_isGround)
		{
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(nBow_Idle, true);
		}
	}
	CPlayer::Update(fTimeElapsed);
}

void CBowPlayer::SetAttack(bool shoot)
{
	m_isAttack = shoot;

	if(!shoot)
		m_pSkinnedAnimationController->SetTrackPosition(nBow_ShotReady, 0);
}

void CBowPlayer::RButtonDown()
{
}

void CBowPlayer::RButtonUp()
{
}

void CBowPlayer::LButtonDown()
{
	if (!m_isCharging && !m_isAttack)
		SetCharging(true);
}

void CBowPlayer::LButtonUp()
{
	if (m_isCharging) {
		SetAttack(true);
		SetCharging(false);
	}
}


void CBowPlayer::CheckCollision(CGameObject* pObject)
{
	// Bullet - pObject
	if (m_ppBullets)
		for (int i = 0; i < m_nBullets; ++i) {
			if (pObject->isCollide(m_ppBullets[i])) {
				cout << "화살 충돌 - " << pObject->m_pstrFrameName << ": Hp = " << pObject->m_iHp << endl;
				DeleteBullet(i);
				--pObject->m_iHp;
			}
		}

	// Player - pObject
	if (m_fHitCool <= 0 && isCollide(pObject)) {
		m_fHitCool = 0.f;
		XMFLOAT3 d = Vector3::Subtract(m_xmf3Position, pObject->GetPosition());
		CPlayer::Move(Vector3::ScalarProduct(d, 50.25f, true), true);

		cout << "충돌 - " << pObject->m_pstrFrameName << endl;
	}
}

void CBowPlayer::Shot(float fTimeElapsed, float fSpeed)
{
	CGameObject* pBow = FindFrame("Bow_Main");

	m_ppBullets[m_nBullets]->m_xmf4x4World = pBow->m_xmf4x4World;

	XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
	xmf4x4Scale._11 = 0.5;
	xmf4x4Scale._22 = 0.5;
	xmf4x4Scale._33 = 0.5;

	m_ppBullets[m_nBullets]->m_xmf4x4ToParent = Matrix4x4::Multiply(xmf4x4Scale, m_xmf4x4ToParent);
	m_ppBullets[m_nBullets]->m_xmf3MovingDirection = GetCamera()->GetLookVector();
	m_ppBullets[m_nBullets]->SetSpeed(fSpeed);
	m_ppBullets[m_nBullets++]->Move(GetLook(), 10);
}

void CBowPlayer::Shot(float fTimeElapsed, float fSpeed, XMFLOAT3 Look)
{
	CGameObject* pBow = FindFrame("Bow_Main");

	m_ppBullets[m_nBullets]->m_xmf4x4World = pBow->m_xmf4x4World;

	XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
	xmf4x4Scale._11 = 0.5;
	xmf4x4Scale._22 = 0.5;
	xmf4x4Scale._33 = 0.5;

	m_ppBullets[m_nBullets]->m_xmf4x4ToParent = Matrix4x4::Multiply(xmf4x4Scale, m_xmf4x4ToParent);
	m_ppBullets[m_nBullets]->m_xmf3MovingDirection = Look;
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

	for (int i = 0; i < m_nBullets; ++i)
		m_ppBullets[i]->Animate(fTimeElapsed);
}

void CBowPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) {
		CGameObject::Render(pd3dCommandList, pCamera);
	}
	for (int i = 0; i < m_nBullets; ++i) {
		m_ppBullets[i]->Render(pd3dCommandList, pCamera);
	}
}

C1HswordPlayer::C1HswordPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	strcpy_s(m_pstrFrameName, "Player_1Hsword");
	CLoadedModelInfo* pPlayerModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Player/Player_1Hsword.bin", NULL);
	SetChild(pPlayerModel->m_pModelRootObject, true);

	pPlayerModel->m_pModelRootObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 30), XMFLOAT3(10, 10, 30));
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, 13, pPlayerModel);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Idle, n1Hsword_Idle);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Walk, n1Hsword_Walk);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Run, n1Hsword_Run);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_RunBack, n1Hsword_RunBack);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_RunLeft, n1Hsword_RunLeft);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_RunRight, n1Hsword_RunRight);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Attack1, n1Hsword_Attack1);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Attack2, n1Hsword_Attack2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Attack3, n1Hsword_Attack3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Attack4, n1Hsword_Attack4);
	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_TakeDamage, n1Hsword_TakeDamage);

	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Jump, n1Hsword_Jump);
	m_pSkinnedAnimationController->SetTrackType(n1Hsword_Jump, ANIMATION_TYPE_ONCE);

	m_pSkinnedAnimationController->SetTrackAnimationSet(n1Hsword_Death, n1Hsword_Death);
	m_pSkinnedAnimationController->SetTrackType(n1Hsword_Death, ANIMATION_TYPE_ONCE);


#ifdef _WITH_SOUND_CALLBACK
	m_pSkinnedAnimationController->SetCallbackKeys(n1Hsword_Walk, 1);
	m_pSkinnedAnimationController->SetCallbackKey(n1Hsword_Walk, 0, 0.001f, _T("Sound/Footstep01.wav"));

	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(n1Hsword_Walk, pAnimationCallbackHandler);
#endif

	SetPlayerUpdatedContext(pContext);
	//SetCameraUpdatedContext(pContext);


	m_pSkinnedAnimationController->SetAllTrackDisable();
	m_pSkinnedAnimationController->SetTrackEnable(n1Hsword_Idle, true);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CGameObject* pBlade = pPlayerModel->m_pModelRootObject->FindFrame("Sword_Blade");

	BoundingBox bb = BoundingBox(pBlade->m_pMesh->m_xmf3AABBCenter, pBlade->m_pMesh->m_xmf3AABBExtents);

	pBlade->SetBBObject(pd3dDevice, pd3dCommandList, 
		XMFLOAT3(0,0,0),
		XMFLOAT3(bb.Extents.x, bb.Extents.y, bb.Extents.z));

	if (pPlayerModel) delete pPlayerModel;
}

C1HswordPlayer::~C1HswordPlayer()
{
}

void C1HswordPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection && GetGround())
	{
		if (m_isRunning) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(n1Hsword_Run, true);
		}
		else {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(n1Hsword_Walk, true);
		}
	}

	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void C1HswordPlayer::Update(float fTimeElapsed)
{
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);

		if (m_isJump) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackPosition(n1Hsword_Jump, 0);
			m_pSkinnedAnimationController->SetTrackEnable(n1Hsword_Jump, true);
		}
		else if (m_isAttack) {
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(n1Hsword_Attack1 + m_nAttack, true);
		}
		else if (::IsZero(fLength) && m_isGround)
		{
			m_pSkinnedAnimationController->SetAllTrackDisable();
			m_pSkinnedAnimationController->SetTrackEnable(n1Hsword_Idle, true);
		}
	}
	CPlayer::Update(fTimeElapsed);
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
		m_pSkinnedAnimationController->SetTrackPosition(n1Hsword_Attack1, 0);
		m_pSkinnedAnimationController->SetTrackPosition(n1Hsword_Attack2, 0);
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
		m_pSkinnedAnimationController->SetTrackPosition(n1Hsword_Attack3, 0);
		m_pSkinnedAnimationController->SetTrackPosition(n1Hsword_Attack4, 0);
	}
}


void C1HswordPlayer::CheckCollision(CGameObject* pObject)
{
	// Player - pObject
	if (m_fHitCool <= 0 && isCollide(pObject)) {
		m_fHitCool = 0.f;
		XMFLOAT3 d = Vector3::Subtract(m_xmf3Position, pObject->GetPosition());
		CPlayer::Move(Vector3::ScalarProduct(d, 50.25f, true), true);

		cout << "충돌 - " << pObject->m_pstrFrameName << endl;
	}
	if (m_isAttack) {
		CGameObject* pBlade = FindFrame("Sword_Blade");
		if (pObject->isCollide(pBlade)) {
			cout << "검 충돌 - " << pObject->m_pstrFrameName << ": Hp = " << pObject->m_iHp << endl;
			--pObject->m_iHp;
		}
	}
}