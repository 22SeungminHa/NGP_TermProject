#include"client_pch.h"
#include "ClientManager.h"

ClientManager::~ClientManager()
{
	
}

bool ClientManager::Initialize(HWND _hwnd)
{
	hwnd = _hwnd;
	GetClientRect(hwnd, &window);

	ball = { 30, 12.5, 0, 0, 0 };
	ball.playerID = 7;
	otherPlayer = { 30, 500, 0, 0, 0 };
	otherPlayer.playerID = 7;

	GamePlay = StagePlay;
	starcnt = 0;
	isSwitchOff = false;
	Scheck = 0;

	// ���� ���� ����Ʈ
	list[0].type = Star;
	list[1].type = JumpBk;
	list[2].type = RStraightBk;
	list[3].type = LStraightBk;
	list[4].type = RectBHBk;
	list[5].type = RectWHBk;
	list[6].type = CircleBHBk;
	list[7].type = CircleWHBk;
	list[8] = { 0, 0, SwitchBk, 0, 0 };
	list[9] = { 0, 0, SwitchBk, 1, 0 };
	list[10] = { 0, 0, ElectricBk, 0, 0 };
	list[11].type = ClimbBK;
	list[12].type = MusicBk;
	for (int i = 0; i < 11; i++) {
		list[i + 13] = { 0, 0, BasicBk, i, 0 };
	}

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	InitializeCriticalSection(&packetQueueCS);

	return true;
}

void ClientManager::Destroy()
{
	WSACleanup();
}

bool ClientManager::ConnectWithServer()
{
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket == INVALID_SOCKET) {
		err_quit("socket()");
	}

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(serverPort);

	retval = connect(clientSocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR) {
		err_quit("connect()");
		return false;
	}
	return true;
}

void ClientManager::LoginToGame()
{
	SendLoginPacket(0, "");
}

bool ClientManager::SendLoginPacket(int sock, const char* name)
{
	CS_LOGIN_PACKET loginPacket(ball.playerID);
	retval = send(clientSocket, (char*)&loginPacket, sizeof(CS_LOGIN_PACKET), 0);
	cout << ball.playerID << endl;
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return false;
	}

	return true;
}

bool ClientManager::SendKeyPacket(int sock, pair<KEY_TYPE, KEY_STATE> key)
{
	CS_KEY_PACKET keyPacket(ball.playerID);
	keyPacket.keyType = key.first;
	keyPacket.keyState = key.second;

	retval = send(clientSocket, (char*)&keyPacket, sizeof(CS_KEY_PACKET), 0);

	if (retval == SOCKET_ERROR) {

		err_display("send()");
		return false;
	}
	return true;
}

bool ClientManager::SendMousePacket(KEY_TYPE key, POINT mousePos)
{
	CS_MOUSE_PACKET mousePacket(ball.playerID);
	mousePacket.keyType = key;
	mousePacket.mousePos = mousePos;

	retval = send(clientSocket, (char*)&mousePacket, sizeof(CS_MOUSE_PACKET), 0);

	if (retval == SOCKET_ERROR) {

		err_display("send()");
		return false;
	}

	return true;
}

bool ClientManager::ReceivePlayerID()
{
	return true;
}

bool ClientManager::ReceiveServerData()
{
	char buf[BUFSIZE + 1] = { 0 };

	while(true) {
		// 데이터 수신
		int receivedBytes = recv(clientSocket, buf, sizeof(buf), 0);
		if (receivedBytes <= 0) {
			err_display("recv()");
			return 1;
		}
		//cout << "receivedBytes : " << receivedBytes << endl;

		int remain_data = receivedBytes + recv_remain; // 총 데이터 크기
		char* p = buf; // 패킷 처리 시작 위치

		// 패킷 처리 루프
		while (remain_data > 0) {
			unsigned short* byte = reinterpret_cast<unsigned short*>(p); // 패킷 크기 읽기
			int packet_size = *byte;

			// 패킷 크기 검증
			if (packet_size > BUFSIZE * 2 || packet_size < sizeof(WORD)) {
				std::cerr << "Invalid packet size: " << packet_size << std::endl;
				recv_remain = 0;
				remain_data = 0;
				break;
			}

			// 패킷 처리 가능 여부 확인
			if (packet_size <= remain_data) {
				PACKET* receivedPacket = reinterpret_cast<PACKET*>(p);
				UsingPacket(reinterpret_cast<char*>(receivedPacket));

				p += packet_size;       // 다음 패킷으로 이동
				remain_data -= packet_size;
			}
			else {
				break; // 패킷 데이터가 부족함
			}
		}

		recv_remain = remain_data;
		if (remain_data > 0) {
			// 버퍼 크기를 초과하는 데이터를 저장하려면 경고 출력
			if (remain_data > sizeof(save_buf)) {
				std::cerr << "Remaining data exceeds buffer size, data may be lost!" << std::endl;
				remain_data = sizeof(save_buf); // 데이터 크기를 버퍼 크기로 제한
			}

			memmove(save_buf, p, remain_data); // 남은 데이터를 save_buf로 이동
		}
	}

	return true;
}

void ClientManager::ProcessPackets()
{
}

void ClientManager::UsingPacket(char* buffer)
{
	PACKET* pPacket = reinterpret_cast<PACKET*>(buffer);

	switch (pPacket->packetID) {
	case SC_LOGIN_INFO: {
		auto loginInfoPacket = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(buffer);
		u_short newClientID = loginInfoPacket->c_id;
		std::cout << "SC_LOGIN_INFO_PACKET c_id = " << (int)newClientID << std::endl;

		if (ball.playerID == 7) {
			ball.playerID = newClientID;
		}
		else if( ball.playerID != newClientID){
			otherPlayer.playerID = newClientID;
		}
		break;
	}
	case SC_FRAME: {
		SC_FRAME_PACKET* framePacket = reinterpret_cast<SC_FRAME_PACKET*>(buffer);
		//std::cout << "SC_MOVE_BALL_PACKET "<<
		//	"c1_id = " << framePacket->c1_id << ", x = " << framePacket->x1 << ", y = " << framePacket->y1 << std::endl <<
		//	"c2_id = " << framePacket->c2_id << ", x = " << framePacket->x2 << ", y = " << framePacket->y2 << std::endl;

		if (framePacket->c1_id == ball.playerID) {
			ball.x = framePacket->x1;
			ball.y = framePacket->y1;

			if (otherPlayer.playerID != 999) {
				otherPlayer.x = framePacket->x2;
				otherPlayer.y = framePacket->y2;
			}
		}
		else if(framePacket->c2_id == ball.playerID){
			ball.x = framePacket->x2;
			ball.y = framePacket->y2;

			if (otherPlayer.playerID != 999) {
				otherPlayer.x = framePacket->x1;
				otherPlayer.y = framePacket->y1;
			}
		}
		break;
	}
	case SC_DEATH: {
		SC_DEATH_PACKET* deathPacket = reinterpret_cast<SC_DEATH_PACKET*>(buffer);
		std::cout << "SC_DEATH_PACKET c_id = " << deathPacket->c1_id << std::endl;
		break;
	}
	case SC_EDIT_MAP: {
		SC_EDIT_MAP_PACKET* editMapPacket = reinterpret_cast<SC_EDIT_MAP_PACKET*>(buffer);
		std::cout << "SC_EDIT_MAP_PACKET block = " << editMapPacket->block << std::endl;
		break;
	}
	case SC_LOAD_MAP: {
		SC_LOAD_MAP_PACKET* loadMapPacket = reinterpret_cast<SC_LOAD_MAP_PACKET*>(buffer);
		memcpy(Map, loadMapPacket->map, M_WIDTH * M_HEIGHT);
		std::cout << "SC_LOAD_MAP_PACKET" << std::endl;
		break;
	}
	default:
		std::cout << "[UsingPacket()] Unknown packet received: ID = " << (int)pPacket->packetID << std::endl;
		break;
	}
}

void ClientManager::LoadMap(char* map)
{
}

void ClientManager::err_quit(const char* msg)
{
	LPVOID lpMsgBuf{};

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);

	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);

}
void ClientManager::err_display(const char* msg)
{
	LPVOID lpMsgBuf{};

	FormatMessageA(

		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,

		NULL, WSAGetLastError(),

		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),

		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void ClientManager::err_display(int errcode)
{
	LPVOID lpMsgBuf{};
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[����] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void ClientManager::log_display(const std::string& msg)
{
	cout << msg << endl;
}