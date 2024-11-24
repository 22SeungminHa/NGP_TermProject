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
	isLeftPressed = false, isRightPressed = false;
	GamePlay = Start;
	starcnt = 0;
	isSwitchOff = false;
	Scheck = 0, score = 0, blockDown = 0, random = 0, PrintLc = 3;

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
	return true;
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
	case SC_FRAME: {
		SC_FRAME_PACKET* framePacket = reinterpret_cast<SC_FRAME_PACKET*>(buffer);
		log_display("SC_MOVE_BALL_PACKET\nc1_id = " + std::to_string(framePacket->c1_id) +
			", x = " + std::to_string(framePacket->x1) +
			", y = " + std::to_string(framePacket->y1) + 
			"\nc2_id = " + std::to_string(framePacket->c2_id) +
			", x = " + std::to_string(framePacket->x2) +
			", y = " + std::to_string(framePacket->y2));
		break;
	}
	case SC_DEATH: {
		SC_DEATH_PACKET* deathPacket = reinterpret_cast<SC_DEATH_PACKET*>(buffer);
		log_display("SC_DEATH_PACKET\nc_id = " + std::to_string(deathPacket->c1_id));
		break;
	}
	case SC_EDIT_MAP: {
		SC_EDIT_MAP_PACKET* editMapPacket = reinterpret_cast<SC_EDIT_MAP_PACKET*>(buffer);
		log_display("SC_EDIT_MAP_PACKET\nblock = " + std::to_string(editMapPacket->block));
		break;
	}
	case SC_LOAD_MAP: {
		SC_LOAD_MAP_PACKET* loadMapPacket = reinterpret_cast<SC_LOAD_MAP_PACKET*>(buffer);
		log_display("SC_LOAD_MAP_PACKET");
		break;
	}
	default:
		log_display("Unknown packet received: ID = " + std::to_string(pPacket->packetID));
		break;
	}
}

// �� �迭���� ���ͷ� ��ȯ (�� ��ǥ, ����ġ ���´� ���� �ޱ�)
void ClientManager::MakeVector() {
	ClearVector();
	Block temp;
	int groupcnt = 1; // �̵����� �׷�
	bool Continuous = false;
	starcnt = 0;

	if (GamePlay == StageDeath || GamePlay == CustomDeath || GamePlay == CustomPlay) {
		for (int i = 0; i < 15; i++) {
			for (int j = 0; j < 25; j++) {
				if (Map[i][j]) { // ������ ���
					// ��
					if (Map[i][j] == 1)
						starcnt++;

					temp.x = Map[i][j] - 1 == 17 ? j * side : j;
					temp.y = Map[i][j] - 1 == 17 ? i * side : i;
					temp.type = list[Map[i][j] - 1].type;
					if (Map[i][j] - 1 == 13 || Map[i][j] - 1 == 14 || Map[i][j] - 1 == 15) // ���� ���� ����
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

					// ������ �׷�ȭ
					if (Map[i][j] == 20) {
						// �� ���� ���̰ų�, �� ���� �Ʒ��� �ƴϰ� ���� ���� �����̰� �ƴϰ� �Ʒ��� �����̸� 1��
						if (i == 0 || i < 14 && Map[i - 1][j] != 20 && Map[i + i][j] == 20)
							temp.subtype = 1;
						// �� ���� ���� �Ʒ��� �ƴϰ� ���� ���� �Ʒ��� �����̸� 2��
						else if (i > 0 && i < 14 && Map[i - 1][j] == 20 && Map[i + 1][j] == 20)
							temp.subtype = 2;
						// �� ���� �Ʒ��̰ų�, �� ���� ���� �ƴϰ� ���� ���� �����̰� �Ʒ��� �����̰� �ƴϸ� 2��
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

void ClientManager::ClearVector() { // �� �� �ʱ�ȭ�ϰ���
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
	printf("[����] %s\n", (char*)lpMsgBuf);
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