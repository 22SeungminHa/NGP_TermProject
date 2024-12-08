#pragma once

#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기
#define _CRT_NON_CONFORMINGS_SWPRINTFS
#define _DEBUG
#pragma comment (lib, "fmod_vc.lib")

#include <winsock2.h> 
#include <ws2tcpip.h> 
#include <tchar.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <atlImage.h>
#include <vector>
#include <string>
#include<sstream>
#include <fstream>
#include "FMOD\FMOD Studio API Windows\api\core\inc\fmod.hpp"
#include "FMOD\FMOD Studio API Windows\api\core\inc\fmod_errors.h" 
#include <random>
#include <time.h>
#include <thread>
#include<array>
#include<queue>
#include <windows.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "ws2_32")

#include <protocol.h>

#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif
	// #include <enum.h>
using namespace std;



#define MAKE_SINGLETON(T)		\
private:						\
	T() {}						\
	~T() {}						\
public:							\
	static T& GetInstance()		\
	{							\
		static T instance;		\
		return instance;		\
	}							

#define INSTANCE(T) T::GetInstance()

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}