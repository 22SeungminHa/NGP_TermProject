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
	SOCKET s_sock, c_sock;
	concurrency::concurrent_queue<std::shared_ptr<PACKET>> sendPacketQ;
	CRITICAL_SECTION cs;
	int retval;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	array<Session, MAX_USER> clients;
	array<POINT, MAX_USER> ballStartPos{};

	bool isWaiting[4]{false};
	int cl_num;

	ServerManager();
	~ServerManager();

	void S_Bind_Listen();
	void S_Accept();
	void MakeThreads();
	void MakeSendThreads();
	void MakeTimerThreads();
	void Do_timer();
	void Disconnect(int c_id);
	void ProcessPacket(int c_id, char* packet);

	void Do_Send(const std::shared_ptr<PACKET>& packet);
	void ProcessSendQueue();

	void Send_frame_packet();
	void Send_death_packet(int deathID);

	void EnterTheStage(Session& client, int stageNum);
	void MapLoad(int mapNumber);
	void MapLoad(int c_id, char* mapName);
	void MapListLoad(int c_id);
};
