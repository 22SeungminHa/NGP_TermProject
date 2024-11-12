#include "ServerManager.h"

ServerManager::ServerManager()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	S_Bind_Listen();

	S_Accept();

	MakeThreads();
}
ServerManager::~ServerManager()
{
	closesocket(s_sock);
	WSACleanup();
}