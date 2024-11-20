#pragma once

#define _CRT_NON_CONFORMINGS_SWPRINTFS
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment (lib, "fmod_vc.lib")

#include <windows.h> //--- ������ ��� ����
#include <winsock2.h> 
#include <ws2tcpip.h> 
#include <tchar.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <atlImage.h> // cimg ������ �ʿ��� ���
#include <vector>
#include <string>
#include <fstream>
#include "FMOD\FMOD Studio API Windows\api\core\inc\fmod.hpp"
#include "FMOD\FMOD Studio API Windows\api\core\inc\fmod_errors.h" 
#include <random>
#include <time.h>
#include <thread>
#include<array>

#include <protocol.h>
	// #include <enum.h>
using namespace std;

constexpr int g = 10;
constexpr float t = 0.19; 
constexpr int side = 60; 
constexpr float rd = 12.5;
constexpr int SVMAPCNT = 24;

constexpr const char* serverIP = "asdf";
constexpr const WORD serverPort = 9000;

#define MAKE_SINGLETON(T)		\
private:						\
	T() {}						\
	~T() {}						\
public:							\
	static T& GetInstance()		\
	{							\
		static T instance;		\
		return instance;		\
	}							\

#define INSTANCE(T) T::GetInstance()
