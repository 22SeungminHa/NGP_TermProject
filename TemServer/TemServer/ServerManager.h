#pragma once
#include "Session.h"

class ServerManager
{
public:
	SOCKET s_sock, c_sock, listen_sock;
	array<Session, MAX_USER> clients;
	concurrency::concurrent_priority_queue<std::shared_ptr<PACKET>> sendPacketQ;
	CRITICAL_SECTION cs;
	int retval;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	ServerManager();
	~ServerManager();

	void S_Bind_Listen();
	void S_Accept();
	void MakeThreads();
	void WorkThreads();
	void Do_timer();
	void Disconnect(int c_id);
	void ProcessPacket(int c_id, char* packet);

	void err_quit(const char* msg);
	void err_display(const char* msg);
	void err_display(int errcode);
};
