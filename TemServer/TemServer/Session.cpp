#include"stdafx.h"
#include "Session.h"
#include "ServerManager.h"

DWORD Session::Do_Recv(LPVOID arg)
{
	char buf[BUFSIZE + 1] = { 0 };

	while (true) {
		// 데이터 수신
		int receivedBytes = recv(sock, buf, sizeof(buf), 0);
		if (receivedBytes <= 0) {
			err_display("recv()");
			serverManager->Disconnect(id);
			return 1;
		}
		cout << "receivedBytes : " << receivedBytes << endl;

		int remain_data = receivedBytes + recv_remain; // 총 데이터 크기
		char* p = buf; // 패킷 처리 시작 위치

		// 패킷 처리 루프
		while (remain_data > 0) {
			unsigned short* byte = reinterpret_cast<unsigned short*>(p); // 패킷 크기 읽기
			int packet_size = *byte;

			// 패킷 크기 검증
			if (packet_size > BUFSIZE * 2 || packet_size < sizeof(WORD)) {
				std::cerr << "[Do_Recv()] Invalid packet size: " << packet_size << std::endl;
				recv_remain = 0;
				remain_data = 0;
				break;
			}

			// 패킷 처리 가능 여부 확인
			if (packet_size <= remain_data) {
				PACKET* receivedPacket = reinterpret_cast<PACKET*>(p);
				serverManager->ProcessPacket(id, reinterpret_cast<char*>(receivedPacket));

				p += packet_size;       // 다음 패킷으로 이동
				remain_data -= packet_size;
			}
			else {
				break; // 패킷 데이터가 부족함
			}
		}

		// 남은 데이터 보관
		recv_remain = remain_data;
		if (remain_data > 0) {
			// 버퍼 크기를 초과하는 데이터를 저장하려면 경고 출력
			if (remain_data > sizeof(save_buf)) {
				std::cerr << "[Do_Recv()] Remaining data exceeds buffer size, data may be lost!" << std::endl;
				remain_data = sizeof(save_buf); // 데이터 크기를 버퍼 크기로 제한
			}

			memmove(save_buf, p, remain_data); // 남은 데이터를 save_buf로 이동
		}
	}

	return 0;
}

void Session::AddPacketToQueue(std::shared_ptr<PACKET> packet)
{
    if (!packet) {
        std::cerr << "[AddPacket()] Invalid packet received." << std::endl;
        return;
    }

    // 큐에 패킷을 추가
    serverManager->sendPacketQ.push(packet);
    //std::cout << "Packet" << (int)packet->packetID << " added to the send queue." << std::endl;
}

void Session::Send_login_info_packet(Session* client)
{
	auto p = std::make_shared<SC_LOGIN_INFO_PACKET>(id);
	p->c_id = client->id;

	cout << "Send_login_info_packet 완료     " << id << ">>" << client->id << endl;

	AddPacketToQueue(p);
}

void Session::Send_logout_packet(Session* client)
{
	auto p = std::make_shared<SC_LOGOUT_PACKET>(id);

	p->c_id = client->id;

	AddPacketToQueue(p);
}

void Session::Send_edit_map_packet(Session* client)
{
	auto p = std::make_shared<SC_EDIT_MAP_PACKET>(id);

	p->block = isSwitchOff;

	AddPacketToQueue(p);
}

void Session::Send_load_map_packet(Session* client)
{
	// SC_LOGIN_INFO_PACKET 객체 생성
	auto p = std::make_shared<SC_LOAD_MAP_PACKET>(id);

	int cnt = 0;
	for (int y = 0; y < 15; ++y) {
		for (int x = 0; x < 25; ++x) {
			p->map[cnt++] = Map[y][x];
		}
	}

	// 패킷을 큐에 추가
	AddPacketToQueue(p);
}
    


// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------

void Session::Initialize() {
	ball = { 30, 12.5, 0, 0, 0 };
	isLeftPressed = false, isRightPressed = false;
	GamePlay = StageDeath;
	starcnt = 0;
	isSwitchOff = false;
	Scheck = 0, score = 0;

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
	for (int i = 0; i < 14; i++) {
		list[i + 13] = { 0, 0, BasicBk, i, 0 };
	}
}

void Session::CrashExamin() {
	floatRECT blockrc;
	int crashStart, crashEnd, crashDir;
	bool doQ2block = true;

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

	crash.clear();
	crashStart = ballrc.top >= 0 ? (int)(ballrc.top / 60) : 0;
	crashEnd = (int)(ballrc.bottom / 60);
	for (int y = crashStart; y <= crashEnd; y++) {
		for (int i = 0; i < block[y].size(); i++) {
			//충돌체크 안해도 되는 블럭일 경우 넘어감
			if ((block[y][i].type == ElectricBk && isSwitchOff) || block[y][i].type == RectWHBk || block[y][i].type == CircleWHBk)
				continue;

			//충돌체크
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
void Session::Crash(int dir, int i, int y) {
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

	// 방향에 따라 달라지는 블럭일 경우
	switch (dir) {
	case dirUp: { // 블럭 위쪽에 충돌했을 경우
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
		case ClimbBK: {
			if (!(block[y][i].subtype == 2 || block[y][i].subtype == 3)) {
				CrashBasicTop(&block[y][i]);
			}
			return;
		}
		default: {
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
int Session::BlockQuality(const Block* block) {
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
int Session::MyIntersectRect(const floatRECT* ballrc, const floatRECT* blockrc) {
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
int Session::isCrashed(const floatRECT* ballrc, const floatRECT* blockrc) {
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
Block* Session::Search(const int type) {
	for (int y = 0; y < 15; y++) {
		for (int i = 0; i < block[y].size(); i++) {
			if (block[y][i].type == type) return &block[y][i];
		}
	}
	return 0;
}

// 기본블럭 위에 충돌한 경우
void Session::CrashBasicTop(const Block* block) {
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
//기본블럭 아래에 충돌한 경우
void Session::CrashBasicBottom(const Block* block) {
	Scheck = ballcrach;
	ball.y = block->y * side + side + rd;
	ball.vy = -ball.vy;
}
//기본블럭 왼쪽에 충돌한 경우
void Session::CrashBasicLeft(const Block* block) {
	if (block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	ball.x = block->x * side - rd;
	ball.vx = -ball.vx / 2;
	ball.ax = -7;
}
//기본블럭 오른쪽에 충돌한 경우
void Session::CrashBasicRight(const Block* block) {
	if (block->type != JumpBk && block->type != SwitchBk) {
		Scheck = ballcrach;
	}
	ball.x = block->x * side + side + rd;
	ball.vx = -ball.vx / 2;
	ball.ax = 7;
}

//공 이동
void Session::MoveBall() {
	// 눌려있는지 확인
	if (ball.vy != 5.1 && ball.vy != 5 && GamePlay != StageClear) {
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

// 맵 배열에서 벡터로 변환 (공 좌표, 스위치 상태는 따로 받기)
void Session::MakeVector() {
	ClearVector();
	Block temp;
	starcnt = 0;

	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 25; j++) {
			if (Map[i][j]) { // 블럭일 경우
				// 별
				if (Map[i][j] == 1)
					starcnt++;

				temp.x = j;
				temp.y = i;
				temp.type = list[Map[i][j] - 1].type;
				if (Map[i][j] - 1 == 10) // 전기 관련 블럭
					temp.subtype = isSwitchOff;
				else
					temp.subtype = list[Map[i][j] - 1].subtype;
				temp.ani = list[Map[i][j] - 1].ani;

				// 끈끈이 그룹화
				if (Map[i][j] == 12) {
					// 맵 가장 위이거나, 맵 가장 아래가 아니고 블럭 위가 끈끈이가 아니고 아래가 끈끈이면 1번
					if (i == 0 || i < 14 && Map[i - 1][j] != 12 && Map[i + 1][j] == 12)
						temp.subtype = 1;
					// 맵 가장 위나 아래가 아니고 블럭 위와 아래가 끈끈이면 2번
					else if (i > 0 && i < 14 && Map[i - 1][j] == 12 && Map[i + 1][j] == 12)
						temp.subtype = 2;
					// 맵 가장 아래이거나, 맵 가장 위가 아니고 블럭 위가 끈끈이고 아래가 끈끈이가 아니면 2번
					else if (i == 14 || i > 0 && Map[i - 1][j] == 12 && Map[i + 1][j] != 12)
						temp.subtype = 3;
					else
						temp.subtype = 4;
				}
				block[i].emplace_back(temp);
			}
		}
	}
}
void Session::ClearVector() { // 걍 다 초기화하게함
	animation.clear();
	for (int i = 0; i < 15; i++) {
		block[i].clear();
	}
}

bool Session::CrashBottom() {
	if (ball.y + rd >= 939) {
		animation.emplace_back(Block{ (int)ball.x - 90, (int)ball.y - 90, CustomDeath, rand() % 4, 0 });
		Scheck = balldeath;
		if (GamePlay == StagePlay)
			GamePlay = StageDeath;
		else if (GamePlay == CustomPlay)
			GamePlay = CustomDeath;
		return true;
	}
	return false;
}