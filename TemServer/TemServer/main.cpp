#include "stdafx.h"
#include "ServerManager.h"

using namespace std;

ServerManager s_mng;

int main()
{
	s_mng.Worker_thread();
}
