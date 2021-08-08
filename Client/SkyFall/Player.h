#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#include "Object.h"
#include "Camera.h"

class CPacket;

class CCamera;

class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;


	int							m_iSpeedJump;
	bool						m_isJump;
	bool						m_isGround;
	bool						m_isRunning;
	bool						m_isAttack;
	bool						m_isStanding;
	bool						m_isCharging;

	int							m_nPlace;
	bool						m_bHit = false;
	// stat

	LPVOID*						m_ppPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CCamera						*m_pCamera = NULL;

public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetJump(bool jump) { m_isJump = jump; }
	void SetRunning(bool run) { m_isRunning = run; }
	void SetGround(bool ground) { m_isGround = ground; }
	void SetStanding(bool stand) { m_isStanding = stand; }
	virtual void SetAttack(bool shoot) { m_isAttack = shoot; }
	void SetCharging(bool charge) { m_isCharging = charge; }
	void SetPlace(int nPlace) { m_nPlace = nPlace; }

	virtual void RButtonDown() {};
	virtual void RButtonUp() {};
	virtual void LButtonDown() {};
	virtual void LButtonUp() {};

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }
	bool  GetJump() const { return(m_isJump); }
	bool  GetRunning() const { return(m_isRunning); }
	bool  GetGround() const { return(m_isGround); }
	bool  GetAttack() const { return(m_isAttack); }
	bool  GetStanding() const { return(m_isStanding); }
	bool  GetCharging() const { return(m_isCharging); }
	int GetPlace() const { return(m_nPlace); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	virtual void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID* ppContext) { m_ppPlayerUpdatedContext = ppContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);


	virtual void Shot(float fTimeElapsed, float fSpeed) {};
	virtual void Shot(float fTimeElapsed, float fSpeed, XMFLOAT3 Look) {};
	virtual void DeleteBullet(const int& idx) {};
	virtual void CheckCollision(CGameObject* pObject);
	virtual void CheckMap(CGameObject* pObject);
	void RotatePlayer(int iYaw);

	CPacket* m_pPacket = NULL;

protected:
public:
};

class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void *pCallbackData, float fTrackPosition); 
};

class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer() :CPlayer() {};
	CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void ** ppContext=NULL);
	virtual ~CTerrainPlayer();

	virtual void OnPrepareRender();
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);

	virtual void Animate(float fTimeElapsed);

#ifdef _WITH_SOUND_CALLBACK
	virtual void Move(DWORD dwDirection, float fDistance, bool bVelocity = false);
	virtual void Update(float fTimeElapsed);
protected:
	CGameObject* pWeapon;

#endif
	const int nBasic_Idle = 0;
	const int nBasic_Death = 1;
	const int nBasic_Jump = 2;
	const int nBasic_Walk = 3;
	const int nBasic_Run = 4;
	const int nBasic_RunBack = 5;
	const int nBasic_RunLeft = 7;
	const int nBasic_RunRight = 6;
	const int nBasic_TakeDamage = 8;
};


class CBowPlayer : public CTerrainPlayer
{
public:
	void Shot(float fTimeElapsed, float fSpeed);
	void Shot(float fTimeElapsed, float fSpeed, XMFLOAT3 Look);
	void DeleteBullet(const int& idx);
	void Animate(float fTimeElapsed);

	CBowPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext = NULL);
	virtual ~CBowPlayer();

	virtual void Update(float fTimeElapsed);
	virtual void SetAttack(bool shoot);

	virtual void RButtonDown();
	virtual void RButtonUp();
	virtual void LButtonDown();
	virtual void LButtonUp();

	virtual void CheckCollision(CGameObject* pObject);

	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	const int nShotHold = 9;
	const int nShotReady = 10;
	const int nShotRelease = 11;

protected:
	int m_nBullets = 0;
	//vector<CBullet*> m_vpBullets;
	CBullet** m_ppBullets = 0;

	const int MAX_BULLET = 1000;
	bool		m_isRelease;
};

class C1HswordPlayer : public CTerrainPlayer
{
public:
	C1HswordPlayer() :CTerrainPlayer() {};
	C1HswordPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext = NULL);
	virtual ~C1HswordPlayer();

	virtual void Update(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	virtual void RButtonDown();
	virtual void RButtonUp();
	virtual void LButtonDown();
	virtual void LButtonUp();

	virtual void CheckCollision(CGameObject* pObject);

protected:
	int m_nAttack = 0;
	
	const int nAttack1 = 9;
	const int nAttack2 = 10;
	const int nAttack3 = 11;
	const int nAttack4 = 12;
};

class C2HswordPlayer : public C1HswordPlayer
{
public:
	C2HswordPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext = NULL);
	virtual ~C2HswordPlayer();

};

class C2HspearPlayer : public C1HswordPlayer
{
public:
	C2HspearPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void** ppContext = NULL);
	virtual ~C2HspearPlayer();


};