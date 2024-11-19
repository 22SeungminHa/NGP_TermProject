//-----------------------------------------------------------------------------
// File: CGameTimer.cpp
//-----------------------------------------------------------------------------
#include"stdafx.h"
#include "Timer.h"


void CGameTimer::Initilaize()
{
	__int64 countsPerSec{};

	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	QueryPerformanceCounter((LARGE_INTEGER*)&lastTime);
	secondsPerCount = 1.0 / (double)countsPerSec;

	baseTime = lastTime;
}

void CGameTimer::Tick(float fLockFPS)
{
	if (isStopped)
	{
		deltaTime = 0.0f;
		return;
	}
	float _deltaTime;

	QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);

	_deltaTime = (currentTime - lastTime) * secondsPerCount;

    if (fLockFPS > 0.0f)
    {
        while (_deltaTime < (1.0f / fLockFPS))
        {
	        QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
			_deltaTime = (currentTime - lastTime) * secondsPerCount;
        }
    } 

	lastTime = currentTime;

    if (fabsf(_deltaTime - deltaTime) < 1.0f)
    {
		::memmove(&m_fFrameTime[1], m_fFrameTime, (static_cast<size_t>(MAX_SAMPLE_COUNT) - 1) * sizeof(float));
        m_fFrameTime[0] = _deltaTime;
        if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
    }

	m_FramePerSecond++;
	m_fFPSTimeElapsed += _deltaTime;
	if (m_fFPSTimeElapsed > 1.0f) 
    {
		m_nCurrentFrameRate	= m_FramePerSecond;
		m_FramePerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	} 

    deltaTime = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) {
		deltaTime += m_fFrameTime[i];
	}

    if (m_nSampleCount > 0) deltaTime /= m_nSampleCount;
}

std::wstring CGameTimer::GetFrameRate()
{
	return std::to_wstring(m_nCurrentFrameRate) + L" FPS)";
}

float CGameTimer::GetDeltaTime() 
{
    return deltaTime;
}

float CGameTimer::GetTotalTime()
{
	if (isStopped) return(float(((stopTime - pausedTime) - baseTime) * secondsPerCount));
	return(float(((currentTime - pausedTime) - baseTime) * secondsPerCount));
}

void CGameTimer::Reset()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);

	baseTime = nPerformanceCounter;
	lastTime = nPerformanceCounter;
	stopTime = 0;
	isStopped = false;
}

void CGameTimer::Start()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER *)&nPerformanceCounter);
	if (isStopped)
	{
		pausedTime += (nPerformanceCounter - stopTime);
		lastTime = nPerformanceCounter;
		stopTime = 0;
		isStopped = false;
	}
}

void CGameTimer::Stop()
{
	if (!isStopped)
	{
		::QueryPerformanceCounter((LARGE_INTEGER *)&stopTime);
		isStopped = true;
	}
}
