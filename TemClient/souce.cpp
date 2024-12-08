#include "client_pch.h"
#include "ClientManager.h"
#include"InputManager.h"
#include"Timer.h"

// 최대 vx = 21
// 최대 vy = 40
// ax = 7
// x축 최대 이동거리 = 180
// x축 최소 이동거리 = 90
// 좌우직진블럭 vx = 60
// 상승블럭 vy = -65
// 끈적이 블럭 좌 vy = 5 우 vy = 5.1

//전역 변수
ClientManager game;
HANDLE hThreadForSend;
HANDLE hThreadForReceive;

HWND hEdit;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Trip of a Ball";

OPENFILENAME OFN;
TCHAR filter[] = L"Every File(*.*)\0*.*\0Text File\0*.txt;*.doc\0";
TCHAR lpstrFile[100], str[20];
bool drag = false;
POINT BallStartLC{};
int selection = 0, electictimer = 0, bestscore = 0;

FMOD::System* ssystem;
FMOD::Sound* ballCrach_Sound, * Telpo_Sound, * EatStar_Sound, * ballDeath_Sound, * Click_Sound, * GameClear_Sound, * MusicBk_Sound;
FMOD::Channel* channel = 0;
FMOD_RESULT result;
void* extradriverdata = 0;


std::queue<pair<KEY_TYPE, KEY_STATE>> keyEventQueue{};
std::queue<KEY_TYPE> mouseEventQueue{};

#pragma region Images
CImage imgBall, imgBlock, imgSwitchBk, imgElectricBk,
imgStartScreen, imgStageScreen, imgStopScreen, imgClearScreen, imgPlayScreen, imgMaptoolScreen,
imgHomeButton, imgResetButton, imgLoadButton, imgSaveButton, imgEraseButton, imgPlayButton,
imgBlockList, imgOutline, imgWaiting, imgMapList,
imgStarAni, imgDeadAni;
#pragma endregion


void LoadResources();
void Update();
void Render();

void ProcessInput();

void SendKeyPackets();

DWORD WINAPI gameSend(LPVOID arg);
DWORD WINAPI gameReceive(LPVOID arg);
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	srand((unsigned int)time(NULL));
	HWND hwnd;
	MSG Message{};
	WNDCLASSEX WndClass{};
	g_hInst = hInstance;
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);
	hwnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1516, 939, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	if (!game.Initialize(hwnd)) {
		return 1;
	}

	TIMER.Initilaize();
	INPUT.Initialize(hwnd);
	LoadResources();
	

	if (game.ConnectWithServer(serverIP)) {
		game.LoginToGame();

		//네트워크용 쓰레드 생성
		game.hThreadForSend = CreateThread(NULL, 0, gameSend, NULL, 0, NULL);
		game.hThreadForReceive = CreateThread(NULL, 0, gameReceive, NULL, 0, NULL);
	}
	else {
		cerr << "Failed to connect to Server." << endl;
		return 1;
	}

	//키 이벤트 전송 이벤트
	TIMER.Reset();

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT) break;
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{
			TIMER.Tick(120.f);
			Update();
			Render();
		}
	}

	/*HANDLE threads[] = { hThreadForSend, hThreadForReceive };
	WaitForMultipleObjects(2, threads, TRUE, INFINITE);*/

	game.Destroy();

	return Message.wParam;
}

void LoadResources()
{
	//이미지 로드
	{
		imgBall.Load(TEXT("바운스볼 PNG/공.png"));

		imgBlock.Load(TEXT("바운스볼 PNG/블럭.png"));
		imgSwitchBk.Load(TEXT("바운스볼 PNG/전기스위치블럭.png"));
		imgElectricBk.Load(TEXT("바운스볼 PNG/전기블럭.png"));

		imgStarAni.Load(TEXT("바운스볼 PNG/별 스프라이트.png"));
		imgDeadAni.Load(TEXT("바운스볼 PNG/공 스프라이트.png"));

		imgStartScreen.Load(TEXT("바운스볼 PNG/시작화면.png"));
		imgStageScreen.Load(TEXT("바운스볼 PNG/스테이지.png"));
		imgStopScreen.Load(TEXT("바운스볼 PNG/일시정지.png"));
		imgClearScreen.Load(TEXT("바운스볼 PNG/게임클리어.png"));
		imgPlayScreen.Load(TEXT("바운스볼 PNG/게임플레이배경.png"));
		imgMaptoolScreen.Load(TEXT("바운스볼 PNG/맵툴.png"));

		imgHomeButton.Load(TEXT("바운스볼 PNG/홈버튼.png"));
		imgResetButton.Load(TEXT("바운스볼 PNG/맵툴_리셋.png"));
		imgLoadButton.Load(TEXT("바운스볼 PNG/맵툴_불러오기.png"));
		imgSaveButton.Load(TEXT("바운스볼 PNG/맵툴_저장.png"));
		imgEraseButton.Load(TEXT("바운스볼 PNG/맵툴_지우개.png"));
		imgPlayButton.Load(TEXT("바운스볼 PNG/맵툴_플레이.png"));

		imgBlockList.Load(TEXT("바운스볼 PNG/맵툴_블럭 선택 목록.png"));
		imgOutline.Load(TEXT("바운스볼 PNG/맵툴_블럭 선택 테두리.png"));

		imgWaiting.Load(TEXT("바운스볼 PNG/로비화면.png"));
		imgMapList.Load(TEXT("바운스볼 PNG/맵 리스트.png"));
	}
	// 사운드 로드
	{
		result = FMOD::System_Create(&ssystem); //--- 사운드 시스템 생성
		if (result != FMOD_OK)
			exit(0);
		ssystem->init(32, FMOD_INIT_NORMAL, extradriverdata); //--- 사운드 시스템 초기화
		ssystem->createSound("Sound/ball.ogg", FMOD_LOOP_OFF, 0, &ballCrach_Sound); //--- 공 튀기는 소리
		ssystem->createSound("Sound/telpo.mp3", FMOD_LOOP_OFF, 0, &Telpo_Sound); //--- 공 텔포하는 소리 (텔포, 블랙홀)
		ssystem->createSound("Sound/eatStar.mp3", FMOD_LOOP_OFF, 0, &EatStar_Sound); //--- 별 먹으면 나는 소리
		ssystem->createSound("Sound/balldeath.wav", FMOD_LOOP_OFF, 0, &ballDeath_Sound); //--- 공 죽으면 나는 소리
		ssystem->createSound("Sound/Click.mp3", FMOD_LOOP_OFF, 0, &Click_Sound); //--- 클릭
		ssystem->createSound("Sound/GameClear.mp3", FMOD_LOOP_OFF, 0, &GameClear_Sound); //--- 게임 클리어
		ssystem->createSound("Sound/musicbk.mp3", FMOD_LOOP_OFF, 0, &MusicBk_Sound); //--- 게임 클리어
	}

	const wchar_t* fontPath = L"고령딸기체+OTF.otf";
	AddFontResource(fontPath);
}

void Update()
{
#pragma region key event
	INPUT.Update();

	POINT MouseLC = INPUT.GetMousePosition();

	switch (game.GamePlay)
	{
	case Start:
	case StageSelect:
	case StagePlay:
	case StageStop:
	case StageClear:
	case StageWaiting:
	case StageDeath:
	{
		ProcessInput();
		break;
	}
	case CustomSelect:
	case CustomSelect2:
	{
		ProcessInput();
		if (INPUT.IsKeyUp(KEY_TYPE::LBUTTON)) {
			int offset = game.GamePlay == CustomSelect ? 0 : game.customList.size() / 2;
			for (int i = 0; i < game.customList.size() / 2; i++) {
				if (PtInRect(&game.mapNameRect[i], MouseLC)) {
					game.SendSelectMapPacket(i + offset);
				}
			}
		}
		break;
	}
	case CustomMode: {
		ProcessInput();

		HWND hwnd = game.hwnd;

		if(INPUT.IsKeyDown(KEY_TYPE::LBUTTON) || INPUT.IsKeyDown(KEY_TYPE::LBUTTON)){
			drag = true;

			if (MouseLC.y >= 756 && MouseLC.y <= 756 + 60) {
				game.Scheck = click;
				for (int i = 0; i < 22; i++) {
					if (MouseLC.x >= 17 + 60 * i + 7 * i && MouseLC.x <= 17 + 60 * i + 7 * i + 60)
						selection = i;
				}
			}
			else if (MouseLC.y >= 756 + 60 + 7 && MouseLC.y <= 756 + 60 + 7 + 60) {
				game.Scheck = click;
				for (int i = 0; i < 22; i++) {
					if (MouseLC.x >= 17 + 60 * i + 7 * i && MouseLC.x <= 17 + 60 * i + 7 * i + 60)
						selection = i + 22;
				}
			}
			// �÷��� ��ư
			else if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 164 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
				game.Scheck = click;
				if (BallStartLC.x == -1 || BallStartLC.y == -1) {
					TCHAR a[100];
					wsprintf(a, L"�� ��ġ�� �������ּ���.");
					MessageBox(hwnd, a, L"�˸�", MB_OK);
					drag = false;
					break;
				}
				game.ball = { (float)BallStartLC.x * side + 30, (float)BallStartLC.y * side + 30, 0, 0, 0 };
				game.GamePlay = CustomPlay;
			}
			else if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				selection = -1;
			}
			else if (MouseLC.x >= 1325 && MouseLC.x <= 1325 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				memset(game.Map, 0, sizeof(game.Map));
				BallStartLC = { -1, -1 };
			}
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				//맵 불러오기
				game.Scheck = click;

			}
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
				game.Scheck = click;
				drag = false;

			}
		}
		if (INPUT.IsKeyUp(KEY_TYPE::LBUTTON)) {
			if (drag) drag = false;
		}
	}
		break;
	default:
		break;
	}
#pragma endregion

	// 공 관련 효과음 재생
	switch (game.Scheck)
	{
		case ballcrach: {
			ssystem->playSound(ballCrach_Sound, 0, false, &channel);
			channel->setVolume(0.35);
			game.Scheck = X;
			break;
		}
		case telpo: {
			ssystem->playSound(Telpo_Sound, 0, false, &channel);
			channel->setVolume(0.5);
			game.Scheck = X;
			break;
		}
		case eatstar: {
			ssystem->playSound(EatStar_Sound, 0, false, &channel);
			channel->setVolume(1);
			game.Scheck = X;
			break;
		}
		case balldeath: {
			ssystem->playSound(ballDeath_Sound, 0, false, &channel);
			channel->setVolume(0.5);
			game.Scheck = X;
			break;
		}
		case click: {
			ssystem->playSound(Click_Sound, 0, false, &channel);
			channel->setVolume(1);
			game.Scheck = X;
			break;
		}
		case gameclear: {
			ssystem->playSound(GameClear_Sound, 0, false, &channel);
			channel->setVolume(1);
			game.Scheck = X;
			break;
		}
		case music: {
			ssystem->playSound(MusicBk_Sound, 0, false, &channel);
			channel->setVolume(1);
			game.Scheck = X;
			break;
		}
	}
}

void Render()
{
	HDC hdc; HDC mdc; HBITMAP HBitmap, OldBitmap;
	hdc = GetDC(game.hwnd);
	mdc = CreateCompatibleDC(hdc);
	HBitmap = CreateCompatibleBitmap(hdc, game.window.right, game.window.bottom);
	OldBitmap = (HBITMAP)SelectObject(mdc, (HBITMAP)HBitmap);
	HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
	FillRect(mdc, &game.window, brush);
	DeleteObject(brush);

	POINT MouseLC = INPUT.GetMousePosition();

	POINT BallStartLC = game.ballStartPos;

	//맵툴 블럭 설치
	if (game.GamePlay == CustomMode && drag == true && MouseLC.x >= 21 && MouseLC.x <= 21 + 1200 && MouseLC.y >= 21 && MouseLC.y <= 21 + 720) {
		if (selection > 0) {// 블럭이 선택되었을 경우
			if ((MouseLC.x - 21) / 48 == BallStartLC.x && (MouseLC.y - 21) / 48 == BallStartLC.y) // 공이 있을 경우
				BallStartLC = { -1, -1 };

			if (selection == 6 || selection == 8) {
				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						if (game.Map[y][x] == selection) {
							game.Map[y][x] = 0;
						}
					}
				}
			}
			game.Map[(MouseLC.y - 21) / 48][(MouseLC.x - 21) / 48] = selection;
			if (game.list[selection - 1].type == SwitchBk)
				game.isSwitchOff = game.list[selection - 1].subtype;
		}
		else if (selection == 0) { // 공이 선택되었을 경우
			if (game.Map[(MouseLC.y - 21) / 48][(MouseLC.x - 21) / 48]) // 블럭이 있을 경우
				game.Map[(MouseLC.y - 21) / 48][(MouseLC.x - 21) / 48] = 0;
			BallStartLC = { (MouseLC.x - 21) / 48, (MouseLC.y - 21) / 48 };
		}
		else {// 지우개가 선택되었을 경우
			if ((MouseLC.x - 21) / 48 == BallStartLC.x && (MouseLC.y - 21) / 48 == BallStartLC.y) // 공 지우기
				BallStartLC = { -1, -1 };
			else
				game.Map[(MouseLC.y - 21) / 48][(MouseLC.x - 21) / 48] = 0; // 블럭 지우기
		}
	}

	// 게임 시작 화면
	if (game.GamePlay == Start) {
		if (MouseLC.x <= 410 && MouseLC.y >= 593 && MouseLC.y <= 693)
			imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom); // 스테이지 모드 위 커서
		else if (MouseLC.x <= 410 && MouseLC.y >= 717 && MouseLC.y <= 817)
			imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 3000, 0, game.window.right, game.window.bottom); // 커스텀 모드 위 커서
		else
			imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // 기본 시작화면
	}
	else if (game.GamePlay == StageSelect) {
		if (MouseLC.x >= 93 && MouseLC.x <= 442 && MouseLC.y >= 365 && MouseLC.y <= 715)
			imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom);
		else if (MouseLC.x >= 574 && MouseLC.x <= 923 && MouseLC.y >= 365 && MouseLC.y <= 715)
			imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 3000, 0, game.window.right, game.window.bottom);
		else if (MouseLC.x >= 1060 && MouseLC.x <= 1408 && MouseLC.y >= 365 && MouseLC.y <= 715)
			imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 4500, 0, game.window.right, game.window.bottom);
		else
			imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);

		if (MouseLC.x >= 1368 && MouseLC.x <= 1448 && MouseLC.y >= 48 && MouseLC.y <= 128)
			imgHomeButton.Draw(mdc, 1368, 48, 80, 80, 80, 0, 80, 80); // 홈버튼 위 커서
		else
			imgHomeButton.Draw(mdc, 1368, 48, 80, 80, 0, 0, 80, 80); // 기본 홈버튼
	}
	else if (game.GamePlay == CustomSelect) {
		imgMapList.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);
		for (int i = 0; i < game.customList.size() / 2; i++) {
			if (!game.customList[i].empty()) {
				std::wstring name = AnsiToWString(game.customList[i]);
				DrawText(mdc, name.c_str(), name.size(), &game.mapNameRect[i], DT_LEFT | DT_WORDBREAK);
			}
		}
	}
	else if (game.GamePlay == CustomSelect2) {
		imgMapList.Draw(mdc, 0, 0, game.window.right, game.window.bottom, game.window.right, 0, game.window.right + game.window.right, game.window.bottom);

		for (int i = game.customList.size() / 2; i < game.customList.size(); i++) {
			int idx = game.customList.size() % (NAME_SIZE / 2);
			if (!game.customList[i].empty()) {
				std::wstring name = AnsiToWString(game.customList[i]);
				DrawText(mdc, name.c_str(), name.size(), &game.mapNameRect[idx], DT_LEFT | DT_WORDBREAK);
			}
		}
	}
	else if (game.GamePlay == CustomMode) {
		imgMaptoolScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);

		//버튼
		if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 164 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78)
			imgPlayButton.Draw(mdc, 1239, 16, 164, 78, 164, 0, 164, 78); // 플레이버튼 위 커서
		else
			imgPlayButton.Draw(mdc, 1239, 16, 164, 78, 0, 0, 164, 78); // 기본 플레이버튼
		if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78)
			imgSaveButton.Draw(mdc, 1410, 16, 78, 78, 78, 0, 78, 78); // 저장버튼 위 커서
		else
			imgSaveButton.Draw(mdc, 1410, 16, 78, 78, 0, 0, 78, 78); // 기본 저장버튼
		if ((MouseLC.x >= 1239 && MouseLC.x <= 1239 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78))
			imgEraseButton.Draw(mdc, 1239, 105, 78, 78, 78, 0, 78, 78); // 지우개버튼 위 커서
		else
			imgEraseButton.Draw(mdc, 1239, 105, 78, 78, 0, 0, 78, 78); // 기본 지우개버튼
		if (MouseLC.x >= 1325 && MouseLC.x <= 1325 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78)
			imgResetButton.Draw(mdc, 1325, 105, 78, 78, 78, 0, 78, 78); // 리셋버튼 위 커서
		else
			imgResetButton.Draw(mdc, 1325, 105, 78, 78, 0, 0, 78, 78); // 기본 리셋버튼
		if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78)
			imgLoadButton.Draw(mdc, 1410, 105, 78, 78, 78, 0, 78, 78); // 불러오기버튼 위 커서
		else
			imgLoadButton.Draw(mdc, 1410, 105, 78, 78, 0, 0, 78, 78); // 기본 불러오기버튼

		// 블럭 목록
		for (int i = 0; i < 28; i++) {
			if (i == selection)
				imgOutline.Draw(mdc, 17 + 60 * (i % 14) + 7 * (i % 14), 756 + 68 * (i / 14), 60, 60, 60, 0, 60, 60); // 빨간테두리
			else
				imgOutline.Draw(mdc, 17 + 60 * (i % 14) + 7 * (i % 14), 756 + 68 * (i / 14), 60, 60, 0, 0, 60, 60); // 검정테두리
		}
		imgBlockList.Draw(mdc, 20, 759, 925, 122, 0, 0, 925, 122); // 블럭

		//맵
		for (int i = 0; i < 15; i++) {
			for (int j = 0; j < 25; j++) {
				if (game.Map[i][j] == 9 || game.Map[i][j] == 10) // 스위치
					imgSwitchBk.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, game.isSwitchOff * side, 0, side, side);
				else if (game.Map[i][j] == 11) // 전기블럭
					imgElectricBk.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, game.isSwitchOff * side, 0, side, side);
				else if (game.Map[i][j]) // 기타 모두
					imgBlock.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, (game.Map[i][j] - 1) * side, 0, side, side);
			}
		}

		//공
		if (BallStartLC.x >= 0 && BallStartLC.y >= 0)
			imgBall.Draw(mdc, 21 + BallStartLC.x * 48 + 14, 21 + BallStartLC.y * 48 + 14, 20, 20, 0, 0, rd * 2, rd * 2);


		//선택된 것
		if (selection > 0) { // 블럭
			if (selection == 9 || selection == 10) // 스위치
				imgSwitchBk.Draw(mdc, 1315, 307, 80, 80, game.isSwitchOff * side, 0, side, side);
			else if (selection == 11) // 전기블럭
				imgElectricBk.Draw(mdc, 1315, 307, 80, 80, game.isSwitchOff * side, 0, side, side);
			else // 기타 모두
				imgBlock.Draw(mdc, 1315, 307, 80, 80, (selection - 1) * side, 0, side, side);
		}
		else if (selection == 0) // 공
			imgBall.Draw(mdc, 1315 + 15, 307 + 15, 50, 50, 0, 0, rd * 2, rd * 2);
		else // 지우개
			imgEraseButton.Draw(mdc, 1315, 307, 78, 78, 0, 0, 78, 78);
	}
	else if (game.GamePlay == StageWaiting) {
		imgWaiting.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // 기본 시작화면
	}

	// 게임 플레이 화면
	else if (game.GamePlay == StagePlay || game.GamePlay == StageClear || game.GamePlay == StageStop || game.GamePlay == CustomPlay || game.GamePlay == StageDeath || game.GamePlay == CustomDeath) { // 죽었고 파티클 애니메이션 있을 때 그리려고 추가함
		imgPlayScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);

		//블럭 출력
		for (int i = 0; i < 15; ++i) {
			for (int j = 0; j < 25; ++j) {
				if (game.Map[i][j] == 9 || game.Map[i][j] == 10) // 스위치
					imgSwitchBk.Draw(mdc, j * side, i * side, side, side, game.isSwitchOff * side, 0, side, side);
				else if (game.Map[i][j] == 11) // 전기블럭
					imgElectricBk.Draw(mdc, j* side, i* side, side, side, game.isSwitchOff* side, 0, side, side);
				else if (game.Map[i][j]) // 기타 모두
					imgBlock.Draw(mdc, j * side, i * side, side, side, (game.Map[i][j] - 1) * side, 0, side, side);
			}
		}
		if (!game.ball.isDead && game.ball.playerID != 7) {
			imgBall.Draw(mdc, game.ball.x - rd, game.ball.y - rd, rd * 2, rd * 2, 0, 0, 25, 25);
		}
		if (!game.otherPlayer.isDead && game.otherPlayer.playerID != 7) {
			imgBall.Draw(mdc, game.otherPlayer.x - rd, game.otherPlayer.y - rd, rd * 2, rd * 2, 100, 0, 25, 25);
		}

		// 파티클 출력
		for (int i = 0; i < game.animation.size(); i++) {
			switch (game.animation[i].type) {
			case StageDeath: // 이넘 겹쳐서 걍 이거씀
				imgDeadAni.AlphaBlend(mdc, game.animation[i].x, game.animation[i].y, 180, 180, 180 * (game.animation[i].ani / 2), 180 * game.animation[i].subtype, 180, 180, 170, AC_SRC_OVER);
				break;
			case Star:
				imgStarAni.AlphaBlend(mdc, game.animation[i].x, game.animation[i].y, 180, 180, 180 * (game.animation[i].ani / 2), 180 * game.animation[i].subtype, 180, 180, 170, AC_SRC_OVER);
				break;
			}
			game.animation[i].ani++;
			if (game.animation[i].ani == 40) {
				if (game.animation[i].type == Star) { // 애니메이션 끝나고 clear로 바뀜
					game.starcnt--;
					if (game.starcnt == 0) {
						if (game.GamePlay == StagePlay || game.GamePlay == StageDeath) {// 별 먹고 죽었을 때도 클리어되게,,, 동시에 일어나도 WM_TIMER가먼저 돌아가서 아마 death가 먼저 될거라 괜찮을거같긴한데 버그나면뭐,, 아쉬운거임
							game.Scheck = gameclear;
							game.GamePlay = StageClear;
						}
						else if (game.GamePlay == CustomPlay || game.GamePlay == CustomDeath)
							game.GamePlay = CustomMode;
					}
				}
				game.animation.erase(game.animation.begin() + i);
			}
		}

		// 화면 출력
		if (game.GamePlay == StageStop) {
			if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 284 && MouseLC.y <= 381)
				imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom); // 메인화면 버튼 위 커서
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 397 && MouseLC.y <= 494)
				imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 3000, 0, game.window.right, game.window.bottom); // 스테이지 버튼 위 커서
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 509 && MouseLC.y <= 606)
				imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 4500, 0, game.window.right, game.window.bottom); // 재시작 위 커서
			else
				imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // 기본 정지화면
		}
		else if (game.GamePlay == StageClear) {
			if (MouseLC.x >= 587 && MouseLC.x <= 587 + 674 && MouseLC.y >= 530 && MouseLC.y <= 530 + 155)
				imgClearScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom);
			else
				imgClearScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);
		}
	}

	BitBlt(hdc, 0, 0, game.window.right, game.window.bottom, mdc, 0, 0, SRCCOPY);

	SelectObject(mdc, OldBitmap);
	DeleteObject(HBitmap);
	DeleteDC(mdc);
	ReleaseDC(game.hwnd, hdc);
}

void ProcessInput()
{
	EnterCriticalSection(&(INPUT.keyEventCS));
	if (INPUT.IsKeyDown(KEY_TYPE::RIGHT)) {
		keyEventQueue.push({ KEY_TYPE::RIGHT, KEY_STATE::DOWN });
	}
	else if (INPUT.IsKeyUp(KEY_TYPE::RIGHT)) {
		keyEventQueue.push({ KEY_TYPE::RIGHT, KEY_STATE::UP });
	}
	if (INPUT.IsKeyDown(KEY_TYPE::LEFT)) {
		keyEventQueue.push({ KEY_TYPE::LEFT, KEY_STATE::DOWN });
	}
	else if (INPUT.IsKeyUp(KEY_TYPE::LEFT)) {
		keyEventQueue.push({ KEY_TYPE::LEFT, KEY_STATE::UP });
	}
	if (INPUT.IsKeyDown(KEY_TYPE::ESCAPE)) {
		keyEventQueue.push({ KEY_TYPE::ESCAPE, KEY_STATE::DOWN });
	}
	if (INPUT.IsKeyDown(KEY_TYPE::L)) {
		keyEventQueue.push({ KEY_TYPE::L, KEY_STATE::DOWN });
	}
	LeaveCriticalSection(&(INPUT.keyEventCS));

	EnterCriticalSection(&(INPUT.mouseEventCS));
	if (INPUT.IsKeyDown(KEY_TYPE::LBUTTON)) {
		mouseEventQueue.push(KEY_TYPE::LBUTTON);
	}
	if (INPUT.IsKeyDown(KEY_TYPE::RBUTTON)) {
		mouseEventQueue.push(KEY_TYPE::RBUTTON);
	}
	LeaveCriticalSection(&(INPUT.mouseEventCS));
}

void SendKeyPackets()
{
	EnterCriticalSection(&(INPUT.keyEventCS));
	while (!keyEventQueue.empty()) {
		game.SendKeyPacket(0, keyEventQueue.front());
		keyEventQueue.pop();
	}
	LeaveCriticalSection(&(INPUT.keyEventCS));

	EnterCriticalSection(&(INPUT.mouseEventCS));
	while (!mouseEventQueue.empty()) {
		game.SendMousePacket(mouseEventQueue.front(), INPUT.GetMousePosition());
		mouseEventQueue.pop();
	}
	LeaveCriticalSection(&(INPUT.mouseEventCS));
}

DWORD __stdcall gameSend(LPVOID arg)
{
	while (game.isConnected)
	{
		SendKeyPackets();
	}

	return 0;
}

DWORD __stdcall gameReceive(LPVOID arg)
{
	game.ReceiveServerData();

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE: {
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	//switch (message) {
	//case WM_INITDIALOG:
	//	hEdit = GetDlgItem(hDlg, IDC_EDIT1); // 텍스트 입력 필드
	//	return (INT_PTR)TRUE;
	//case WM_COMMAND:
	//	if (LOWORD(wParam) == IDOK) {
	//		char buffer[256];
	//		GetWindowText(hEdit, buffer, sizeof(buffer));
	//		MessageBox(hDlg, buffer, "입력한 텍스트", MB_OK);
	//		EndDialog(hDlg, IDOK);
	//		return (INT_PTR)TRUE;
	//	}
	//	else if (LOWORD(wParam) == IDCANCEL) {
	//		EndDialog(hDlg, IDCANCEL);
	//		return (INT_PTR)TRUE;
	//	}
	//	break;
	//}
	return (INT_PTR)FALSE;
}