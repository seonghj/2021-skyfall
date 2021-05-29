#pragma once
#include "Timer.h"

using namespace std::chrono;

void Timer::init(HANDLE h_cp)
{
	m_hiocp = h_cp;

	std::priority_queue<Timer_event> empty_queue;
	std::swap(m_Timer_queue, empty_queue);

	Timer_main();
	m_isRun = TRUE;
}

void Timer::push_event(int key, OVER_EX_Type type, int time_ms, char* message)
{
	Timer_event te;
	te.key = key;
	te.OE_Type = type;
	te.start_time = system_clock::now() + milliseconds(time_ms);
	memcpy(te.event_message, message, sizeof(message));
	std::lock_guard <std::mutex> lg{ m_timer_lock };
	m_Timer_queue.push(te);
}

void Timer::Timer_main()
{
	while (m_isRun) {
		std::lock_guard <std::mutex> lg{ m_timer_lock };
		if ((m_Timer_queue.empty() == FALSE)
			&& (m_Timer_queue.top().start_time <= system_clock::now())) {
			Timer_event te = m_Timer_queue.top();
			m_Timer_queue.pop();

			OVER_EX* over_ex = new OVER_EX;
			over_ex->type = te.OE_Type;
			memcpy(over_ex->messageBuffer, te.event_message, sizeof(te.event_message));
			over_ex->dataBuffer.buf = over_ex->messageBuffer;
			over_ex->dataBuffer.len = sizeof(te.event_message);

			PostQueuedCompletionStatus(m_hiocp, NULL, te.key, &over_ex->overlapped);
		}
		else
			std::this_thread::sleep_for(10ms);
	}
}
