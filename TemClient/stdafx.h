#pragma once

#define _CRT_NON_CONFORMINGS_SWPRINTFS
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment (lib, "fmod_vc.lib")

#include <windows.h> //--- ������ ��� ����
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
#include <protocol.h>
	// #include <enum.h>

#define g 10 // �߷°��ӵ�
#define t 0.19 // �ӵ� ���� ����
#define side 60 // �� �� ���� ����
#define rd 12.5 // �� ������
#define SVMAPCNT 24
