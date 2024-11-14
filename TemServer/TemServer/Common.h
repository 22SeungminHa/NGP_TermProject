#pragma once
#include "stdafx.h"

class Common
{
public:
	void err_quit(const char* msg);
	void err_display(const char* msg);
	void err_display(int errcode);
};

