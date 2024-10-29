#pragma once
#include "stdafx.h"

struct floatRECT {
	float left, top, right, bottom;
};

struct Ball {
	float x = 30, y = 10, vx, vy, ax, remx, remy;
	int item; // 아이템 먹은 상태
	int state; // 별 먹을 수 있는 상태
};

struct Block {
	int x, y, type, subtype, ani = 0;
};

struct CrashedBk {
	int dir, i, j;
	float x, y;
	int quality;
};

class GameManager
{
public:
	Ball ball = { 30, 12.5, 0, 0, 0, Normal, Normal };
	bool isLeftPressed = false, isRightPressed = false;
	int GamePlay = Start;
	std::vector <Block> block[15]{}, bullet{}, Readyblock[4]{};
	std::vector <Block> animation{};
	std::vector <CrashedBk> crash{};
	floatRECT ballrc{};
	Block list[43]{};
	int Map[15][25]{}, SurvivalMap[SVMAPCNT][4][9]{}, starcnt = 0;
	bool isSwitchOff{};
	int Scheck = 0, score = 0, blockDown = 0, random{}, PrintLc = 3;

	void CrashExamin();
	int MyIntersectRect(const floatRECT* ballrc, const floatRECT* blockrc);
	int isCrashed(const floatRECT* ballrc, const floatRECT* blockrc);
	void Crash(int dir, int i, int y);
	void CrashBasicRight(const Block* block);
	void CrashBasicLeft(const Block* block);
	void CrashBasicBottom(const Block* block);
	void CrashBasicTop(const Block* block);

	Block* Search(const int type);
	int BlockQuality(const Block* block);
	void MakeBullet(const Block* block, int BulletType);
	void MoveBullet();
	void CrashBullet();
	bool OnceMvBkGo(const Block* b);
	void MoveOnceMvBk(int y, int i);
	bool MoveMoveBk(Block* b);
	void TurnMoveBk(Block* b);

	void MoveBall();
	void UseItem();

	void MakeBlockList();
	void ClearVector();
	void MakeReadyVector();
	void ClearReadyVector();
	void MakeVector();
};

