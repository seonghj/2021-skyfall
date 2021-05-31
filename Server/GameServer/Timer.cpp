#pragma once
#include "Timer.h"

using namespace std::chrono;

void Timer::init(HANDLE h_cp)
{
	m_hiocp = h_cp;

	std::priority_queue<Timer_event> empty_queue;
	std::swap(m_Timer_queue, empty_queue);

	m_isRun = TRUE;
	printf("Timer ready\n");
	Timer_main();
}

void Timer::push_event(int key, int event_type, int delaytime_ms, char* message)
{
	//std::lock_guard <std::mutex> lg{ m_timer_lock };
	Timer_event te;
	te.key = key;
	te.OE_Type = event_type;
	te.start_time = system_clock::now() + milliseconds(delaytime_ms);
	strcpy_s(te.event_message, message);
	//m_timer_lock.lock();
	m_Timer_queue.push(te);
	//m_timer_lock.unlock();
}

void Timer::Timer_main()
{
	while (m_isRun) {
		//std::lock_guard <std::mutex> lg{ m_timer_lock };
		//m_timer_lock.lock();
		if ((m_Timer_queue.empty() == FALSE)
			&& (m_Timer_queue.top().start_time <= system_clock::now())) {
			Timer_event te = m_Timer_queue.top();
			m_Timer_queue.pop();
			//m_timer_lock.unlock();
			OVER_EX* over_ex = new OVER_EX;
			over_ex->type = te.OE_Type;
			strcpy_s(over_ex->messageBuffer, te.event_message);
			over_ex->dataBuffer.buf = over_ex->messageBuffer;
			over_ex->dataBuffer.len = BUFSIZE;

			PostQueuedCompletionStatus(m_hiocp, 1, (ULONG_PTR)(&te.key), (LPOVERLAPPED)over_ex);
		}
		else {
			//m_timer_lock.unlock();
			std::this_thread::sleep_for(100ms);
		}
	}
}
