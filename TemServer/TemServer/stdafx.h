#pragma once
#include <iostream>
#include <array>
//#include <WS2tcpip.h>
//#include <MSWSock.h>
#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <unordered_set>
#include <concurrent_priority_queue.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#include <windows.h>  

#include "protocol.h"
	// + enum.h

using namespace std;

#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기


#pragma comment(lib, "ws2_32") // ws2_32.lib 링크