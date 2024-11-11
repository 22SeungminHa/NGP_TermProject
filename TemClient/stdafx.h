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
#include <thread>

#include "protocol.h"
	// #include "enum.h"
