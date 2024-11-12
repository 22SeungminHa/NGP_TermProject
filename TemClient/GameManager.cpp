#include "GameManager.h"

void GameManager::Initialize() {
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
	for (int i = 0; i < 14; i++) {
		list[i + 13] = { 0, 0, BasicBk, i, 0 };
	}
}

void GameManager::CrashExamin() {
	floatRECT blockrc;
	int crashStart, crashEnd, crashDir;
	bool doQ2block = true;

	// ������ ��
	if (ball.x + rd >= 1516) {
		ball.x = 1516 - rd;
		ball.vx = -ball.vx / 2;
		ball.ax = -7;
		return;
	}
	// ���ʺ�
	if (ball.x - rd <= 0) {
		ball.x = rd;
		ball.vx = -ball.vx / 2;
		ball.ax = 7;
		return;
	}

	crash.clear();
	crashStart = ballrc.top >= 0 ? (int)(ballrc.top / 60) : 0;
	crashEnd = (int)(ballrc.bottom / 60);
	for (int y = crashStart; y <= crashEnd; y++) {
		for (int i = 0; i < block[y].size(); i++) {
			//�浹üũ ���ص� �Ǵ� ������ ��� �Ѿ
			if ((block[y][i].type == ElectricBk && isSwitchOff) || block[y][i].type == RectWHBk || block[y][i].type == CircleWHBk)
				continue;

			//�浹üũ
			blockrc = { (float)block[y][i].x * side, (float)block[y][i].y * side, (float)((block[y][i].x + 1) * side), (float)((block[y][i].y + 1) * side) };
			if (isCrashed(&ballrc, &blockrc) != 4) {
				crashDir = MyIntersectRect(&ballrc, &blockrc);
				crash.emplace_back(CrashedBk{ crashDir, i, y, blockrc.left, blockrc.top, BlockQuality(&block[y][i]) });
			}
		}
	}

	if (crash.size()) {
		for (int i = 0; i < crash.size() - 1; i++) {
			for (int j = 0; j < crash.size() - i - 1; j++) {
				if (crash[j].quality > crash[j + 1].quality)
					std::swap(crash[j], crash[j + 1]);
			}
		}
		if (crash[0].quality != 1)
			doQ2block = false;

		for (int i = 0; i < crash.size(); i++) {
			if (crash[i].quality == 1 && doQ2block == false);
			else {
				Crash(crash[i].dir, crash[i].i, crash[i].j);
			}
		}
	}
}
void GameManager::Crash(int dir, int i, int y) {
	Block* temp;
	floatRECT blockrc;

	// ��� ��� ��� ���� ����
	switch (block[y][i].type) {
	case CircleBHBk:
	case RectBHBk: {
		temp = Search(block[y][i].type + 2); // ���� RectWHBk, CircleWHBk �˻�
		if (temp) {
			Scheck = telpo;
			ball.x = temp->x * side + side / 2;
			ball.y = temp->y * side + side / 2;
			ball.vx = 0;
			ball.vy = 40;
		}
		return;
	}
	case Star: {
		Scheck = eatstar;
		animation.emplace_back(Block{ (block[y][i].x - 1) * side, (block[y][i].y - 1) * side, Star, rand() % 4, 0 });
		block[y].erase(block[y].begin() + i);
		for (int j = 0; j < crash.size(); j++) {
			if (y == crash[j].j && i < crash[j].i)
				crash[j].i -= 1;
		}
	}
	case ElectricBk: {
		blockrc = { (float)block[y][i].x * side + 20, (float)block[y][i].y * side + 5, (float)(block[y][i].x + 1) * side - 20, (float)(block[y][i].y + 1) * side - 5 };
		if (isCrashed(&ballrc, &blockrc) != 4) {
			animation.emplace_back(Block{ (int)ball.x - 90, (int)ball.y - 90, StageDeath, rand() % 4, 0 });
			Scheck = balldeath;
			if (GamePlay == StagePlay)
				GamePlay = StageDeath;
			else if (GamePlay == CustomPlay)
				GamePlay = CustomDeath;
		}
		return;
	}
	}

	// ���⿡ ���� �޶����� ������ ���
	switch (dir) {
	case dirUp: { // ���� ���ʿ� �浹���� ���
		switch (block[y][i].type) {
		case JumpBk: {
			Scheck = telpo;
			ball.y = block[y][i].y * side - rd;
			ball.vy = -65;
			ball.ax = 0;
			if (isLeftPressed || isRightPressed) ball.vx = ball.vx > 0 ? 21 : -21;
			else ball.vx = 0;
			return;
		}
		case RStraightBk:
		case LStraightBk: {
			Scheck = telpo;
			ball.x = block[y][i].type == RStraightBk ? (block[y][i].x + 1) * side + rd : block[y][i].x * side - rd;
			ball.y = block[y][i].y * side + side / 2;
			ball.vy = ball.ax = 0;
			ball.vx = block[y][i].type == RStraightBk ? 60 : -60;
			return;
		}
		case SwitchBk: {
			CrashBasicTop(&block[y][i]);
			isSwitchOff = 1 - isSwitchOff;
			return;
		}
		case MusicBk: {
			CrashBasicTop(&block[y][i]);
			Scheck = music;
			return;
		}
		default: {
			if (block[y][i].type == ClimbBK && (block[y][i].subtype == 2 || block[y][i].subtype == 3)) {}
			else
				CrashBasicTop(&block[y][i]);
			return;
		}
		}
		break;
	}
	case dirRight: { // ���� �����ʿ� �浹���� ���
		if (block[y][i].type == ClimbBK) {
			ball.x = block[y][i].x * side + side + rd;
			ball.vx = ball.ax = 0;
			ball.vy = 5.1; // ������ �浹
		}
		else
			CrashBasicRight(&block[y][i]);
		return;
	}
	case dirLeft: { // ���� ���ʿ� �浹���� ���
		if (block[y][i].type == ClimbBK) {
			ball.x = block[y][i].x * side - rd;
			ball.vx = ball.ax = 0;
			ball.vy = 5;
		}
		else
			CrashBasicLeft(&block[y][i]);
		return;
	}
	case dirDown: { // ���� �Ʒ��ʿ� �浹���� ���
		if (block[y][i].type == ClimbBK && (block[y][i].subtype == 2 || block[y][i].subtype == 1)) {}
		else
			CrashBasicBottom(&block[y][i]);
		return;
	}
	default: // �浹���� �ʾ��� ���
		return;
	}
}
int GameManager::BlockQuality(const Block* block) {
	switch (block->type) {
	case Star:
		return 2;
	case ElectricBk:
	case RectBHBk:
	case CircleBHBk:
	case JumpBk:
	case RStraightBk:
	case LStraightBk:
		return 1;
	default:
		return 0;
	}
}
int GameManager::MyIntersectRect(const floatRECT* ballrc, const floatRECT* blockrc) {
	floatRECT tempballrc;

	//vy�� ���� �浹�ߴ���
	tempballrc = { (float)ballrc->left, (float)ballrc->top - ball.remy * t, (float)ballrc->right, (float)ballrc->bottom - ball.remy * t };
	if (isCrashed(&tempballrc, blockrc) == 4) {
		if (ball.vy > 0)
			return dirUp;
		else
			return dirDown;
	}
	// vx�� ���� �浹�ߴ���
	tempballrc = { (float)ballrc->left - ball.remx * t, (float)ballrc->top, (float)ballrc->right - ball.remx * t, (float)ballrc->bottom - ball.remy * t };
	if (isCrashed(&tempballrc, blockrc) == 4) {
		if (ball.vx < 0)
			return dirRight;
		else
			return dirLeft;
	}
	return isCrashed(ballrc, blockrc);
}
int GameManager::isCrashed(const floatRECT* ballrc, const floatRECT* blockrc) {
	float dbleft = max(ballrc->left, blockrc->left);
	float dbtop = max(ballrc->top, blockrc->top);
	float dbright = min(ballrc->right, blockrc->right);
	float dbbottom = min(ballrc->bottom, blockrc->bottom);

	if (dbleft <= dbright && dbtop <= dbbottom) { // �浹�� ���
		if (dbright - dbleft < dbbottom - dbtop) { // �¿쿡�� �浹�� ���
			if (abs(ball.x - blockrc->right) <= abs(ball.x - blockrc->left)) // ���� �����ʿ� ���� ���
				return dirRight;
			else // ���� ���ʿ� ���� ���
				return dirLeft;
		}
		else { // ���Ͽ��� �浹�� ���
			if (abs(ball.y - blockrc->top) <= abs(ball.y - blockrc->bottom)) // ���� ���ʿ� ���� ���
				return dirUp;
			else // ���� �Ʒ��ʿ� ���� ���
				return dirDown;
		}
	}
	else { // �浹���� ���� ���
		return 4;
	}
}

//�˻�
Block* GameManager::Search(const int type) {
	for (int y = 0; y < 15; y++) {
		for (int i = 0; i < block[y].size(); i++) {
			if (block[y][i].type == type) return &block[y][i];
		}
	}
	return 0;
}

// �⺻���� ���� �浹�� ���
void GameManager::CrashBasicTop(const Block* block) {
	if (block->type != MusicBk && block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	ball.y = block->y * side - rd;
	ball.vy = -40;
	ball.ax = 0;
	if (isLeftPressed) ball.vx = ball.vx < 0 ? -21 : ball.vx;
	else if (isRightPressed) ball.vx = ball.vx > 0 ? 21 : ball.vx;
	else ball.vx = 0;
}
//�⺻���� �Ʒ��� �浹�� ���
void GameManager::CrashBasicBottom(const Block* block) {
	Scheck = ballcrach;
	ball.y = block->y * side + side + rd;
	ball.vy = -ball.vy;
}
//�⺻���� ���ʿ� �浹�� ���
void GameManager::CrashBasicLeft(const Block* block) {
	if (block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	ball.x = block->x * side - rd;
	ball.vx = -ball.vx / 2;
	ball.ax = -7;
}
//�⺻���� �����ʿ� �浹�� ���
void GameManager::CrashBasicRight(const Block* block) {
	if (block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	ball.x = block->x * side + side + rd;
	ball.vx = -ball.vx / 2;
	ball.ax = 7;
}

//�� �̵�
void GameManager::MoveBall() {
	SHORT leftKeyState, rightKeyState;

	// �����ִ��� Ȯ��
	if (ball.vy != 5.1 && ball.vy != 5 && GamePlay != StageClear) {
		leftKeyState = GetAsyncKeyState(VK_LEFT);
		rightKeyState = GetAsyncKeyState(VK_RIGHT);
		isLeftPressed = (leftKeyState & 0x8000) != 0;
		isRightPressed = (rightKeyState & 0x8000) != 0;

		if (isLeftPressed && isRightPressed == false) { // ���ʸ� ������ ���
			if (abs(ball.vx) == 60) // �����ϰ� �־��� ���
				ball.vx = 0;
			if (abs(ball.vx) != 50) // �뽬�ϰ����� ���� �� �⺻
				ball.ax = -7;
		}
		else if (isRightPressed && isLeftPressed == false) { // �����ʸ� ������ ���
			if (abs(ball.vx) == 60) // �����ϰ� �־��� ���
				ball.vx = 0;
			if (abs(ball.vx) != 50)
				ball.ax = 7;
		}
		else if (ball.vx && isLeftPressed == false && isRightPressed == false && abs(ball.vx) != 60) // �Ѵ� �ȴ����� ���
			ball.ax = -ball.vx / 4.3;
	}

	ball.x += ball.vx * t;
	ball.y += ball.vy * t;
	ball.remx = ball.vx;
	ball.remy = ball.vy;
	ball.vx += ball.ax * t;
	if (abs(ball.vx) != 60) ball.vy += g * t;
	//���ӵ� ����
	if ((ball.ax == 7 && ball.vx >= 21 || ball.ax == -7 && ball.vx <= -21) && abs(ball.vx) != 50) {
		ball.ax = 0;
		ball.vx = ball.vx > 0 ? 21 : -21;
	}
}

// �� �迭���� ���ͷ� ��ȯ (�� ��ǥ, ����ġ ���´� ���� �ޱ�)
void GameManager::MakeVector() {
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
void GameManager::ClearVector() { // �� �� �ʱ�ȭ�ϰ���
	animation.clear();
	for (int i = 0; i < 15; i++) {
		block[i].clear();
	}
}