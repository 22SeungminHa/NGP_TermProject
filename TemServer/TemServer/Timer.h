#pragma once
#include"stdafx.h"

#define TIMER INSTANCE(CGameTimer)

const ULONG MAX_SAMPLE_COUNT = 50; // Maximum frame time sample count

class CGameTimer
{
private:						
	CGameTimer() {}						
	~CGameTimer() {}						
public:							
	static CGameTimer& GetInstance()		
{							
static CGameTimer instance;		
return instance;		
}

private:
	double secondsPerCount{};
	float deltaTime{};

	__int64 baseTime{};
	__int64 pausedTime{};
	__int64 stopTime{};
	__int64 currentTime{};
	__int64 lastTime{};

	float m_fFrameTime[MAX_SAMPLE_COUNT]{};
	ULONG m_nSampleCount{};

	unsigned long m_nCurrentFrameRate{};
	unsigned long m_FramePerSecond{};
	float m_fFPSTimeElapsed{};

	bool isStopped{};

public:
	void Initilaize();

	void Tick(float fLockFPS = 0.0f);
	void Start();
	void Stop();
	void Reset();

    std::wstring GetFrameRate();
    float GetDeltaTime();
	float GetTotalTime();
};
