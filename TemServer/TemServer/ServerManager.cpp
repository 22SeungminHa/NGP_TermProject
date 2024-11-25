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
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // ���� ����
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverIP, &serveraddr.sin_addr);
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
		MakeTimerThreads();
    }
}

void ServerManager::MakeThreads()
{
    int id;
    for (auto& c : clients) {
		if (c.ball.x == -999) {
			id = c.id;
			break;
		}
    }

    // Session ��ü�� ù ��°�� �Ҵ�
    Session* session = &clients[id]; 
	session->sock = c_sock;
	cout << c_sock << endl;

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

void ServerManager::MakeTimerThreads()
{
	std::thread timerThread([this]() { Do_timer(); });
	timerThread.detach();  // �����带 �и��Ͽ� ��׶��忡�� ����ǵ��� ��
}

void ServerManager::Do_timer()
{
	while (true) {
		// 33.33ms ��� (30������ / 1�� ����)
		std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 33ms ���

		clients[0].ball.x++;

		Send_frame_packet();
	}
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

        u_int sessionID = p->sessionID;
        Session& client = clients[sessionID];
            
        switch (p->keyType)
        {
        case KEY_TYPE::LEFT: {
			/*if (client.ball.vy == 5) {
				client.Scheck = telpo;
				client.ball.vy = -40;
				client.ball.vx = -21;
			}
			else if (client.ball.vy == 5.1) {
				client.Scheck = telpo;
				client.ball.vx = 21;
				client.ball.vy = -40;
			}*/
			client.ball.x -= 5;
            break;
        }
        case KEY_TYPE::RIGHT: {
			/*if (client.ball.vy == 5) {
				client.Scheck = telpo;
				client.ball.vx = -21;
				client.ball.vy = -40;
			}
			else if (client.ball.vy == 5.1) {
				client.Scheck = telpo;
				client.ball.vy = -40;
				client.ball.vx = 21;
			}*/
			client.ball.x += 5;
            break;
        }
        case KEY_TYPE::ESCAPE: {
			if (client.GamePlay == StagePlay)
				client.GamePlay = StageStop;
			else if (client.GamePlay == StageStop)
				client.GamePlay = StagePlay;
			else if (client.GamePlay == CustomMode || client.GamePlay == StageSelect)
				client.GamePlay = Start;
			else if (client.GamePlay == CustomPlay)
				client.GamePlay = CustomMode;
			else if (client.GamePlay == StageClear)
				client.GamePlay = StageSelect;
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
    /*case CS_MOUSE_POS: {
		CS_MOUSE_PACKET* p = reinterpret_cast<CS_MOUSE_PACKET*>(packet);

		u_int sessionID = p->sessionID;
		Session& client = clients[sessionID];
		POINT MouseLC = p->mousePos;

		switch (p->keyType)
		{
		case KEY_TYPE::RBUTTON: {
			break;
		}
		case KEY_TYPE::LBUTTON: {

			break;
		}
		default:
			break;
		}

		if (client.GamePlay == Start) {
			if (MouseLC.x <= 410 && MouseLC.y >= 593 && MouseLC.y <= 693) { // �������� ��ư
				client.Scheck = click;
				client.GamePlay = StageSelect;
			}
			else if (MouseLC.x <= 410 && MouseLC.y >= 717 && MouseLC.y <= 817) { // ���� ��ư
				client.Scheck = click;
				client.GamePlay = CustomMode;
				client.ballStartPos = { -1, -1 };
				client.isSwitchOff = 0;
				memset(client.Map, 0, sizeof(client.Map));
				selection = 0;
			}
		}
		else if (client.GamePlay == StageSelect) {
			if (MouseLC.x >= 93 && MouseLC.x <= 442 && MouseLC.y >= 365 && MouseLC.y <= 715) {
				client.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ���� ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage1.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> client.Map[y][x];
					}
				}

				in >> ballStartPos.x;
				in >> ballStartPos.y;
				in >> client.isSwitchOff;
				ballStartPos.x = ballStartPos.x * side + 30;
				ballStartPos.y = ballStartPos.y * side + 30;

				client.GamePlay = StageDeath;
				in.close();
			}
			else if (MouseLC.x >= 574 && MouseLC.x <= 923 && MouseLC.y >= 365 && MouseLC.y <= 715) {
				client.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ���� ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage2.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> client.Map[y][x];
					}
				}

				in >> ballStartPos.x;
				in >> ballStartPos.y;
				in >> client.isSwitchOff;
				ballStartPos.x = ballStartPos.x * side + 30;
				ballStartPos.y = ballStartPos.y * side + 30;

				client.GamePlay = StageDeath;
				in.close();
			}
			else if (MouseLC.x >= 1060 && MouseLC.x <= 1408 && MouseLC.y >= 365 && MouseLC.y <= 715) {
				client.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ���� ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage3.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> client.Map[y][x];
					}
				}

				in >> ballStartPos.x;
				in >> ballStartPos.y;
				in >> client.isSwitchOff;
				ballStartPos.x = ballStartPos.x * side + 30;
				ballStartPos.y = ballStartPos.y * side + 30;

				client.GamePlay = StageDeath;
				in.close();
			}
			else if (MouseLC.x >= 1368 && MouseLC.x <= 1448 && MouseLC.y >= 48 && MouseLC.y <= 128) {
				client.Scheck = click;
				client.GamePlay = Start;
			}
			else if (MouseLC.x >= 1490 && MouseLC.x <= 1500 && MouseLC.y >= 850 && MouseLC.y <= 900) {
				client.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ���� ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage4.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> client.Map[y][x];
					}
				}

				in >> ballStartPos.x;
				in >> ballStartPos.y;
				in >> client.isSwitchOff;
				ballStartPos.x = ballStartPos.x * side + 30;
				ballStartPos.y = ballStartPos.y * side + 30;

				client.GamePlay = StageDeath;
				in.close();
			}
		}
		else if (client.GamePlay == StageStop) {
			if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 284 && MouseLC.y <= 381) { // ����ȭ�� ��ư �� Ŀ�� 
				client.Scheck = click;
				client.GamePlay = Start;
			}
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 397 && MouseLC.y <= 494) { // �������� ��ư �� Ŀ�� 
				client.Scheck = click;
				client.GamePlay = StageSelect;
			}
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 509 && MouseLC.y <= 606) { // ����� ��ư �� Ŀ��
				client.Scheck = click;
				client.MakeVector();
				client.GamePlay = StageDeath;
				client.ball = { (float)ballStartPos.x, (float)ballStartPos.y, 0, 0, 0 }; // ����� �����ɷ� �ϸ� death�� �ٲ�� �ִϸ��̼� ������ �Ѿ�ߵż� �� �ٷ� ��������Ŵ
			}
		}
		else if (client.GamePlay == CustomMode) {
			drag = true;
			//�� ����
			if (MouseLC.y >= 756 && MouseLC.y <= 756 + 60) {
				client.Scheck = click;
				for (int i = 0; i < 14; i++) {
					if (MouseLC.x >= 17 + 60 * i + 7 * i && MouseLC.x <= 17 + 60 * i + 7 * i + 60)
						selection = i;
				}
			}
			else if (MouseLC.y >= 756 + 60 + 7 && MouseLC.y <= 756 + 60 + 7 + 60) {
				client.Scheck = click;
				for (int i = 0; i < 14; i++) {
					if (MouseLC.x >= 17 + 60 * i + 7 * i && MouseLC.x <= 17 + 60 * i + 7 * i + 60)
						selection = i + 14;
				}
			}
			// �÷��� ��ư
			else if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 164 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
				client.Scheck = click;
				if (ballStartPos.x == -1 || ballStartPos.y == -1) {
					TCHAR a[100];
					wsprintf(a, L"�� ��ġ�� �������ּ���.");
					MessageBox(hwnd, a, L"�˸�", MB_OK);
					drag = false;
					break;
				}
				client.ball = { (float)ballStartPos.x * side + 30, (float)ballStartPos.y * side + 30, 0, 0, 0 };
				client.GamePlay = CustomPlay;
				client.MakeVector();
			}
			// ���찳 ��ư
			else if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				client.Scheck = click;
				selection = -1;
			}
			// ���� ��ư
			else if (MouseLC.x >= 1325 && MouseLC.x <= 1325 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				client.Scheck = click;
				memset(client.Map, 0, sizeof(client.Map));
				ballStartPos = { -1, -1 };
			}
			// �ҷ����� ��ư
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				client.Scheck = click;
				memset(&OFN, 0, sizeof(OPENFILENAME)); //--- ����ü �ʱ�ȭ
				OFN.lStructSize = sizeof(OPENFILENAME);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = filter;
				OFN.lpstrFile = lpstrFile;
				OFN.nMaxFile = 256;
				OFN.lpstrInitialDir = L".";

				if (GetOpenFileNameW(&OFN) != 0) { //--- ���� �Լ� ȣ��
					TCHAR a[100];
					wsprintf(a, L"%s ������ ���ðڽ��ϱ� ?", OFN.lpstrFile);
					MessageBox(hwnd, a, L"���� ����", MB_OK);

					ifstream in{ OFN.lpstrFile };

					for (int y = 0; y < 15; ++y) {
						for (int x = 0; x < 25; ++x) {
							in >> client.Map[y][x];
						}
					}

					in >> ballStartPos.x;
					in >> ballStartPos.y;
					in >> client.isSwitchOff;

					in.close();
				}
				drag = false;
			}
			// ���� ��ư
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
				client.Scheck = click;
				memset(&OFN, 0, sizeof(OPENFILENAME)); //--- ����ü �ʱ�ȭ
				OFN.lStructSize = sizeof(OPENFILENAME);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = filter;
				OFN.lpstrFile = lpstrFile;
				OFN.nMaxFile = 256;
				OFN.lpstrInitialDir = L".";

				if (GetSaveFileNameW(&OFN) != 0) { //--- ���� �Լ� ȣ��
					TCHAR a[100];
					wsprintf(a, L"%s ��ġ�� ������ �����Ͻðڽ��ϱ� ?", OFN.lpstrFile);
					MessageBox(hwnd, a, L"�����ϱ� ����", MB_OK);
					TCHAR b[100];
					wsprintf(b, L"%s.txt", OFN.lpstrFile);

					ofstream out{ b };


					// �����迭 ����
					for (int y = 0; y < 15; ++y) {
						for (int x = 0; x < 25; ++x) {
							out << client.Map[y][x] << " ";
						}
						out << endl;
					}
					// �� ������ġ, ���� ���� ����
					out << ballStartPos.x << " " << ballStartPos.y << " " << client.isSwitchOff << endl;

					out.close();
				}
				drag = false;
			}
		}
		else if (client.GamePlay == StageClear) {
			if (MouseLC.x >= 587 && MouseLC.x <= 587 + 674 && MouseLC.y >= 530 && MouseLC.y <= 530 + 155) {
				client.Scheck = click;
				client.GamePlay = StageSelect;
			}
		}
        break;
    }*/
    default:
        break;
    }
}

void ServerManager::Do_Send(const std::shared_ptr<PACKET>& packet) 
{
    if (!packet) return;

    unsigned int sessionID = packet->sessionID;
    if (sessionID >= clients.size()) {
        std::cerr << "[Do_Send()] Packet session ID: " << sessionID << std::endl;
        return;
    }

    Session& session = clients[sessionID];

    // ������ ������
    int retval = send(session.sock, reinterpret_cast<const char*>(&(*packet)), packet->size, 0);

    if (retval == SOCKET_ERROR) {
        std::cerr << "[send()] Failed to send packet" << (int)packet->packetID << " to session" << sessionID << std::endl;
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