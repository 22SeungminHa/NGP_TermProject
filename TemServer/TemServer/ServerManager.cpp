#include "ServerManager.h"

ServerManager::ServerManager()
{
    cl_num = 0;

    for (unsigned int i = 0; i < clients.size(); ++i) {
        clients[i].serverManager = this;
    }

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

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
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // ���� ����
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
    // ������ ���� �غ�
    ThreadArgs* args = new ThreadArgs;
    args->num = cl_num;
    args->client_sock = c_sock;

    // Session ��ü�� ù ��°�� �Ҵ�
    Session* session = &clients[0];  // ù ��° Session ��ü ���

    // std::thread�� ������ ����
    std::thread recvThread([session]() {
        session->Do_Recv((LPVOID)session);  // Session�� Do_Recv ȣ��
        });

    // ������ ���Ḧ �����ϱ� ���� detach() �Ǵ� join()�� ����� �� �ֽ��ϴ�.
    recvThread.detach();  // ��׶��忡�� ����ǵ��� �����带 �и�
}

void ServerManager::MakeSendThreads()
{
    std::thread sendThread([this]() { ProcessSendQueue(); });
    sendThread.detach();  // �����带 �и��Ͽ� ��׶��忡�� ����ǵ��� ��
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

void ServerManager::Do_Send(const std::shared_ptr<PACKET>& packet) 
{
    if (!packet) return;

    unsigned int sessionID = packet->sessionID;
    if (sessionID >= clients.size()) {
        std::cerr << "Do_Send() Packet session ID: " << sessionID << std::endl;
        return;
    }

    Session& session = clients[sessionID];

    // ������ ������
    int retval = send(session.sock, reinterpret_cast<const char*>(&(*packet)), packet->size, 0);

    if (retval == SOCKET_ERROR) {
        std::cerr << "Failed to send packet to session " << sessionID << std::endl;
    }
}

void ServerManager::ProcessSendQueue() 
{
    while (true) {  // ���� ���� (��Ŷ�� ���� �� ��� ó��)
        std::shared_ptr<PACKET> packet;

        // ť���� ��Ŷ�� ������
        if (sendPacketQ.try_pop(packet)) {
            // ť���� ���� ��Ŷ�� ����
            Do_Send(packet);
        }

        // 33.33ms ��� (30������ / 1�� ����)
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 33ms ���
    }
}

