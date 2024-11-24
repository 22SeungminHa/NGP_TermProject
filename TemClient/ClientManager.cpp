#include "ClientManager.h"

ClientManager::~ClientManager()
{
	
}

bool ClientManager::Initialize(HWND _hwnd)
{
	hwnd = _hwnd;
	GetClientRect(hwnd, &window);

	ball = { 30, 12.5, 0, 0, 0 };
	isLeftPressed = false, isRightPressed = false;
	GamePlay = Start;
	starcnt = 0;
	isSwitchOff = false;
	Scheck = 0, score = 0, blockDown = 0, random = 0, PrintLc = 3;

	// 맵툴 블럭 리스트
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
}

bool ClientManager::SendLoginPacket(int sock, char* name)
{
}

bool ClientManager::SendKeyPacket(int sock, KEY_TYPE key)
{
	CS_KEY_PACKET keyPacket(ball.playerID);
	keyPacket.keyType = key;

	retval = send(clientSocket, (char*)&keyPacket, sizeof(CS_KEY_PACKET), 0);

	if (retval == SOCKET_ERROR) {

		err_display("send()");
		return false;
	}
	return true;
}

bool ClientManager::SendMousePositionPacket(POINT mousePos)
{
	
	CS_MOUSE_POSITION_PACKET mousePacket(ball.playerID);
	mousePacket.mousePos = mousePos;

	retval = send(clientSocket, (char*)&mousePacket, sizeof(CS_MOUSE_POSITION_PACKET), 0);

	if (retval == SOCKET_ERROR) {

		err_display("send()");
		return false;
	}

	return true;
}

bool ClientManager::ReceivePlayerID()
{
}

bool ClientManager::ReceiveServerData()
{
	char buffer[1024];
	int retval = recv(clientSocket, buffer, sizeof(buffer), 0);

	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		return false;
	}
	else if (retval == 0) {
		return false;
	}

	PACKET* pPacket = reinterpret_cast<PACKET*>(buffer);

	if (retval < pPacket->size) {
		return false;
	}

	UsingPacket(buffer);

	return true;
}

void ClientManager::UsingPacket(char* buffer)
{
	PACKET* pPacket = reinterpret_cast<PACKET*>(buffer);

	switch (pPacket->packetID) {
	case SC_LOGIN_INFO: {
		SC_LOGIN_INFO_PACKET* loginInfoPacket = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(buffer);
		ball.playerID = loginInfoPacket->c_id;
		log_display("SC_LOGIN_INFO_PACKET\nc_id = " + std::to_string(loginInfoPacket->c_id));
		break;
	}
	case SC_MOVE_BALL: {
		SC_MOVE_BALL_PACKET* moveBallPacket = reinterpret_cast<SC_MOVE_BALL_PACKET*>(buffer);
		log_display("SC_MOVE_BALL_PACKET\nc_id = " + std::to_string(moveBallPacket->c_id) +
			", x = " + std::to_string(moveBallPacket->x) +
			", y = " + std::to_string(moveBallPacket->y));
		break;
	}
	case SC_DEATH: {
		SC_DEATH_PACKET* deathPacket = reinterpret_cast<SC_DEATH_PACKET*>(buffer);
		log_display("SC_DEATH_PACKET\nc_id = " + std::to_string(deathPacket->c1_id) +
			", x = " + std::to_string(deathPacket->x) +
			", y = " + std::to_string(deathPacket->y));
		break;
	}
	case SC_EDIT_MAP: {
		SC_EDIT_MAP_PACKET* editMapPacket = reinterpret_cast<SC_EDIT_MAP_PACKET*>(buffer);
		log_display("SC_EDIT_MAP_PACKET\nx = " + std::to_string(editMapPacket->x) +
			", y = " + std::to_string(editMapPacket->y) +
			", block = " + std::to_string(editMapPacket->block));
		break;
	}
	case SC_LOAD_MAP: {
		SC_LOAD_MAP_PACKET* loadMapPacket = reinterpret_cast<SC_LOAD_MAP_PACKET*>(buffer);
		log_display("SC_LOAD_MAP_PACKET");
		break;
	}
	case SC_RESPAWN: {
		SC_RESPAWN_PACKET* respawnPacket = reinterpret_cast<SC_RESPAWN_PACKET*>(buffer);
		log_display("SC_RESPAWN_PACKET\nc_id = " + std::to_string(respawnPacket->c_id) +
			", x = " + std::to_string(respawnPacket->x) +
			", y = " + std::to_string(respawnPacket->y));
		break;
	}
	default:
		log_display("Unknown packet received: ID = " + std::to_string(pPacket->packetID));
		break;
	}
}

// 맵 배열에서 벡터로 변환 (공 좌표, 스위치 상태는 따로 받기)
void ClientManager::MakeVector() {
	ClearVector();
	Block temp;
	int groupcnt = 1; // 이동블럭 그룹
	bool Continuous = false;
	starcnt = 0;

	if (GamePlay == StageDeath || GamePlay == CustomDeath || GamePlay == CustomPlay) {
		for (int i = 0; i < 15; i++) {
			for (int j = 0; j < 25; j++) {
				if (Map[i][j]) { // 블럭일 경우
					// 별
					if (Map[i][j] == 1)
						starcnt++;

					temp.x = Map[i][j] - 1 == 17 ? j * side : j;
					temp.y = Map[i][j] - 1 == 17 ? i * side : i;
					temp.type = list[Map[i][j] - 1].type;
					if (Map[i][j] - 1 == 13 || Map[i][j] - 1 == 14 || Map[i][j] - 1 == 15) // 전기 관련 블럭
						temp.subtype = isSwitchOff;
					else
						temp.subtype = list[Map[i][j] - 1].subtype;
					if (Continuous) {
						if (Map[i][j] - 1 == 17 && Map[i][j - 1] - 1 == 17) temp.ani = groupcnt;
						else {
							Continuous = false;
							groupcnt++;
						}
					}
					if (Map[i][j] - 1 == 17) {
						temp.ani = groupcnt;
						Continuous = true;
					}
					if (Map[i][j] - 1 != 17)
						temp.ani = list[Map[i][j] - 1].ani;

					// 끈끈이 그룹화
					if (Map[i][j] == 20) {
						// 맵 가장 위이거나, 맵 가장 아래가 아니고 블럭 위가 끈끈이가 아니고 아래가 끈끈이면 1번
						if (i == 0 || i < 14 && Map[i - 1][j] != 20 && Map[i + i][j] == 20)
							temp.subtype = 1;
						// 맵 가장 위나 아래가 아니고 블럭 위와 아래가 끈끈이면 2번
						else if (i > 0 && i < 14 && Map[i - 1][j] == 20 && Map[i + 1][j] == 20)
							temp.subtype = 2;
						// 맵 가장 아래이거나, 맵 가장 위가 아니고 블럭 위가 끈끈이고 아래가 끈끈이가 아니면 2번
						else if (i == 14 || i > 0 && Map[i - 1][j] == 20 && Map[i + 1][j] != 20)
							temp.subtype = 3;
						else
							temp.subtype = 4;
					}
					block[i].emplace_back(temp);
				}
			}
		}
	}
}

void ClientManager::LoadMap(char* map)
{
}

void ClientManager::ClearVector() { // 걍 다 초기화하게함
	animation.clear();
	for (int i = 0; i < 15; i++) {
		block[i].clear();
	}
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
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void ClientManager::log_display(const std::string& msg)
{
	MessageBoxA(
		NULL, 
		msg.c_str(),
		"Log Message",
		MB_OK | MB_ICONINFORMATION
	);
}