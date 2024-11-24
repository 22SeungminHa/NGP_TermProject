#include "stdafx.h"
#include "ServerManager.h"

ServerManager::ServerManager()
{
    cl_num = 0;

    for (unsigned int i = 0; i < clients.size(); ++i) {
        clients[i].serverManager = this;
        clients[i].id = i;
    }

	S_Bind_Listen();

    MakeSendThreads();

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
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");
}

void ServerManager::S_Accept()
{
    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        c_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (c_sock == INVALID_SOCKET) {
            err_display("accept()");
            return;
        }

        MakeThreads();
    }
}

void ServerManager::MakeThreads()
{
    int id;
    for (auto& c : clients) {
        if (c.ball.x == -999) id = c.id;
    }

    // Session 객체를 첫 번째로 할당
    Session* session = &clients[id]; 

    // std::thread로 스레드 생성
    std::thread recvThread([session]() {
        session->Do_Recv((LPVOID)session);  // Session의 Do_Recv 호출
        });

    // 스레드 종료를 관리하기 위해 detach() 또는 join()을 사용할 수 있습니다.
    recvThread.detach();  // 백그라운드에서 실행되도록 스레드를 분리
}

void ServerManager::MakeSendThreads()
{
    std::thread sendThread([this]() { ProcessSendQueue(); });
    sendThread.detach();  // 스레드를 분리하여 백그라운드에서 실행되도록 함
}

void ServerManager::Do_timer()
{
}

void ServerManager::Disconnect(int c_id)
{
}

void ServerManager::ProcessPacket(int c_id, char* packet)
{   
    switch (packet[2])
    {
    case CS_LOGIN: {
        CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
        strncpy(p->name, clients[p->sessionID].name, p->size - 7);

        for (auto& c : clients) {
            c.Send_login_info_packet();
        }

        break;
    }
    case CS_KEY_PRESS: {
        CS_KEY_PACKET* p = reinterpret_cast<CS_KEY_PACKET*>(packet);

        switch (p->keyType)
        {
        case KEY_TYPE::LEFT: {
            break;
        }
        case KEY_TYPE::RIGHT: {
            break;
        }
        case KEY_TYPE::ESCAPE: {
            break;
        }
        case KEY_TYPE::SPACE: {
            break;
        }
        case KEY_TYPE::LBUTTON: {
            break;
        }
        case KEY_TYPE::RBUTTON: {
            break;
        }
        default:
            break;
        }
        break;
    }
    case CS_MOUSE_POS: {
        break;
    }
    default:
        break;
    }
}

void ServerManager::Do_Send(const std::shared_ptr<PACKET>& packet) 
{
    if (!packet) return;

    unsigned int sessionID = packet->sessionID;
    if (sessionID >= clients.size()) {
        std::cerr << "Do_Send() Packet session ID: " << sessionID << std::endl;
        return;
    }

    Session& session = clients[sessionID];

    // 전송할 데이터
    int retval = send(session.sock, reinterpret_cast<const char*>(&(*packet)), packet->size, 0);

    if (retval == SOCKET_ERROR) {
        std::cerr << "Failed to send packet to session " << sessionID << std::endl;
    }
}

void ServerManager::ProcessSendQueue() 
{
    while (true) {  // 무한 루프 (패킷이 있을 때 계속 처리)
        std::shared_ptr<PACKET> packet;

        // 큐에서 패킷을 가져옴
        if (sendPacketQ.try_pop(packet)) {
            // 큐에서 꺼낸 패킷을 전송
            Do_Send(packet);
        }

        // 33.33ms 대기 (30프레임 / 1초 기준)
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 33ms 대기
    }
}

void ServerManager::Send_frame_packet()
{
    auto p = std::make_shared<SC_FRAME_PACKET>(clients[0].id);

    p->c1_id = clients[0].id;
    p->x1 = clients[0].ball.x;
    p->y1 = clients[0].ball.y;
    p->c2_id = clients[1].id;
    p->x2 = clients[1].ball.x;
    p->y2 = clients[1].ball.y;

    clients[0].AddPacketToQueue(p);

    p->sessionID = clients[1].id;
    clients[1].AddPacketToQueue(p);
}