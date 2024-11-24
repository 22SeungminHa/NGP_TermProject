#pragma once
#include "client_pch.h"

struct floatRECT {
	float left, top, right, bottom;
};

struct Ball {
	float x = 30, y = 10, vx, vy, ax, remx, remy;
	u_short playerID{};
};

struct Block {
	int x, y, type, subtype, ani = 0;
};

struct CrashedBk {
	int dir, i, j;
	float x, y;
	int quality;
};

class ClientManager
{
public:
	HWND hwnd;
	RECT window{};

	Ball ball{};
	POINT ballStartPos{};
	std::vector<Ball> otherPlayers{};

	floatRECT ballrc{};

	bool isLeftPressed{};
	bool isRightPressed{};
	bool isSwitchOff{};

	std::vector <Block> block[15]{};
	std::vector <Block> animation{};
	std::vector <CrashedBk> crash{};

	Block list[24]{};
	int Map[15][25]{};

	int GamePlay{};
	int starcnt{};
	int Scheck{};
	int score{};
	int blockDown{};
	int random{};
	int PrintLc{};

	int retval{};
	SOCKET clientSocket{};
	

public:
	ClientManager() {}
	~ClientManager();

public:
	bool Initialize(HWND _hwnd);
	void Destroy();

	bool ConnectWithServer();
	void LoginToGame();

	bool SendLoginPacket(int sock, char* name);
	bool SendKeyPacket(int sock, KEY_TYPE key);
	bool SendMousePacket(KEY_TYPE key, POINT mousePos);

	bool ReceivePlayerID();
	bool ReceiveServerData();

	void UsingPacket(char* buffer);

	void LoadMap(char* map);
	void ClearVector();
	void MakeVector();

private:
	void err_quit(const char* msg);
	void err_display(const char* msg);
	void err_display(int errcode);
	void log_display(const std::string& msg);
};

