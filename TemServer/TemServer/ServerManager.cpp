#include "ServerManager.h"

ServerManager::ServerManager()
{
    cl_num = 0;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	S_Bind_Listen();

	S_Accept();
}
ServerManager::~ServerManager()
{
	closesocket(s_sock);
	WSACleanup();
}

void ServerManager::S_Bind_Listen()
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // 소켓 생성
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) cm.err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) cm.err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) cm.err_quit("listen()");
}

void ServerManager::S_Accept()
{
    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        c_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (c_sock == INVALID_SOCKET) {
            cm.err_display("accept()");
            return;
        }

        MakeThreads();
    }
}

void ServerManager::MakeThreads()
{
    // 스레드 인자 준비
    ThreadArgs* args = new ThreadArgs;
    args->num = cl_num;
    args->client_sock = c_sock;

    // 스레드 생성
    Session* session = &clients[0];  // 첫 번째 Session 객체 사용
    hThread = CreateThread(NULL, 0, ServerManager::Session_Do_Recv, (LPVOID)session, 0, NULL);

    if (hThread == NULL) {
        delete args;
        closesocket(c_sock);
    }
    else { CloseHandle(hThread); }
}

void ServerManager::WorkThreads()
{
}

void ServerManager::Do_timer()
{
}

void ServerManager::Disconnect(int c_id)
{
}

void ServerManager::ProcessPacket(int c_id, char* packet)
{
}

DWORD __stdcall ServerManager::Session_Do_Recv(LPVOID arg)
{
    Session* session = (Session*)arg;  // Session 객체로 변환
    return session->Do_Recv(arg);  // Session의 Do_Recv 호출
}


