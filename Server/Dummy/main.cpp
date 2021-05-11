#include "stdafx.h"
#include "Player.h"
#include "CPacket.h"

LPCTSTR lpszClass = TEXT("windows program");
LPCTSTR windowName = TEXT("WINDOW NAME");

#define CLIENT_WIDTH   600
#define CLIENT_HEIGHT  600

#pragma warning(disable : 4996)

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

void draw(HDC hDC);

CPacket *packet = new CPacket;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;	
	WndClass.lpfnWndProc = WndProc;	
	WndClass.cbClsExtra = 0;	
	WndClass.cbWndExtra = 0;	
	WndClass.hInstance = hInstance;	
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);	
	WndClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));	
	WndClass.lpszMenuName = NULL;	
	WndClass.lpszClassName = lpszClass;
	RegisterClass(&WndClass);	
	hwnd = CreateWindow(
		lpszClass,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,		//x��ǥ
		CW_USEDEFAULT,		//y��ǥ
		CLIENT_WIDTH,		//������ ����ũ��
		CLIENT_HEIGHT,		//������ ����ũ��
		NULL,	// �θ� ������ �ڵ�
		NULL,	// �޴� �ڵ�
		hInstance,		//���� ���α׷� �ν��Ͻ�
		NULL		//���� ������ ����
	);
	ShowWindow(hwnd, nCmdShow);	//(��Ÿ�� ������ �ڵ鰪, �����츦 ȭ�鿡 ��Ÿ���� ������� ����� ���� ex)SW_MAXIMIZE)
	UpdateWindow(hwnd);	//������ ȭ�鿡 �⺻ ����ϱ�

	if (AllocConsole())
	{
		freopen("CONIN$", "rb", stdin);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
	}

	packet->Init();

	while (GetMessage(&msg, NULL, 0, 0))	
	{
		TranslateMessage(&msg);		
		DispatchMessage(&msg);		
	}

	packet->Thread_join();
	return (int)msg.wParam;
}

void CALLBACK MyTimerProc(HWND hwnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
	InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static int x, y;
	static RECT rectView;
	static BOOL flag;

	switch (iMsg)
	{
	case WM_CREATE:	//�����찡 �����������
		GetClientRect(hwnd, &rectView);
		SetTimer(hwnd, 1, 1000, MyTimerProc);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		draw(hdc);
		EndPaint(hwnd, &ps);
		break;

	case WM_KEYUP:
		flag = 0;
		InvalidateRgn(hwnd, NULL, TRUE);
		break;

	case WM_KEYDOWN:
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	case WM_DESTROY:	//�����찡 ��������
		KillTimer(hwnd, 1);
		PostQuitMessage(0);	//GetMessage�Լ��� 0�� ��ȯ�ϰ���
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);	//�������� Ŀ�ο��� ó��
}

void draw(HDC hDC)
{
	for (auto& p : packet->players) {
		if (p.connected) {
			packet->rcTmpPlayer[p.id] = p.m_rcObject;
			packet->rcTmpPlayer[p.id].left += p.pos.x / 5.f;
			packet->rcTmpPlayer[p.id].right += p.pos.x / 5.f;
			packet->rcTmpPlayer[p.id].top += p.pos.y/ 5.f;
			packet->rcTmpPlayer[p.id].bottom += p.pos.y/5.f;
			FillRect(hDC, &packet->rcTmpPlayer[p.id], p.m_hbrObject);
		}
	}
}