#pragma once
#include "stdafx.h"

class Timer {
public:
	static int time;

	HANDLE hTimerQueue;
	HANDLE hTimer;

	void init();
	static void CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired);
};