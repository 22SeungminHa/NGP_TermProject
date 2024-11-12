#pragma once
#include "stdafx.h"

#include "Session.h"

class ServerManager
{
public:
	SOCKET s_sock, c_sock;
	array<Session, MAX_USER> clients;
	concurrency::concurrent_priority_queue<std::shared_ptr<PACKET>> sendPacketQ;
	CRITICAL_SECTION cs;

	ServerManager();
	~ServerManager();

	void S_Bind_Listen();
	void S_Accept();
	void MakeThreads();
	void WorkThreads();
	void Do_timer();
	void Disconnect(int c_id);
	void ProcessPacket(int c_id, char* packet);
};

