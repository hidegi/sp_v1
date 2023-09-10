#ifndef SP_STOPWATCH_H
#define SP_STOPWATCH_H
#include <sp/sp.h>
#ifdef SP_SYSTEM_WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#endif


namespace sp
{
	class StopWatch
	{
		public:
			StopWatch()
			{
#ifdef SP_SYSTEM_WINDOWS
				QueryPerformanceFrequency(&m_counterFrequency);
				QueryPerformanceCounter(&m_lastCount);
#else
				gettimeofday(&m_lastCount, 0);
#endif
			}
				
			inline void reset()
			{
#ifdef SP_SYSTEM_WINDOWS
				QueryPerformanceCounter(&m_lastCount);
#else
				gettimeofday(&m_lastCount, 0);
#endif
			}
			float getElapsedSeconds()
			{
#ifdef SP_SYSTEM_WINDOWS
				LARGE_INTEGER current;
				QueryPerformanceCounter(&current);
				return float((current.QuadPart - m_lastCount.QuadPart) / double(m_counterFrequency.QuadPart));
#else
				timeval current;
				gettimeofday(&current, 0);
				float seconds = (float)(current.tv_set - m_lastCount.tv_sec);
				float fraction = (flat)(current.tv_usec - m_lastCount.tv_usec) * .000001f;
				return seconds + fraction;
#endif
			}
			
		protected:
#ifdef SP_SYSTEM_WINDOWS
		LARGE_INTEGER			m_counterFrequency;
		LARGE_INTEGER			m_lastCount;
#else
		timeval					m_lastCount;
#endif
	};
}
#endif