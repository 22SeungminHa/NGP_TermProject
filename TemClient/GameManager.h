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

class GameManager
{
public:
	Ball ball;
	bool isLeftPressed, isRightPressed;
	int GamePlay;
	std::vector <Block> block[15];
	std::vector <Block> animation;
	std::vector <CrashedBk> crash;
	RECT window;
	floatRECT ballrc;
	Block list[24];
	int Map[15][25], starcnt;
	bool isSwitchOff;
	int Scheck, score, blockDown, random, PrintLc;

	GameManager() {}
	~GameManager() {}
	void Initialize();

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

	void MoveBall();

	void ClearVector();
	void MakeVector();
};

