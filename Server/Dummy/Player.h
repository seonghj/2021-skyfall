#pragma once
#include "stdafx.h"
#include "protocol.h"

class Player {
public:
	POINT pos = POINT{ 500, 500 };
	RECT m_rcObject = RECT{ -5, -5, 5, 5 };
	HBRUSH m_hbrObject = CreateSolidBrush(RGB(0, 175, 197));

	OVER_EX over;
	SOCKET  sock;
	char   packet_buf[BUFSIZE];

	char* packet_start;
	char* recv_start;
	int   prev_size;

	BOOL connected;
	int id;

	int event_type;
};