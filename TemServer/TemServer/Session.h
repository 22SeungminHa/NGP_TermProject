#pragma once
#include "stdafx.h"

class ServerManager;

struct floatRECT {
	float left, top, right, bottom;
};

#define EPSILON	1.0e-6f
inline bool IsZero(float fValue) {
	return((fabsf(fValue) < EPSILON));
}
inline bool IsEqual(float fA, float fB) {
	return(IsZero(fA - fB));
}

struct Ball {
	float x = -999, y = 12.5, vx, vy, ax, remx, remy;

	bool SameBall(Ball a, Ball b) {
		return IsEqual(a.x, b.x) && IsEqual(a.y, b.y);
	}
	void BallXYCopy(Ball& a, Ball& in) {
		a.x = in.x;
		a.y = in.y;
	}
};

struct Block {
	int x, y, type, subtype = 0;
};

struct CrashedBk {
	int dir, i, j;
	float x, y;
	int quality;
};

class Session
{
public:
	Ball				ball, last_send_ball;
	char				name[NAME_SIZE];
	bool				isLeftPressed, isRightPressed;
	int					GamePlay;
	vector <Block>		block[15];
	vector <CrashedBk>	crash;
	RECT				window;
	floatRECT			ballrc;
	Block				list[43];

	std::array<std::array<char, 25>, 15>		Map;
	int					starcnt;
	bool				isSwitchOff;
	int					Scheck;
	int					score;

	int stage;

	void Initialize();

	void CrashExamin();
	void Crash(int dir, int i, int y);
	int BlockQuality(const Block* block);
	int MyIntersectRect(const floatRECT* ballrc, const floatRECT* blockrc);
	int isCrashed(const floatRECT* ballrc, const floatRECT* blockrc);
	Block* Search(const int type);
	bool CrashBottom();

	void CrashBasicRight(const Block* block);
	void CrashBasicLeft(const Block* block);
	void CrashBasicBottom(const Block* block);
	void CrashBasicTop(const Block* block);

	void MoveBall();

	void ClearVector();
	void MakeVector();

	void CustomMapSave();

	int		id;
	SOCKET	sock;
	ServerManager* serverManager;
	int		recv_remain = 0;
	char	save_buf[BUFSIZE * 2] = { 0 };

	DWORD Do_Recv(LPVOID arg);

	void AddPacketToQueue(std::shared_ptr<PACKET> packet);

	void Send_login_info_packet(Session* client);
	void Send_edit_map_packet(Session* client);
	void Send_load_map_packet(Session* client);
	void Send_logout_packet(Session* client);
	void Send_game_state_packet(Session* client);
	void Send_sound_state_packet(Session* client);
	void Send_edit_map_packet(Session* client);

	bool CustomMapSave(char*);
};

