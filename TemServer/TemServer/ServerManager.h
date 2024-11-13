#pragma once
#include "Session.h"

// thread에게 넘겨주기 위함
struct ThreadArgs {
	int num;
	SOCKET client_sock;
};

class ServerManager
{
public:
	SOCKET s_sock, c_sock, listen_sock;
	concurrency::concurrent_priority_queue<std::shared_ptr<PACKET>> sendPacketQ;
	CRITICAL_SECTION cs;
	int retval;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	array<Session, MAX_USER> clients;
	int cl_num;

	ServerManager();
	~ServerManager();

	void S_Bind_Listen();
	void S_Accept();
	void MakeThreads();
	void WorkThreads();
	void Do_timer();
	void Disconnect(int c_id);
	void ProcessPacket(int c_id, char* packet);

	static DWORD WINAPI Session_Do_Recv(LPVOID arg);

	void err_quit(const char* msg);
	void err_display(const char* msg);
	void err_display(int errcode);
};
