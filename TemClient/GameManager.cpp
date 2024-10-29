#include "GameManager.h"

void GameManager::CrashExamin() {
	floatRECT blockrc;
	int crashStart, crashEnd, crashDir;
	bool doQ2block = true;

	if (GamePlay == SurvivalPlay) {
		// 오른쪽 벽
		if (ball.x + rd >= side * 17) {
			ball.x = side * 17 - rd;
			ball.vx = -ball.vx / 2;
			ball.ax = -7;
			return;
		}
		// 왼쪽벽
		else if (ball.x - rd <= side * 8) {
			ball.x = side * 8 + rd;
			ball.vx = -ball.vx / 2;
			ball.ax = 7;
			return;
		}
	}
	else {
		// 오른쪽 벽
		if (ball.x + rd >= 1516) {
			ball.x = 1516 - rd;
			ball.vx = -ball.vx / 2;
			ball.ax = -7;
			return;
		}
		// 왼쪽벽
		if (ball.x - rd <= 0) {
			ball.x = rd;
			ball.vx = -ball.vx / 2;
			ball.ax = 7;
			return;
		}
	}

	crash.clear();
	crashStart = ballrc.top >= 0 ? (int)(ballrc.top / 60) : 0;
	crashEnd = (int)(ballrc.bottom / 60);
	for (int y = crashStart; y <= crashEnd; y++) {
		for (int i = 0; i < block[y].size(); i++) {
			//충돌체크 안해도 되는 블럭일 경우 넘어감
			if (block[y][i].type == MvBkStopBk || (block[y][i].type == ElectricBk && isSwitchOff) || block[y][i].type == RectWHBk || block[y][i].type == CircleWHBk)
				continue;
			else if (block[y][i].type == OnceMvBk && block[y][i].ani != 255)
				continue;

			//충돌체크
			if (block[y][i].type == MoveBk)
				blockrc = { (float)block[y][i].x, (float)block[y][i].y, (float)(block[y][i].x + side), (float)(block[y][i].y + side) };
			else
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

	// 어디에 닿든 상관 없는 블럭
	switch (block[y][i].type) {
	case CircleBHBk:
	case RectBHBk: {
		temp = Search(block[y][i].type + 2); // 각각 RectWHBk, CircleWHBk 검색
		if (temp) {
			Scheck = telpo;
			ball.x = temp->x * side + side / 2;
			ball.y = temp->y * side + side / 2;
			ball.vx = 0;
			ball.vy = 40;
		}
		return;
	}
	case Item: {
		if (block[y][i].subtype < ShortTelpo) // 공 상태를 바꾸는 아이템일 경우
			ball.state = block[y][i].subtype;
		else
			ball.item = block[y][i].subtype;
		block[y].erase(block[y].begin() + i);
		for (int j = 0; j < crash.size(); j++) {
			if (y == crash[j].j && i < crash[j].i)
				crash[j].i -= 1;
		}
		return;
	}
	case LightBk: {
		if (block[y][i].subtype < 2) {
			animation.emplace_back(Block{ (int)ball.x - 90, (int)ball.y - 90, StageDeath, rand() % 4, 0 });
			Scheck = balldeath;
			if (GamePlay == StagePlay)
				GamePlay = StageDeath;
			else if (GamePlay == CustomPlay)
				GamePlay = CustomDeath;
			else if (GamePlay == SurvivalPlay)
				GamePlay = SurvivalDeath;
			return;
		}
		else break;
	}
	case Star: {
		if (ball.state == Normal) {
			Scheck = eatstar;
			animation.emplace_back(Block{ (block[y][i].x - 1) * side, (block[y][i].y - 1) * side, Star, rand() % 4, 0 });
			block[y].erase(block[y].begin() + i);
			for (int j = 0; j < crash.size(); j++) {
				if (y == crash[j].j && i < crash[j].i)
					crash[j].i -= 1;
			}

			if (GamePlay == SurvivalPlay)
				score += 10;
		}
		return;
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
			else if (GamePlay == SurvivalPlay)
				GamePlay = SurvivalDeath;
		}
		return;
	}
	}

	// 방향에 따라 달라지는 블럭일 경우
	switch (dir) {
	case dirUp: { // 블럭 위쪽에 충돌했을 경우
		switch (block[y][i].type) {
		case BreakBk: {
			CrashBasicTop(&block[y][i]);
			animation.emplace_back(Block{ (block[y][i].x - 1) * side, (block[y][i].y - 1) * side, BreakBk, rand() % 4, 0 });
			block[y].erase(block[y].begin() + i);
			for (int j = 0; j < crash.size(); j++) {
				if (y == crash[j].j && i < crash[j].i)
					crash[j].i -= 1;
			}
			return;
		}
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
		case OnceMvBk: {
			if (block[y][i].ani == 255) {
				CrashBasicTop(&block[y][i]);
				block[y][i].ani -= 30;
			}
			return;
		}
		case LauncherBk: {
			CrashBasicTop(&block[y][i]);
			MakeBullet(&block[y][i], 1);
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
	case dirRight: { // 블럭 오른쪽에 충돌했을 경우
		if (block[y][i].type == ClimbBK) {
			ball.x = block[y][i].x * side + side + rd;
			ball.vx = ball.ax = 0;
			ball.vy = 5.1; // 오른쪽 충돌
		}
		else
			CrashBasicRight(&block[y][i]);
		return;
	}
	case dirLeft: { // 블럭 왼쪽에 충돌했을 경우
		if (block[y][i].type == ClimbBK) {
			ball.x = block[y][i].x * side - rd;
			ball.vx = ball.ax = 0;
			ball.vy = 5;
		}
		else
			CrashBasicLeft(&block[y][i]);
		return;
	}
	case dirDown: { // 블럭 아래쪽에 충돌했을 경우
		if (block[y][i].type == ClimbBK && (block[y][i].subtype == 2 || block[y][i].subtype == 1)) {}
		else
			CrashBasicBottom(&block[y][i]);
		return;
	}
	default: // 충돌하지 않았을 경우
		return;
	}
}
int GameManager::BlockQuality(const Block* block) {
	switch (block->type) {
	case Item:
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

	//vy에 의해 충돌했는지
	tempballrc = { (float)ballrc->left, (float)ballrc->top - ball.remy * t, (float)ballrc->right, (float)ballrc->bottom - ball.remy * t };
	if (isCrashed(&tempballrc, blockrc) == 4) {
		if (ball.vy > 0)
			return dirUp;
		else
			return dirDown;
	}
	// vx에 의해 충돌했는지
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

	if (dbleft <= dbright && dbtop <= dbbottom) { // 충돌한 경우
		if (dbright - dbleft < dbbottom - dbtop) { // 좌우에서 충돌한 경우
			if (abs(ball.x - blockrc->right) <= abs(ball.x - blockrc->left)) // 공이 오른쪽에 있을 경우
				return dirRight;
			else // 공이 왼쪽에 있을 경우
				return dirLeft;
		}
		else { // 상하에서 충돌한 경우
			if (abs(ball.y - blockrc->top) <= abs(ball.y - blockrc->bottom)) // 공이 위쪽에 있을 경우
				return dirUp;
			else // 공이 아래쪽에 있을 경우
				return dirDown;
		}
	}
	else { // 충돌하지 않은 경우
		return 4;
	}
}

//검색
Block* GameManager::Search(const int type) {
	for (int y = 0; y < 15; y++) {
		for (int i = 0; i < block[y].size(); i++) {
			if (block[y][i].type == type) return &block[y][i];
		}
	}
	return 0;
}

// 기본블럭 위에 충돌한 경우
void GameManager::CrashBasicTop(const Block* block) {
	if (block->type != MusicBk && block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	if (block->type == MoveBk)
		ball.y = block->y - rd;
	else
		ball.y = block->y * side - rd;
	ball.vy = -40;
	ball.ax = 0;
	if (isLeftPressed) ball.vx = ball.vx < 0 ? -21 : ball.vx;
	else if (isRightPressed) ball.vx = ball.vx > 0 ? 21 : ball.vx;
	else ball.vx = 0;
}
//기본블럭 아래에 충돌한 경우
void GameManager::CrashBasicBottom(const Block* block) {
	Scheck = ballcrach;
	if (block->type == MoveBk)
		ball.y = block->y + side + rd;
	else
		ball.y = block->y * side + side + rd;
	ball.vy = -ball.vy;
}
//기본블럭 왼쪽에 충돌한 경우
void GameManager::CrashBasicLeft(const Block* block) {
	if (block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	if (block->type == MoveBk)
		ball.x = block->x - rd;
	else
		ball.x = block->x * side - rd;
	ball.vx = -ball.vx / 2;
	ball.ax = -7;
}
//기본블럭 오른쪽에 충돌한 경우
void GameManager::CrashBasicRight(const Block* block) {
	if (block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	if (block->type == MoveBk)
		ball.x = block->x + side + rd;
	else
		ball.x = block->x * side + side + rd;
	ball.vx = -ball.vx / 2;
	ball.ax = 7;
}

//공 이동
void GameManager::MoveBall() {
	SHORT leftKeyState, rightKeyState;

	// 눌려있는지 확인
	if (ball.vy != 5.1 && ball.vy != 5 && GamePlay != StageClear) {
		leftKeyState = GetAsyncKeyState(VK_LEFT);
		rightKeyState = GetAsyncKeyState(VK_RIGHT);
		isLeftPressed = (leftKeyState & 0x8000) != 0;
		isRightPressed = (rightKeyState & 0x8000) != 0;

		if (isLeftPressed && isRightPressed == false) { // 왼쪽만 눌렸을 경우
			if (abs(ball.vx) == 60) // 직진하고 있었을 경우
				ball.vx = 0;
			if (abs(ball.vx) != 50) // 대쉬하고있지 않을 때 기본
				ball.ax = -7;
		}
		else if (isRightPressed && isLeftPressed == false) { // 오른쪽만 눌렸을 경우
			if (abs(ball.vx) == 60) // 직진하고 있었을 경우
				ball.vx = 0;
			if (abs(ball.vx) != 50)
				ball.ax = 7;
		}
		else if (ball.vx && isLeftPressed == false && isRightPressed == false && abs(ball.vx) != 60) // 둘다 안눌렸을 경우
			ball.ax = -ball.vx / 4.3;
	}

	ball.x += ball.vx * t;
	ball.y += ball.vy * t;
	ball.remx = ball.vx;
	ball.remy = ball.vy;
	ball.vx += ball.ax * t;
	if (abs(ball.vx) != 60) ball.vy += g * t;
	//가속도 조절
	if ((ball.ax == 7 && ball.vx >= 21 || ball.ax == -7 && ball.vx <= -21) && abs(ball.vx) != 50) {
		ball.ax = 0;
		ball.vx = ball.vx > 0 ? 21 : -21;
	}
}

//산탄 생성, 이동, 충돌
void GameManager::MakeBullet(const Block* block, int BulletType) {
	switch (block->subtype)
	{
	case dirRight: {
		bullet.emplace_back(Block{ (block->x + 1) * side, block->y * side + 10, dirRight, BulletType, 0 });
		break;
	}
	case dirLeft: {
		bullet.emplace_back(Block{ (block->x - 1) * side + 20, block->y * side + 10, dirLeft, BulletType, 0 });
		break;
	}
	case dirDown: {
		bullet.emplace_back(Block{ block->x * side + 10, (block->y + 1) * side, dirDown, BulletType, 0 });
		break;
	}
	case dirUp: {
		bullet.emplace_back(Block{ block->x * side + 10, (block->y - 1) * side + 20, dirUp, BulletType, 0 });
		break;
	}
	}
}
void GameManager::MoveBullet() {
	for (int i = 0; i < bullet.size(); i++) {
		switch (bullet[i].type)
		{
		case dirRight: {
			bullet[i].x += 4;
			break;
		}
		case dirLeft: {
			bullet[i].x -= 4;
			break;
		}
		case dirDown: {
			bullet[i].y += 4;
			break;
		}
		case dirUp: {
			bullet[i].y -= 4;
			break;
		}
		}

		if (bullet[i].x + 42 <= 0 || bullet[i].x >= 1516 || bullet[i].y + 42 <= 0 || bullet[i].y >= 939) {
			bullet.erase(bullet.begin() + i);
		}
	}
}
void GameManager::CrashBullet() {
	floatRECT bulletrc, blockrc;

	for (int i = 0; i < bullet.size(); ++i) {
		bulletrc = { (float)bullet[i].x, (float)bullet[i].y, (float)bullet[i].x + 40, (float)bullet[i].y + 40 };

		// 공 & 산탄 충돌
		if (isCrashed(&ballrc, &bulletrc) != 4 && GamePlay != StageDeath && GamePlay != CustomDeath && GamePlay != SurvivalDeath) {
			animation.emplace_back(Block{ (int)ball.x - 90, (int)ball.y - 90, StageDeath, rand() % 4, 0 });
			Scheck = balldeath;
			if (GamePlay == StagePlay)
				GamePlay = StageDeath;
			else if (GamePlay == CustomPlay)
				GamePlay = CustomDeath;
			else if (GamePlay == SurvivalPlay)
				GamePlay = SurvivalDeath;
			return;
		}

		// 블럭 & 산탄 충돌
		for (int y = 0; y < 15; ++y) {
			for (int k = 0; k < block[y].size(); ++k) {
				if (block[y][k].type == MvBkStopBk || block[y][k].type == ElectricBk || block[y][k].type == RectWHBk || block[y][k].type == CircleWHBk)
					continue;

				if (block[y][k].type == MoveBk)
					blockrc = { (float)block[y][k].x, (float)block[y][k].y, (float)(block[y][k].x + side), (float)(block[y][k].y + side) };
				else
					blockrc = { (float)block[y][k].x * side, (float)block[y][k].y * side, (float)((block[y][k].x + 1) * side), (float)((block[y][k].y + 1) * side) };

				if (isCrashed(&bulletrc, &blockrc) != 4) {
					if (block[y][k].type == BreakBk && bullet[i].subtype) {
						animation.emplace_back(Block{ (block[y][k].x - 1) * side, (block[y][k].y - 1) * side, BreakBk, rand() % 4, 0 });
						block[y].erase(block[y].begin() + k);
					}
					bullet.erase(bullet.begin() + i);
				}
			}
		}
	}
}

//단칸이동블럭 이동 위치 검사, 이동
bool GameManager::OnceMvBkGo(const Block* b)
{
	switch (b->subtype)
	{
	case dirRight: {
		for (int i = 0; i < block[b->y].size(); ++i) {
			if (block[b->y][i].type == MoveBk) {
				if ((b->x + 1) * side < block[b->y][i].x + side && (b->x + 1) * side + side > block[b->y][i].x)
					return false;
			}
			else if (b->x + 1 == block[b->y][i].x && (block[b->y][i].type != ElectricBk && block[b->y][i].type != MvBkStopBk))
				return false;
		}
		break;
	}
	case dirLeft: {
		for (int i = 0; i < block[b->y].size(); ++i) {
			if (block[b->y][i].type == MoveBk) {
				if ((b->x - 1) * side < block[b->y][i].x + side && (b->x - 1) * side + side > block[b->y][i].x)
					return false;
			}
			else if (b->x - 1 == block[b->y][i].x && (block[b->y][i].type != ElectricBk && block[b->y][i].type != MvBkStopBk))
				return false;
		}
		break;
	}
	case dirDown: {
		for (int i = 0; i < block[b->y + 1].size(); ++i) {
			if (block[b->y + 1][i].type == MoveBk) {
				if (b->x * side < block[b->y + 1][i].x + side && b->x * side + side > block[b->y + 1][i].x)
					return false;
			}
			else if (b->x == block[b->y + 1][i].x && (block[b->y + 1][i].type != ElectricBk && block[b->y + 1][i].type != MvBkStopBk))
				return false;
		}
		break;
	}
	case dirUp: {
		for (int i = 0; i < block[b->y - 1].size(); ++i) {
			if (block[b->y - 1][i].type == MoveBk) {
				if (b->x * side < block[b->y - 1][i].x + side && b->x * side + side > block[b->y - 1][i].x)
					return false;
			}
			else if (b->x == block[b->y - 1][i].x && (block[b->y - 1][i].type != ElectricBk && block[b->y - 1][i].type != MvBkStopBk))
				return false;
		}
		break;
	}
	default:
		break;
	}
	return true;
}
void GameManager::MoveOnceMvBk(int y, int i) {
	switch (block[y][i].subtype)
	{
	case dirRight:
		if (block[y][i].x < 24 && OnceMvBkGo(&block[y][i])) {
			block[y][i].x++;
		}
		break;
	case dirLeft:
		if (block[y][i].x > 0 && OnceMvBkGo(&block[y][i])) {
			block[y][i].x--;
		}
		break;
	case dirUp:
		if (y > 0 && OnceMvBkGo(&block[y][i])) {
			block[y - 1].emplace_back(block[y][i]);
			block[y - 1].back().y -= 1;
			block[y].erase(block[y].begin() + i);
		}
		break;
	case dirDown:
		if (y < 14 && OnceMvBkGo(&block[y][i])) {
			block[y + 1].emplace_back(block[y][i]);
			block[y + 1].back().y += 1;
			block[y].erase(block[y].begin() + i);
		}
		break;
	}
}

// 이동블럭 이동, 방향전환
bool GameManager::MoveMoveBk(Block* b)
{
	if (b->x < 0 || b->x + side > 1516) {
		TurnMoveBk(b);
	}

	for (int k = 0; k < block[b->y / side].size(); ++k) {
		if (block[b->y / side][k].type == MoveBk && block[b->y / side][k].ani == b->ani) continue;//여기 같은 그룹의 이동블럭 충돌체크 안하게 수정했고

		if (b->x + b->subtype < ((block[b->y / side][k].x + 1) * side) && b->x + side + b->subtype >(block[b->y / side][k].x * side)) {//이거 이거만 써도 충돌체크 됨
			TurnMoveBk(b);//반복되는부분 함수로 뺐습니다 추가로 함수명도 수정함
			break;
		}
	}
	return false;
}
void GameManager::TurnMoveBk(Block* b)
{
	if (b->subtype < 0) {
		b->x -= b->subtype * 2;
	}
	for (int k = 0; k < block[b->y / side].size(); ++k) {
		if (block[b->y / side][k].type != MoveBk || block[b->y / side][k].ani != b->ani) continue;
		else {
			block[b->y / side][k].subtype *= -1;
		}
	}
}

// 아이템 사용
void GameManager::UseItem() {
	int tempx, crashStart, crashEnd, crashReturn;
	floatRECT ballrc, blockrc;

	switch (ball.item) {
	case ShortTelpo:
		Scheck = telpo;
		if (ball.vx >= 0) ballrc = { (ball.x + rd), (ball.y - rd), (ball.x + side * 3 + rd), (ball.y + rd) };
		else ballrc = { (ball.x - rd), (ball.y - rd), (ball.x - side * 3 + rd), (ball.y + rd) };

		ball.item = Normal;
		crashStart = ballrc.top >= 0 ? (int)(ballrc.top / 60) : 0;
		crashEnd = (int)(ballrc.bottom / 60);

		if (ball.vx >= 0) {
			for (int y = crashStart; y <= crashEnd; ++y) {
				for (int i = 0; i < block[y].size(); i++) {
					if (block[y][i].type == MoveBk)
						blockrc = { (float)block[y][i].x, (float)block[y][i].y, (float)(block[y][i].x + side), (float)(block[y][i].y + side) };
					else
						blockrc = { (float)block[y][i].x * side, (float)block[y][i].y * side, (float)((block[y][i].x + 1) * side), (float)((block[y][i].y + 1) * side) };
					if (isCrashed(&ballrc, &blockrc) != 4 && i != block[y].size() - 1 && block[y][i + 1].x - block[y][i].x == 1) {
						ball.x = blockrc.left - rd;
						return;
					}
				}
			}
			ball.x += side * 3;
		}

		else {
			for (int y = crashStart; y <= crashEnd; ++y) {
				for (int i = block[y].size() - 1; i >= 0; i--) {
					if (block[y][i].type == MoveBk)
						blockrc = { (float)block[y][i].x, (float)block[y][i].y, (float)(block[y][i].x + side), (float)(block[y][i].y + side) };
					else
						blockrc = { (float)block[y][i].x * side, (float)block[y][i].y * side, (float)((block[y][i].x + 1) * side), (float)((block[y][i].y + 1) * side) };
					if (isCrashed(&ballrc, &blockrc) != 4 && i > 0 && block[y][i].x - block[y][i - 1].x == 1) {
						ball.x = blockrc.right + rd;
						return;
					}
				}
			}
			ball.x -= side * 3;
		}
		break;
	case Dash:
		ball.item = Normal;
		ball.vy = -10;
		ball.vx = ball.vx > 0 ? 50 : -50;
		ball.ax = 0;
		break;
	case HighJmp:
		ball.item = Normal;
		ball.vy = -50;
		ball.vx = ball.ax = 0;
		break;
	}
}

// 맵툴
void GameManager::MakeBlockList() {
	list[0].type = Star;
	for (int i = 0; i < 5; i++) {
		list[i + 1] = { 0, 0, Item, i, 0 };
	}
	list[6].type = JumpBk;
	list[7].type = RStraightBk;
	list[8].type = LStraightBk;
	list[9].type = RectBHBk;
	list[10].type = RectWHBk;
	list[11].type = CircleBHBk;
	list[12].type = CircleWHBk;
	list[13] = { 0, 0, SwitchBk, 0, 0 };
	list[14] = { 0, 0, SwitchBk, 1, 0 };
	list[15] = { 0, 0, ElectricBk, 0, 0 };
	list[16].type = MvBkStopBk;
	list[17] = { 0, 0, MoveBk, 1, 0 };
	list[18].type = BreakBk;
	list[19].type = ClimbBK;
	list[20].type = MusicBk;
	for (int i = 0; i < 11; i++) {
		list[i + 21] = { 0, 0, BasicBk, i, 0 };
	}
	for (int i = 0; i < 4; i++) {
		list[i + 32] = { 0, 0, OnceMvBk, i, 255 };
		list[i + 36] = { 0, 0, LauncherBk, i, 0 };
	}
	list[40] = { 0, 0, LightBk, 0, 0 };
	list[41] = { 0, 0, LightBk, 2, 0 };
	list[42] = { 0, 0, LightBk, 4, 0 };
}

// 맵 배열에서 벡터로 변환 (공 좌표, 스위치 상태는 따로 받기)
void GameManager::MakeVector() {
	ClearVector();
	Block temp;
	int groupcnt = 1; // 이동블럭 그룹
	bool Continuous = false;
	starcnt = 0;
	ball.item = ball.state = 0;

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
	else if (GamePlay == SurvivalReady) {
		random = 12;
		for (int a = 0; a < 3; ++a) {
			if (random <= ((SVMAPCNT - 1) / 2)) // 0 ~ 11이면
				random = rand() % (SVMAPCNT / 2) + (SVMAPCNT / 2);
			else // 12 ~ 23이면
				random = rand() % (SVMAPCNT / 2);

			if (a == 2) // 첫번째 맵
				random = 0;

			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 9; j++) {
					if (SurvivalMap[random][i][j]) { // 블럭일 경우
						temp.x = SurvivalMap[random][i][j] - 1 == 17 ? (j + 8) * side : j + 8;
						temp.y = SurvivalMap[random][i][j] - 1 == 17 ? (i + (a * 4)) * side : i + (a * 4);
						temp.type = list[SurvivalMap[random][i][j] - 1].type;
						if (SurvivalMap[random][i][j] - 1 == 13 || SurvivalMap[random][i][j] - 1 == 14 || SurvivalMap[random][i][j] - 1 == 15)
							temp.subtype = isSwitchOff;
						else
							temp.subtype = list[SurvivalMap[random][i][j] - 1].subtype;

						// 이동블럭 관리
						if (Continuous) {
							if (SurvivalMap[random][i][j] - 1 == 17 && SurvivalMap[random][i][j - 1] - 1 == 17)
								temp.ani = groupcnt;
							else {
								Continuous = false;
								groupcnt++;
							}
						}
						if (SurvivalMap[random][i][j] - 1 == 17) {
							temp.ani = groupcnt;
							Continuous = true;
						}
						if (SurvivalMap[random][i][j] - 1 != 17)
							temp.ani = list[SurvivalMap[random][i][j] - 1].ani;

						// 끈끈이 그룹화
						if (SurvivalMap[random][i][j] == 20) {
							// 맵 가장 위이거나, 맵 가장 아래가 아니고 블럭 위가 끈끈이가 아니고 아래가 끈끈이면 1번
							if (i == 0 || i < 14 && SurvivalMap[random][i - 1][j] != 20 && SurvivalMap[random][i + i][j] == 20)
								temp.subtype = 1;
							// 맵 가장 위나 아래가 아니고 블럭 위와 아래가 끈끈이면 2번
							else if (i > 0 && i < 14 && SurvivalMap[random][i - 1][j] == 20 && SurvivalMap[random][i + 1][j] == 20)
								temp.subtype = 2;
							// 맵 가장 아래이거나, 맵 가장 위가 아니고 블럭 위가 끈끈이고 아래가 끈끈이가 아니면 2번
							else if (i == 14 || i > 0 && SurvivalMap[random][i - 1][j] == 20 && SurvivalMap[random][i + 1][j] != 20)
								temp.subtype = 3;
							else
								temp.subtype = 4;
						}
						block[i + (a * 4)].emplace_back(temp);
					}
				}
			}
		}
	}
}
void GameManager::ClearVector() { // 걍 다 초기화하게함
	bullet.clear();
	animation.clear();
	for (int i = 0; i < 15; i++) {
		block[i].clear();
	}
}
void GameManager::MakeReadyVector() {
	ClearReadyVector();
	Block temp;
	int groupcnt = 1; // 이동블럭 그룹
	bool Continuous = false;
	if (random <= ((SVMAPCNT - 1) / 2))
		random = rand() % ((SVMAPCNT - 1) / 2) + ((SVMAPCNT - 1) / 2) + 1;
	else
		random = rand() % ((SVMAPCNT - 1) / 2);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 9; j++) {
			if (SurvivalMap[random][i][j]) { // 블럭일 경우
				temp.x = SurvivalMap[random][i][j] - 1 == 17 ? (j + 8) * side : j + 8;
				temp.y = 0;
				temp.type = list[SurvivalMap[random][i][j] - 1].type;
				if (SurvivalMap[random][i][j] - 1 == 13 || SurvivalMap[random][i][j] - 1 == 14 || SurvivalMap[random][i][j] - 1 == 15)
					temp.subtype = isSwitchOff;
				else
					temp.subtype = list[SurvivalMap[random][i][j] - 1].subtype;

				// 이동블럭 관리
				if (Continuous) {
					if (SurvivalMap[random][i][j] - 1 == 17 && SurvivalMap[random][i][j - 1] - 1 == 17) temp.ani = groupcnt;
					else {
						Continuous = false;
						groupcnt++;
					}
				}
				if (SurvivalMap[random][i][j] - 1 == 17) {
					temp.ani = groupcnt;
					Continuous = true;
				}
				if (SurvivalMap[random][i][j] - 1 != 17)
					temp.ani = list[SurvivalMap[random][i][j] - 1].ani;

				// 끈끈이 그룹화
				if (SurvivalMap[random][i][j] == 20) {
					// 맵 가장 위이거나, 맵 가장 아래가 아니고 블럭 위가 끈끈이가 아니고 아래가 끈끈이면 1번
					if (i == 0 || i < 14 && SurvivalMap[random][i - 1][j] != 20 && SurvivalMap[random][i + i][j] == 20)
						temp.subtype = 1;
					// 맵 가장 위나 아래가 아니고 블럭 위와 아래가 끈끈이면 2번
					else if (i > 0 && i < 14 && SurvivalMap[random][i - 1][j] == 20 && SurvivalMap[random][i + 1][j] == 20)
						temp.subtype = 2;
					// 맵 가장 아래이거나, 맵 가장 위가 아니고 블럭 위가 끈끈이고 아래가 끈끈이가 아니면 2번
					else if (i == 14 || i > 0 && SurvivalMap[random][i - 1][j] == 20 && SurvivalMap[random][i + 1][j] != 20)
						temp.subtype = 3;
					else
						temp.subtype = 4;
				}
				Readyblock[i].emplace_back(temp);
			}
		}
	}
}
void GameManager::ClearReadyVector() {
	for (int i = 0; i < 4; i++) {
		Readyblock[i].clear();
	}
}