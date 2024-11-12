#pragma once
#include "stdafx.h"

struct floatRECT {
	float left, top, right, bottom;
};

struct Ball {
	float x = 30, y = 10, vx, vy, ax, remx, remy;
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
	RECT window{};

	Ball ball{};
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
	~ClientManager() {}

public:
	bool Initialize();

	void ConnectWithServer();
	void LoginToGame();

	void SendLoginPacket(int sock, char* name);
	void SendKeyPacket(int sock, int key);

	void ReceivePlayerID();
	void ReceiveServerData();

	void UsingPacket(char* id);

	void LoadMap(char* map);
	void ClearVector();
	void MakeVector();

private:
	void err_quit(const char* msg);
	void err_display(const char* msg);
	void err_display(int errcode);
};
