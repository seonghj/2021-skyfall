#pragma once
#include "Timer.h"

int Timer::time = 0;

void CALLBACK Timer::TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	time++;
	std::cout << time << std::endl;
}

void Timer::init()
{
	hTimerQueue = CreateTimerQueue();

	CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerCallback, NULL, 1000, 1000, 0);
}