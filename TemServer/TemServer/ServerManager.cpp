#include "stdafx.h"
#include "ServerManager.h"
#include <fstream>
#include <filesystem>

ServerManager::ServerManager()
{
	cl_num = 0;

	float y = 10.f;

    for (unsigned int i = 0; i < clients.size(); ++i) {
        clients[i].serverManager = this;
        clients[i].id = i;
	    clients[i].ball.y = y;
		clients[i].last_send_ball.y = y;
        clients[i].last_send_ball.x = 0;
		y += 100;
    }


	MakeTimerThreads();

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
	s_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (s_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(s_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(s_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");
}

void ServerManager::S_Accept()
{
	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		c_sock = accept(s_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (c_sock == INVALID_SOCKET) {
			err_display("accept()");
			continue;
		}
		else {
			int flag = 1; // 1: Nagle 알고리즘 비활성화
			int result = setsockopt(c_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
			if (result == SOCKET_ERROR) {
				std::cerr << "setsockopt TCP_NODELAY 실패, 오류 코드: " << WSAGetLastError() << std::endl;
				continue;
			}
		}

		MakeThreads();
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

	// Session 객체를 첫 번째로 할당
	Session* session = &clients[id];
	session->sock = c_sock;

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

void ServerManager::MakeTimerThreads()
{
	std::thread timerThread([this]() { Do_timer(); });
	timerThread.detach();  // 스레드를 분리하여 백그라운드에서 실행되도록 함
}

void ServerManager::Do_timer()
{
	while (true) {
		bool isMoved{ false };

		for (Session& client : clients) {
			if (client.GamePlay != StagePlay && client.GamePlay != CustomPlay) continue;

			Ball& ball = client.ball;
			if (ball.x != -999) {
				client.MoveBall();
				client.ballrc = { (float)ball.x - rd, (float)ball.y - rd, (float)ball.x + rd, (float)ball.y + rd };

				if (!client.CrashBottom())
					client.CrashExamin();
			}
			if (!ball.SameBall(client.last_send_ball, ball)) {
				ball.BallXYCopy(client.last_send_ball, ball);
				isMoved = true;
			}
		}
		if (isMoved) {
			Send_frame_packet();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(30)); // 3ms 대기
	}
}

void ServerManager::Disconnect(int c_id)
{
	clients[c_id].ball.x = -999;
	closesocket(clients[c_id].sock);

	for (auto& c : clients) {
		if (c.ball.x != -999)
			c.Send_logout_packet(&clients[c_id]);
	}
}

void ServerManager::ProcessPacket(int c_id, char* packet)
{
	switch (packet[2])
	{
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		if (p->sessionID == 7) {
			Session cl;

			for (auto& c : clients) {
				if (c.ball.x == -999) {
					c.ball.x = 30;
					cl.id = c.id;
					clients[cl.id].Initialize();
					strncpy(p->name, clients[cl.id].name, p->size - 7);
					break;
				}
			}

			for (auto& c : clients) {
				if (c.ball.x == -999) continue;
				c.Send_login_info_packet(&cl);
			}

			if (cl.id > 0) {
				for (auto& c : clients) {
					if (c.ball.x == -999 || cl.id == c.id) continue;
					clients[cl.id].Send_login_info_packet(&c);
				}
			}
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
			if (p->keyState == KEY_STATE::DOWN) {
				if (client.isLeftPressed == false) {
					if (IsEqual(client.ball.vy, 5)) {
						client.ball.vy = -40;
						client.ball.vx = -21;
					}
					else if (IsEqual(client.ball.vy, 5.1)) {
						client.ball.vx = 21;
						client.ball.vy = -40;
					}
				}
				client.isLeftPressed = true;
			}
			else if (p->keyState == KEY_STATE::UP) {
				client.isLeftPressed = false;
			}

			break;
		}
		case KEY_TYPE::RIGHT: {
			if (p->keyState == KEY_STATE::DOWN) {
				if (client.isRightPressed == false) {
					if (IsEqual(client.ball.vy, 5)) {
						client.ball.vx = -21;
						client.ball.vy = -40;
					}
					else if (IsEqual(client.ball.vy, 5.1)) {
						client.ball.vy = -40;
						client.ball.vx = 21;
					}
				}
				client.isRightPressed = true;
			}
			else if (p->keyState == KEY_STATE::UP) {
				client.isRightPressed = false;

			}
			break;
		}
		case KEY_TYPE::ESCAPE: {
			if (client.GamePlay == StagePlay)
				client.GamePlay = StageStop;
			else if (client.GamePlay == StageStop)
				client.GamePlay = StagePlay;
			else if (client.GamePlay == CustomStop) {
				client.GamePlay = CustomPlay;
			}
			else if (client.GamePlay == CustomSelect || client.GamePlay == StageSelect || client.GamePlay == CustomSelect2)
				client.GamePlay = Start;
			else if (client.GamePlay == CustomMode) {
				client.GamePlay = CustomSelect;
				MapListLoad(p->sessionID);
			}
			else if (client.GamePlay == CustomWaiting) {
				client.GamePlay = CustomSelect;
				MapListLoad(p->sessionID);
				customWaiting[client.mapName] = false;
			}
			else if (client.GamePlay == CustomPlay)
				client.GamePlay = CustomStop;
			else if (client.GamePlay == StageClear)
				client.GamePlay = StageSelect;
			else if (client.GamePlay == StageWaiting) {
				client.GamePlay = StageSelect;
				isWaiting[client.stage] = false;
				client.stage = -1;
			}
			client.Send_game_state_packet(&client);
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
			if (client.GamePlay == Start) {
				if (MouseLC.x <= 410 && MouseLC.y >= 593 && MouseLC.y <= 693) {
					client.Scheck = click;
					client.GamePlay = StageSelect;

					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
				if (MouseLC.x <= 430 && MouseLC.y >= 717 && MouseLC.y <= 863) {
					client.Scheck = click;
					client.GamePlay = CustomSelect;

					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);

					MapListLoad(p->sessionID);
				}
			}
			else if (client.GamePlay == StageSelect) {
				if (MouseLC.x >= 93 && MouseLC.x <= 442 && MouseLC.y >= 365 && MouseLC.y <= 715) {
					EnterTheStage(client, 0);
				}
				else if (MouseLC.x >= 574 && MouseLC.x <= 923 && MouseLC.y >= 365 && MouseLC.y <= 715) {
					EnterTheStage(client, 1);
				}
				else if (MouseLC.x >= 1060 && MouseLC.x <= 1408 && MouseLC.y >= 365 && MouseLC.y <= 715) {
					EnterTheStage(client, 2);
				}
				else if (MouseLC.x >= 1368 && MouseLC.x <= 1448 && MouseLC.y >= 48 && MouseLC.y <= 128) {
					client.Scheck = click;
					client.GamePlay = Start;

					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
				else if (MouseLC.x >= 1490 && MouseLC.x <= 1500 && MouseLC.y >= 850 && MouseLC.y <= 900) {
					EnterTheStage(client, 3);
				}
			}
			else if (client.GamePlay == StageStop) {
				if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 284 && MouseLC.y <= 381) { // 메인화면 버튼 위 커서 
					client.Scheck = click;
					client.GamePlay = Start;

					Send_death_packet(client.id);
					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
				else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 397 && MouseLC.y <= 494) { // 스테이지 버튼 위 커서 
					client.Scheck = click;
					client.GamePlay = StageSelect;

					Send_death_packet(client.id);
					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
				else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 509 && MouseLC.y <= 606) { // 재시작 버튼 위 커서
					client.Scheck = click;
					client.GamePlay = StageDeath;

					Send_death_packet(client.id);
					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
			}
			else if (client.GamePlay == CustomStop) {
				if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 284 && MouseLC.y <= 381) { // 메인화면 버튼 위 커서 
					client.Scheck = click;
					client.GamePlay = Start;

					Send_death_packet(client.id);
					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
				else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 397 && MouseLC.y <= 494) { // 스테이지 버튼 위 커서 
					client.Scheck = click;
					client.GamePlay = CustomSelect;

					Send_death_packet(client.id);
					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
					MapListLoad(p->sessionID);
				}
				else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 509 && MouseLC.y <= 606) { // 재시작 버튼 위 커서
					client.Scheck = click;
					client.GamePlay = CustomDeath;

					Send_death_packet(client.id);
					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
			}
			else if (client.GamePlay == StageClear) {
				if (MouseLC.x >= 587 && MouseLC.x <= 587 + 674 && MouseLC.y >= 530 && MouseLC.y <= 530 + 155) {
					client.Scheck = click;
					client.GamePlay = StageSelect;

					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
			}
			else if (client.GamePlay == CustomSelect || client.GamePlay == CustomSelect2) {
				if (MouseLC.x >= 1015 && MouseLC.x <= 1350 && MouseLC.y >= 60 && MouseLC.y <= 130) {
					client.Scheck = click;
					client.GamePlay = CustomMode;

					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
				else if (MouseLC.x >= 1390 && MouseLC.x <= 1460 && MouseLC.y >= 60 && MouseLC.y <= 130) {
					client.Scheck = click;
					client.GamePlay = Start;

					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
				if (client.GamePlay == CustomSelect) {
					if (MouseLC.x >= 775 && MouseLC.x <= 815 && MouseLC.y >= 840 && MouseLC.y <= 880) {
						client.Scheck = click;

						client.GamePlay = CustomSelect2;
						client.Send_sound_state_packet(&client);
						client.Send_game_state_packet(&client);
					}
				}
				else if (client.GamePlay == CustomSelect2) {
					if (MouseLC.x >= 685 && MouseLC.x <= 720 && MouseLC.y >= 840 && MouseLC.y <= 880) {
						client.Scheck = click;

						client.GamePlay = CustomSelect;
						client.Send_game_state_packet(&client);
						client.Send_sound_state_packet(&client);
					}
				}
			}
			else if (client.GamePlay == CustomMode) {
				if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 164 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
					client.Scheck = click;

					client.GamePlay = CustomPlay;
					client.Send_game_state_packet(&client);
					client.Send_sound_state_packet(&client);
				}
			}
			break;
		}
		default:
			break;
		}

		break;
    }
	case CS_SAVE_CUSTOM_MAP: {
		CS_SAVE_CUSTOM_MAP_PACKET* p = reinterpret_cast<CS_SAVE_CUSTOM_MAP_PACKET*>(packet);

		int i = 0;
		for (int y = 0; y < M_HEIGHT; ++y) {
			for (int x = 0; x < M_WIDTH; ++x) {
				clients[p->sessionID].Map[y][x] = p->map[i++];
			}
		}

		ballStartPos[p->sessionID].x = p->x;
		ballStartPos[p->sessionID].y = p->y;
		clients[p->sessionID].isSwitchOff = p->isSwitchOff;

		clients[p->sessionID].CustomMapSave(p->mapName);
		break;
	}
	case CS_LOAD_CUSTOM_MAP_LIST: {
		CS_LOAD_CUSTOM_MAP_LIST_PACKET* p
			= reinterpret_cast<CS_LOAD_CUSTOM_MAP_LIST_PACKET*>(packet);

		MapListLoad(p->sessionID);
		break;
	}
	case CS_SELECT_LOAD_CUSTOM_MAP: {
		CS_SELECT_LOAD_CUSTOM_MAP_PACKET* p 
			= reinterpret_cast<CS_SELECT_LOAD_CUSTOM_MAP_PACKET*>(packet);

		u_int sessionID = p->sessionID;
		Session& client = clients[sessionID];

		std::string mapName = p->mapName;

		if (!customWaiting[mapName]) {
			client.GamePlay = CustomWaiting;
			client.Send_game_state_packet(&client);
			customWaiting[mapName] = true;
			client.mapName = mapName;
		}
		else {
			MapLoad(p->sessionID, p->mapName);

			for (auto& c : clients) {
				if (c.id != client.id && c.mapName == mapName) {
					c.GamePlay = CustomPlay;
					c.Send_game_state_packet(&c);
					c.Send_load_map_packet();

					c.mapName = "";

					client.GamePlay = CustomPlay;
					client.Send_game_state_packet(&client);
					client.Send_load_map_packet();

					customWaiting[mapName] = false;
					break;
				}
			}
		}
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
		std::cerr << "[Do_Send()] Packet session ID: " << sessionID << std::endl;
		return;
	}

	Session& session = clients[sessionID];

	// 전송할 데이터
	int retval = send(session.sock, reinterpret_cast<const char*>(&(*packet)), packet->size, 0);

	if (retval == SOCKET_ERROR) {
		std::cerr << "[send()] Failed to send Packet" << (int)packet->packetID << " to Session" << sessionID << std::endl;
	}
	else {
		//std::cout << "Successed to send Packet" << (int)packet->packetID << " to Session" << sessionID << std::endl;
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
	}
}

void ServerManager::Send_frame_packet()
{
	for (auto& c : clients) {
		auto packet = std::make_shared<SC_FRAME_PACKET>(c.id);

		packet->c1_id = clients[0].id;
		packet->x1 = clients[0].ball.x;
		packet->y1 = clients[0].ball.y;
		packet->c2_id = clients[1].id;
		packet->x2 = clients[1].ball.x;
		packet->y2 = clients[1].ball.y;

		c.AddPacketToQueue(packet);
		//cout << "Send_frame_packet 완료     " << packet->c1_id << ">>" << packet->x1 << ", " << packet->y1 << endl;
	}
}

void ServerManager::Send_death_packet(int deathID)
{
	for (auto& c : clients) {
		auto packet = std::make_shared<SC_DEATH_PACKET>(c.id);

		packet->c1_id = deathID;

		c.AddPacketToQueue(packet);
		//cout << "Send_frame_packet 완료     " << packet->c1_id << ">>" << packet->x1 << ", " << packet->y1 << endl;
	}
	if (clients[0].GamePlay == StageDeath && clients[1].GamePlay == StageDeath) {
		for (auto& c : clients) {
			Block temp{ 0, 0, SwitchBk, isSwitchOff };
			c.GamePlay = StagePlay;
			c.GameInitialize();
			c.ball.x = ballStartPos[c.id].x * side + side / 2;
			c.ball.y = ballStartPos[c.id].y * side + side / 2;
			c.Send_game_state_packet(&c);
			c.Send_load_map_packet();
			Send_edit_map_packet(&temp, 0, 0);
		}
	}
	else if (clients[0].GamePlay == CustomDeath && clients[1].GamePlay == CustomDeath) {
		for (auto& c : clients) {
			Block temp{ 0, 0, SwitchBk, isSwitchOff };
			c.GamePlay = CustomPlay;
			c.GameInitialize();
			c.ball.x = ballStartPos[c.id].x * side + side / 2;
			c.ball.y = ballStartPos[c.id].y * side + side / 2;
			c.Send_game_state_packet(&c);
			c.Send_load_map_packet();
			Send_edit_map_packet(&temp, 0, 0);
		}
	}
}

void ServerManager::Send_edit_map_packet(Block* block, int i, int y)
{
	for (auto& c : clients) {
		auto packet = std::make_shared<SC_EDIT_MAP_PACKET>(c.id);
		memcpy(packet->block, block, sizeof(Block));
		c.AddPacketToQueue(packet);

		switch (block->type) {
		case Star: {
			for (int j = 0; j < c.crash.size(); j++) {
				if (y == c.crash[j].j && i < c.crash[j].i)
					c.crash[j].i -= 1;
			}
			c.starcnt--;
			if (c.starcnt == 0) {
				if (c.GamePlay == StagePlay || c.GamePlay == StageDeath) {// 별 먹고 죽었을 때도 클리어되게,,, 동시에 일어나도 WM_TIMER가먼저 돌아가서 아마 death가 먼저 될거라 괜찮을거같긴한데 버그나면뭐,, 아쉬운거임
					c.Scheck = gameclear;
					c.GamePlay = StageClear;
					c.Send_game_state_packet(&c);
				}
				else if (c.GamePlay == CustomPlay || c.GamePlay == CustomDeath) {
					c.GamePlay = CustomMode;
					c.Send_game_state_packet(&c);
				}
			}
			break;
		}
		case SwitchBk: {
			c.isSwitchOff = block->subtype;
			break;
		}
		}
	}
	if (block->type == Star) {
		for (auto& c : clients) {
			c.block[y].erase(c.block[y].begin() + i);
		}
	}
}

void ServerManager::EnterTheStage(Session& client, int stageNum)
{
	if (!isWaiting[stageNum] && client.stage == -1) {
		client.GamePlay = StageWaiting;
		client.Send_game_state_packet(&client);
		isWaiting[stageNum] = true;
		client.stage = stageNum;
	}
	else {
		MapLoad(stageNum + 1);
		for (auto& c : clients) {
			if (c.id != client.id && c.stage == stageNum) {
				c.GamePlay = StagePlay;
				c.Send_game_state_packet(&c);
				c.Send_load_map_packet();
				c.stage = -1;
			}
			client.GamePlay = StagePlay;
			client.Send_game_state_packet(&client);
			client.Send_load_map_packet();

			break;
		}
		isWaiting[stageNum] = false;
	}
}

void ServerManager::MapLoad(int mapNumber)
{
	std::string fileName = "Map/Stage" + std::to_string(mapNumber) + ".txt";
	ifstream in{ fileName };

	if (!in.is_open()) {
		std::cerr << "Error: Cannot open file " << fileName << std::endl;
		return;
	}


	int data{};
	for (auto& row : Map) {
		for (auto& cell : row) {
			in >> data;
			cell = (int)data;
		}
	}
	
	for (auto& pos : ballStartPos) {
		in >> pos.x;
		in >> pos.y;
	}
	in >> isSwitchOff;

	in.close();

	for (auto& client : clients) {
		if (client.ball.x == -999) continue;
		client.Map = Map;
		client.isSwitchOff = isSwitchOff;

		client.ball.x = ballStartPos[client.id].x * side + side / 2;
		client.ball.y = ballStartPos[client.id].y * side + side / 2;
		client.GamePlay = StageDeath;
		client.MakeVector();
		client.GamePlay = StagePlay;
	}
}

void ServerManager::MapLoad(int c_id, char* mapName)
{
	std::string fileName = mapName;
	fileName = "CustomMap/" + fileName;
	ifstream in{ fileName };

	if (!in.is_open()) {
		std::cerr << "Error: Cannot open file " << fileName << std::endl;
		return;
	}

	int data{};
	for (auto& row : Map) {
		for (auto& cell : row) {
			in >> data;
			cell = (int)data;
		}
	}

	for (auto& pos : ballStartPos) {
		in >> pos.x;
		in >> pos.y;
	}
	in >> isSwitchOff;

	in.close();

	for (auto& client : clients) {
		if (client.ball.x == -999) continue;
		client.Map = Map;
		client.isSwitchOff = isSwitchOff;

		client.ball.x = ballStartPos[client.id].x * side + side / 2;
		client.ball.y = ballStartPos[client.id].y * side + side / 2;
		client.MakeVector();
	}
}

void ServerManager::MapListLoad(int c_id)
{
	// 디렉터리 경로 검사
	if (!std::filesystem::exists("CustomMap")) {
		std::cerr << "Error: Directory CustomMap does not exist!" << std::endl;
		return;
	}

	if (!std::filesystem::is_directory("CustomMap")) {
		std::cerr << "Error: CustomMap is not a directory!" << std::endl;
		return;
	}

	// 파일 이름 저장
	string fileNames{};

	// 디렉터리 내 파일 이름 읽기
	for (const auto& entry : std::filesystem::directory_iterator("CustomMap")) {
		if (entry.is_regular_file()) { // 파일만 추가
			std::string fileName = entry.path().filename().string();
			fileNames = fileNames + fileName + " ";

			if (!customWaiting.contains(fileName)) {
				customWaiting[fileName] = false;
			}
		}
	}

	clients[c_id].Send_load_custom_map_list_packet(fileNames);
}

