#pragma once
#include <iostream>
#include <array>
//#include <WS2tcpip.h>
//#include <MSWSock.h>
#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���
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

#define _CRT_SECURE_NO_WARNINGS // ���� C �Լ� ��� �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����


#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ