#pragma once
#pragma comment(lib, "ws2_32")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <windows.h>
#include <random>
#include <cmath>
#include <unordered_set>
#include <unordered_map>

#include <thread>
#include <unordered_map>
#include <vector>
#include <mysql.h>
#include <functional>
#include <mutex>
#include <atomic>
#include <DirectXMath.h>

#pragma comment(lib, "libmySQL.lib")


#define TRUE 1
#define FALSE 0

using _packet = unsigned char;

class Vector2D
{
public:
	float x, y;
};

enum GameState {

};

enum terrain {
	Forest,
	Desert,
	Snowy_field
};


using namespace DirectX;

inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + XMLoadFloat3(&xmf3Vector2));
	return(xmf3Result);
}

inline XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar)
{
	XMFLOAT3 xmf3Result;
	XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2) * fScalar));
	return(xmf3Result);
}