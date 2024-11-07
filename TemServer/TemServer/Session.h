#pragma once
#include "stdafx.h"

struct Ball {
	float x = 30, y = 10, vx, vy, ax, remx, remy;
};

struct Block {
	int x, y, type, subtype, ani = 0;
};

struct CrashedBk {
	int dir, i, j, quality;
	float x, y;
};

struct floatRECT {
	float left, top, right, bottom;
};

class Session
{
public:
	Ball				ball;
	bool				isLeftPressed, isRightPressed;
	int					GamePlay;
	vector <Block>		block[15];
	vector <Block>		bullet;
	vector <Block>		Readyblock[4];
	vector <Block>		animation;
	vector <CrashedBk>	crash;
	RECT				window;
	floatRECT			ballrc;
	Block				list[43];
	int					Map[15][25];
	int					SurvivalMap[SVMAPCNT][4][9];
	int					starcnt;
	bool				isSwitchOff;
	int					Scheck;
	int					score;
	int					blockDown;
	int					random;
	int					PrintLc;

	void CrashExamin();
	int MyIntersectRect(const floatRECT* ballrc, const floatRECT* blockrc);
	int isCrashed(const floatRECT* ballrc, const floatRECT* blockrc);
	void Crash(int dir, int i, int y);
	int BlockQuality(const Block* block);
	Block* Search(const int type);
	void CrashBasicRight(const Block* block);
	void CrashBasicLeft(const Block* block);
	void CrashBasicBottom(const Block* block);
	void CrashBasicTop(const Block* block);
	void MakeBullet(const Block* block, int BulletType);
	void MoveBullet();
	void CrashBullet();
	bool OnceMvBkGo(const Block* b);
	void MoveOnceMvBk(int y, int i);
	void MoveBall();
	bool MoveMoveBk(Block* b);
	void TurnMoveBk(Block* b);
	void MakeBlockList();
	void UseItem();
	void ClearVector();
	void MakeReadyVector();
	void ClearReadyVector();
	void MakeVector();
	void Initialize();

	void Send_login_info_packet(Session* client);
	void Send_move_ball_packet(Session* client);
	void Send_death_packet(Session* client);
	void Send_edit_map_packet(Session* client);
	void Send_respawn_packet(Session* client);
	void Send_change_state_packet(Session* client);
	void Send_move_block_packet(Session* client);
	void Send_load_map_packet(Session* client);
	void Do_Recv();
	void Do_Send();
	void AddPacketToQueue(std::shared_ptr<PACKET> packet);
};

