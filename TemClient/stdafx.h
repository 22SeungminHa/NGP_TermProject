#pragma once

#define _CRT_NON_CONFORMINGS_SWPRINTFS
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment (lib, "fmod_vc.lib")

#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <atlImage.h> // cimg 때문에 필요한 헤더
#include <vector>
#include <string>
#include <fstream>
#include "FMOD\FMOD Studio API Windows\api\core\inc\fmod.hpp"
#include "FMOD\FMOD Studio API Windows\api\core\inc\fmod_errors.h" 
#include <random>
#include <time.h>
#include <protocol.h>
	// #include <enum.h>

constexpr int g = 10; // 중력가속도
constexpr float t = 0.19; // 속도 조절 변수
constexpr int side = 60; // 블럭 한 변의 길이
constexpr float rd = 12.5; // 공 반지름
constexpr int SVMAPCNT = 24;
