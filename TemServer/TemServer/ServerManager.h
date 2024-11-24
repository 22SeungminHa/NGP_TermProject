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
	void MakeSendThreads();
	void Do_timer();
	void Disconnect(int c_id);
	void ProcessPacket(int c_id, char* packet);

	void Do_Send(const std::shared_ptr<PACKET>& packet);
	void ProcessSendQueue();

	void Send_frame_packet();
};
