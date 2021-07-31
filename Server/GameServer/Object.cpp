#include "Object.h"
#include <tchar.h>

//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include <iostream>


#define _WITH_DEBUG_FRAME_HIERARCHY

UINT ReadUnsignedIntegerFromFile(FILE* pInFile)
{
	UINT nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(UINT), 1, pInFile);
	return(nValue);
}

int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

int ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	int nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(int), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

static bool isSkinDeformation = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}



CGameObject::~CGameObject()
{
	
}

void CGameObject::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CGameObject::Release()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;
}

void CGameObject::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}


CGameObject* CGameObject::SetBBObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT3 Center, XMFLOAT3 Extents)
{
	CGameObject* pBBObj = new CGameObject();
	strcpy_s(pBBObj->m_pstrFrameName, "BoundingBox");

	CCubeMesh* pBoundingBox = new CCubeMesh(pd3dDevice, pd3dCommandList, Extents.x * 2, Extents.y * 2, Extents.z * 2);
	pBoundingBox->m_xmf3AABBCenter = Center;
	pBoundingBox->m_xmf3AABBExtents = Extents;

	pBBObj->SetBoundingBoxShader();
	pBBObj->Move(Center, 1);
	pBBObj->m_pMesh->m_xmf3AABBCenter = Center;
	pBBObj->m_pMesh->m_xmf3AABBExtents = Extents;
	SetChild(pBBObj, true);

	return pBBObj;
}

bool CGameObject::isCollide(CGameObject* pObject)
{
	BoundingOrientedBox bb = BoundingOrientedBox(
		FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBCenter,
		FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBExtents,
		XMFLOAT4(0, 0, 0, 1));

	bb.Transform(bb, XMLoadFloat4x4(&m_xmf4x4World));

	BoundingOrientedBox bbObject = BoundingOrientedBox(
		pObject->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBCenter,
		pObject->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBExtents,
		XMFLOAT4(0, 0, 0, 1));
	bbObject.Transform(bbObject, XMLoadFloat4x4(&pObject->m_xmf4x4World));

	if (bb.Contains(bbObject) != DirectX::DISJOINT || bb.Intersects(bbObject)) {
		return true;
	}
	return false;
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

CGameObject* CGameObject::FindFrame(char* pstrFrameName)
{
	CGameObject* pFrameObject = NULL;

	if (!strcmp(m_pstrFrameName, pstrFrameName)) return(this);

	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject::SetActive(char* pstrFrameName, bool bActive)
{
	CGameObject* pFrameObject = FindFrame(pstrFrameName);
	if (pFrameObject) pFrameObject->m_bActive = bActive;
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
	/*
		XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		if (!strcmp(m_pstrFrameName, "L_shoulder")) xmf4Color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &xmf4Color, 16);
	*/
}


void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();


	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParent._41 = x;
	m_xmf4x4ToParent._42 = y;
	m_xmf4x4ToParent._43 = z;

	UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetParentLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4ToParent._31, m_xmf4x4ToParent._32, m_xmf4x4ToParent._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed,
		m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 +
		vDirection.z * fSpeed);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	UpdateTransform(NULL);
}

#define _WITH_DEBUG_FRAME_HIERARCHY

UINT ReadUnsignedIntegerFromFile(FILE* pInFile)
{
	UINT nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(UINT), 1, pInFile);
	return(nValue);
}

int ReadIntegerFromFile(FILE* pInFile)
{
	int nValue = 0;
	UINT nReads = (UINT)::fread(&nValue, sizeof(int), 1, pInFile);
	return(nValue);
}

float ReadFloatFromFile(FILE* pInFile)
{
	float fValue = 0;
	UINT nReads = (UINT)::fread(&fValue, sizeof(float), 1, pInFile);
	return(fValue);
}

int ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
	int nStrLength = 0;
	UINT nReads = 0;
	nReads = (UINT)::fread(&nStrLength, sizeof(int), 1, pInFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
	pstrToken[nStrLength] = '\0';

	return(nStrLength);
}

static bool isSkinDeformation = false;
CGameObject* CGameObject::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, int* pnSkinnedMeshes)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nFrame = ::ReadIntegerFromFile(pInFile);

	CGameObject* pGameObject = new CGameObject();
	::ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
	{
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("FrameName: %hs\n"), pGameObject->m_pstrFrameName);
		OutputDebugString(pstrDebug);
	}
	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Transform>:"))
		{
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4ToParent, sizeof(XMFLOAT4X4), 1, pInFile);

			nReads = (UINT)::fread(&pGameObject->m_xmf3Scale, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&pGameObject->m_xmf3Rotation, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&pGameObject->m_xmf3Translation, sizeof(XMFLOAT3), 1, pInFile);


			//XMMATRIX S = XMMatrixScaling(pGameObject->m_xmf3Scale.x, pGameObject->m_xmf3Scale.y, pGameObject->m_xmf3Scale.z);
			//XMMATRIX R = XMMatrixRotationRollPitchYaw(pGameObject->m_xmf3Rotation.x, pGameObject->m_xmf3Rotation.y, pGameObject->m_xmf3Rotation.z);
			////XMMATRIX R = XMMatrixMultiply(XMMatrixMultiply(XMMatrixRotationX(xmf3R.x * fWeight), XMMatrixRotationY(xmf3R.y * fWeight)), XMMatrixRotationZ(xmf3R.z * fWeight));
			//XMMATRIX T = XMMatrixTranslation(pGameObject->m_xmf3Translation.x, pGameObject->m_xmf3Translation.y, pGameObject->m_xmf3Translation.z);

			//XMFLOAT4X4 xmf4x4Transform;
			//XMStoreFloat4x4(&xmf4x4Transform, XMMatrixMultiply(XMMatrixMultiply(S, R), T));
			//pGameObject->UpdateTransform(&xmf4x4Transform);

		}
		else if (!strcmp(pstrToken, "<Children>:"))
		{
			int nChilds = ::ReadIntegerFromFile(pInFile);
			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<Frame>:"))
					{
						CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pnSkinnedMeshes);
						if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
						TCHAR pstrDebug[256] = { 0 };
						_stprintf_s(pstrDebug, 256, _T("[%d] (Frame: %p) (Parent: %p)\n"), i, pChild, pGameObject);
						OutputDebugString(pstrDebug);
#endif
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Frame>"))
		{
			break;
		}
	}
	return(pGameObject);
}

CGameObject* CGameObject::LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	char pstrToken[64] = { '\0' };
	CGameObject* pGameObject = NULL;

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>"))
			{
				for (; ; )
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<Frame>:"))
					{
						pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, NULL);
					}
					else if (!strcmp(pstrToken, "</Hierarchy>"))
					{
						break;
					}
				}
			}
		}
		else
		{
			break;
		}
	}
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pGameObject);
}


void CGameObject::PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}


CLoadedModelInfo* CGameObject::LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName)
{
	FILE* pInFile = NULL;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CLoadedModelInfo* pLoadedModel = new CLoadedModelInfo();
	pLoadedModel->m_pModelRootObject = new CGameObject();
	strcpy_s(pLoadedModel->m_pModelRootObject->m_pstrFrameName, "RootNode");

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		if (::ReadStringFromFile(pInFile, pstrToken))
		{
			if (!strcmp(pstrToken, "<Hierarchy>"))
			{
				for (; ; )
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<Frame>:"))
					{
						CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, pInFile, NULL);
						if (pChild) pLoadedModel->m_pModelRootObject->SetChild(pChild);
					}
					else if (!strcmp(pstrToken, "</Hierarchy>"))
					{
						break;
					}
				}
			}
			else if (!strcmp(pstrToken, "</Animation>"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pLoadedModel->m_pModelRootObject, NULL);
#endif

	::fclose(pInFile);

	return(pLoadedModel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int texture) : CGameObject(1)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	CHeightMapGridMesh* pMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
	SetMesh(pMesh);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//

CMap::CMap(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, std::vector<int> arrange, void* pContext)
{
	CLoadedModelInfo* pDesert_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Collision.bin");
	CLoadedModelInfo* pDesert_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Passable.bin");
	CLoadedModelInfo* pDesert_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Desert_Passable.bin");

	CLoadedModelInfo* pForest_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Collision.bin");
	CLoadedModelInfo* pForest_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Passable.bin");
	CLoadedModelInfo* pForest_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Forest_Passable.bin");

	CLoadedModelInfo* pSnow_Collision = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Collision.bin");
	CLoadedModelInfo* pSnow_Steppable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Passable.bin");
	CLoadedModelInfo* pSnow_Passable = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Map/Probuilder_Snow_Passable.bin");
	//SetChild(pDesert_Test->m_pModelRootObject, true);

	m_ppMaps[0] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Collision, true);
	m_ppMaps[1] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Steppable);
	m_ppMaps[2] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pDesert_Passable);

	m_ppMaps[3] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Collision, true);
	m_ppMaps[4] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Steppable);
	m_ppMaps[5] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pForest_Passable);

	m_ppMaps[3]->SetPosition(-2048.0f, 0.0f, 0.0f);
	m_ppMaps[4]->SetPosition(-2048.0f, 0.0f, 0.0f);
	m_ppMaps[5]->SetPosition(-2048.0f, 0.0f, 0.0f);

	m_ppMaps[6] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Collision, true);
	m_ppMaps[7] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Steppable);
	m_ppMaps[8] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pSnow_Passable);

	m_ppMaps[6]->SetPosition(2048.0f, 60.0f, 0.0f);
	m_ppMaps[7]->SetPosition(2048.0f, 60.0f, 0.0f);
	m_ppMaps[8]->SetPosition(2048.0f, 60.0f, 0.0f);

	//for (int i = 0; i < (arrange.size() / 2); i++)
	//{
	//	m_ppMaps[3 * i + 0]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	//	m_ppMaps[3 * i + 1]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	//	m_ppMaps[3 * i + 2]->SetPosition(2048.0f * arrange[2 * i], 0.0f, 2048.0f * arrange[2 * i + 1]);
	//}

	//for (int i = 0; i < 3; ++i) {
	//	for (int j = 0; j < 3; ++j) {
	//		int n = i * 3 + j;
	//		if (n % 3 == 0) {
	//			m_ppMaps[n] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pMapForest, 0);
	//			m_ppMaps[n]->SetPosition(i * 500.0f, 0, j * 500.0f);
	//		}
	//		else if (n % 3 == 1) {
	//			m_ppMaps[n] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pMapDesert, 0);
	//			m_ppMaps[n]->SetPosition(i * 500.0f, 0, j * 500.0f);
	//		}
	//		else {
	//			m_ppMaps[n] = new CMapObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pMapSnow, 0);
	//			m_ppMaps[n]->SetPosition(i * 500.0f, 0, j * 500.0f);
	//		}
	//		//m_ppMaps[n]->SetScale(10,10,10);
	//	}
	//}

	if (pDesert_Collision)delete pDesert_Collision;
	if (pDesert_Steppable)delete pDesert_Steppable;
	if (pDesert_Passable)delete pDesert_Passable;
}

CMap::~CMap()
{
}

void CMap::ReleaseUploadBuffers()
{
	for (int i = 0; i < 9; ++i)
		m_ppMaps[i]->ReleaseUploadBuffers();
}

void CMap::Release()
{
	for (int i = 0; i < 9; ++i)
		if (m_ppMaps[i])
			m_ppMaps[i]->Release();
}

CGameObject* CMap::GetMap(int idx) const
{
	return m_ppMaps[idx];
}


CMapObject::CMapObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, bool bCollide)
{
	CLoadedModelInfo* pMapModel = pModel;
	if (!pMapModel) pMapModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Tree.bin");

	SetChild(pMapModel->m_pModelRootObject, true);

	CGameObject* pObject = FindFrame("RootNode")->m_pChild->m_pChild;

	while (bCollide) {
		BoundingBox bb = BoundingBox(pObject->m_pMesh->m_xmf3AABBCenter, pObject->m_pMesh->m_xmf3AABBExtents);
		pObject->SetBBObject(pd3dDevice, pd3dCommandList, XMFLOAT3(0, 0, 0), bb.Extents);
		//pObject->UpdateTransform();
		if (pObject->m_pSibling)
			pObject = pObject->m_pSibling;
		else
			break;
	}

	strcpy_s(m_pstrFrameName, "Map");
	Rotate(0, 90.f, 0);

	//SetScale(20.5f, 10.25f, 20.5f);



	//Rotate(-90.0f, 0.0f, 0.0f);
}

CMapObject::~CMapObject()
{
}


CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}