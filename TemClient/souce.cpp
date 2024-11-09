#include "stdafx.h"
#include "ClientManager.h"

// 최대 vx = 21
// 최대 vy = 40
// ax = 7
// x축 최대 이동거리 = 180
// x축 최소 이동거리 = 90
// 좌우직진블럭 vx = 60
// 상승블럭 vy = -65
// 끈적이 블럭 좌 vy = 5 우 vy = 5.1

ClientManager game;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Trip of a Ball";
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	srand((unsigned int)time(NULL));
	HWND hwnd;
	MSG Message;
	WNDCLASSEX WndClass;
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

	if (!game.Initialize()) {
		return 1;
	}

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc; HDC mdc; HBITMAP HBitmap, OldBitmap;
	HFONT hFont, OldFont;
	static CImage imgBall, imgBasicBlock, imgFuctionBlock, imgSwitchBk, imgElectricBk,
		imgStartScreen, imgStageScreen, imgStopScreen, imgClearScreen, imgPlayScreen, imgMaptoolScreen,
		imgHomeButton, imgResetButton, imgLoadButton, imgSaveButton, imgEraseButton, imgPlayButton,
		imgBlockList, imgOutline,
		imgStarAni, imgDeadAni;
	static POINT BallStartLC, MouseLC;
	static OPENFILENAME OFN;
	static TCHAR filter[] = L"Every File(*.*)\0*.*\0Text File\0*.txt;*.doc\0";
	static TCHAR lpstrFile[100], str[20];
	static bool drag = false;
	static int selection = 0, electictimer = 0, bestscore = 0;
	int ani;
	SIZE size;
	const wchar_t* fontPath = L"고령딸기체+OTF.otf";
	AddFontResource(fontPath);

	static FMOD::System* ssystem;
	static FMOD::Sound* ballCrach_Sound, * Telpo_Sound, * EatStar_Sound, * ballDeath_Sound, * Click_Sound, * GameClear_Sound, * MusicBk_Sound;
	static FMOD::Channel* channel = 0;
	static FMOD_RESULT result;
	static void* extradriverdata = 0;

	switch (uMsg) {
	case WM_CREATE: {
		SetTimer(hwnd, 1, 10, NULL);
		GetClientRect(hwnd, &game.window);
		// 이미지 로드
		{
			imgBall.Load(TEXT("바운스볼 PNG/공.png"));
			imgBasicBlock.Load(TEXT("바운스볼 PNG/기본블럭.png"));
			imgFuctionBlock.Load(TEXT("바운스볼 PNG/기능블럭.png"));
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

		MouseLC = { 0, 0 };
		break;
	}
	case WM_CHAR: {
		switch (wParam)
		{
		case 'a':
			game.Scheck = ballcrach;
			break;
		case 'q':
		case 'Q':
			PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	}
	case WM_KEYDOWN: {
		switch (wParam)
		{
		case VK_ESCAPE: {
			if (game.GamePlay == StagePlay)
				game.GamePlay = StageStop;
			else if (game.GamePlay == StageStop)
				game.GamePlay = StagePlay;
			else if (game.GamePlay == CustomMode || game.GamePlay == StageSelect)
				game.GamePlay = Start;
			else if (game.GamePlay == CustomPlay)
				game.GamePlay = CustomMode;
			else if (game.GamePlay == StageClear)
				game.GamePlay = StageSelect;
			break;
		}
		case VK_RIGHT: {
			if (game.isRightPressed == false) {
				if (game.ball.vy == 5) {
					game.Scheck = telpo;
					game.ball.vx = -21;
					game.ball.vy = -40;
				}
				else if (game.ball.vy == 5.1) {
					game.Scheck = telpo;
					game.ball.vy = -40;
					game.ball.vx = 21;
				}
			}
			break;
		}
		case VK_LEFT: {
			if (game.isLeftPressed == false) {
				if (game.ball.vy == 5) {
					game.Scheck = telpo;
					game.Scheck = telpo;
					game.ball.vy = -40;
					game.ball.vx = -21;
				}
				else if (game.ball.vy == 5.1) {
					game.Scheck = telpo;
					game.Scheck = telpo;
					game.ball.vx = 21;
					game.ball.vy = -40;
				}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case WM_KEYUP: {
		switch (wParam) {
		case VK_RIGHT: {
			if (game.ball.vy == 5 || game.ball.vy == 5.1)
				game.isRightPressed = false;
			break;
		}
		case VK_LEFT: {
			if (game.ball.vy == 5 || game.ball.vy == 5.1)
				game.isLeftPressed = false;
			break;
		}
		default:
			break;
		}
		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hwnd, &ps);
		mdc = CreateCompatibleDC(hdc);
		HBitmap = CreateCompatibleBitmap(hdc, game.window.right, game.window.bottom);
		OldBitmap = (HBITMAP)SelectObject(mdc, (HBITMAP)HBitmap);
		FillRect(mdc, &game.window, WHITE_BRUSH);

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
			if (MouseLC.x <= 430 && MouseLC.y >= 515 && MouseLC.y <= 615)
				imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom); // 스테이지 모드 위 커서
			else if (MouseLC.x <= 430 && MouseLC.y >= 640 && MouseLC.y <= 740)
				imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 3000, 0, game.window.right, game.window.bottom); // 서바이벌 모드 위 커서
			else if (MouseLC.x <= 430 && MouseLC.y >= 763 && MouseLC.y <= 863)
				imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 4500, 0, game.window.right, game.window.bottom); // 커스터마이징 위 커서
			else
				imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // 기본 시작화면
		}
		else if (game.GamePlay == StageSelect) {
			if (MouseLC.x >= 93 && MouseLC.x <= 442 && MouseLC.y >= 365 && MouseLC.y <= 715)
				imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom); // 스테이지 모드 위 커서
			else if (MouseLC.x >= 574 && MouseLC.x <= 923 && MouseLC.y >= 365 && MouseLC.y <= 715)
				imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 3000, 0, game.window.right, game.window.bottom); // 서바이벌 모드 위 커서
			else if (MouseLC.x >= 1060 && MouseLC.x <= 1408 && MouseLC.y >= 365 && MouseLC.y <= 715)
				imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 4500, 0, game.window.right, game.window.bottom); // 커스터마이징 위 커서
			else
				imgStageScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // 기본 시작화면

			if (MouseLC.x >= 1368 && MouseLC.x <= 1448 && MouseLC.y >= 48 && MouseLC.y <= 128)
				imgHomeButton.Draw(mdc, 1368, 48, 80, 80, 80, 0, 80, 80); // 홈버튼 위 커서
			else
				imgHomeButton.Draw(mdc, 1368, 48, 80, 80, 0, 0, 80, 80); // 기본 홈버튼
		}
		else if (game.GamePlay == CustomMode) {
			imgMaptoolScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // 기본 시작화면

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
			for (int i = 0; i < 44; i++) {
				if (i == selection)
					imgOutline.Draw(mdc, 17 + 60 * (i % 22) + 7 * (i % 22), 756 + 68 * (i / 22), 60, 60, 60, 0, 60, 60); // 빨간테두리
				else
					imgOutline.Draw(mdc, 17 + 60 * (i % 22) + 7 * (i % 22), 756 + 68 * (i / 22), 60, 60, 0, 0, 60, 60); // 검정테두리
			}
			imgBlockList.Draw(mdc, 20, 759, 1461, 122, 0, 0, 1461, 122); // 블럭

			//맵
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 25; j++) {
					if (game.Map[i][j]) {
						if (game.list[game.Map[i][j] - 1].type < BasicBk) { // 기능블럭 
							imgFuctionBlock.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, game.list[game.Map[i][j] - 1].type * side, 0, side, side);
						}
						else {
							switch (game.list[game.Map[i][j] - 1].type) {
							case BasicBk: // 기본블럭
								imgBasicBlock.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, game.list[game.Map[i][j] - 1].subtype * side, 0, side, side);
								break;
							case SwitchBk:
								imgSwitchBk.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, game.isSwitchOff * side, 0, side, side);
								break;
							case ElectricBk:
								imgElectricBk.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, game.isSwitchOff * side, 0, side, side);
								break;
							}
						}
					}
				}
			}

			//공
			if (BallStartLC.x >= 0 && BallStartLC.y >= 0)
				imgBall.Draw(mdc, 21 + BallStartLC.x * 48 + 14, 21 + BallStartLC.y * 48 + 14, 20, 20, 0, 0, rd * 2, rd * 2);

			//선택된 것
			if (selection > 0) { // 블럭
				if (game.list[selection - 1].type < BasicBk) { // 기능블럭 
					imgFuctionBlock.Draw(mdc, 1315, 307, 80, 80, game.list[selection - 1].type * side, 0, side, side);
				}
				else {
					switch (game.list[selection - 1].type) {
					case BasicBk: // 기본블럭
						imgBasicBlock.Draw(mdc, 1315, 307, 80, 80, game.list[selection - 1].subtype * side, 0, side, side);
						break;
					case SwitchBk:
						imgSwitchBk.Draw(mdc, 1315, 307, 80, 80, game.isSwitchOff * side, 0, side, side);
						break;
					case ElectricBk:
						imgElectricBk.Draw(mdc, 1315, 307, 80, 80, game.isSwitchOff * side, 0, side, side);
						break;
					}
				}
			}
			else if (selection == 0) // 공
				imgBall.Draw(mdc, 1315 + 15, 307 + 15, 50, 50, 0, 0, rd * 2, rd * 2);
			else // 지우개
				imgEraseButton.Draw(mdc, 1315, 307, 78, 78, 0, 0, 78, 78);
		}

		// 게임 플레이 화면
		else if (game.GamePlay == StagePlay || game.GamePlay == StageClear || game.GamePlay == StageStop || game.GamePlay == CustomPlay || game.GamePlay == StageDeath || game.GamePlay == CustomDeath) { // 죽었고 파티클 애니메이션 있을 때 그리려고 추가함
			imgPlayScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);

			//블럭 출력
			for (int y = 0; y < 15; ++y) {
				for (int i = 0; i < game.block[y].size(); ++i) {
					if (game.block[y][i].type < BasicBk) { // 기능블럭 
						imgFuctionBlock.Draw(mdc, game.block[y][i].x * side, game.block[y][i].y * side, side, side, game.block[y][i].type * side, 0, side, side);
					}
					else {
						switch (game.block[y][i].type) {
						case BasicBk: // 기본블럭
							imgBasicBlock.Draw(mdc, game.block[y][i].x * side, game.block[y][i].y * side, side, side, game.block[y][i].subtype * side, 0, side, side);
							break;
						case SwitchBk:
							imgSwitchBk.Draw(mdc, game.block[y][i].x * side, game.block[y][i].y * side, side, side, game.isSwitchOff * side, 0, side, side);
							break;
						case ElectricBk:
							imgElectricBk.Draw(mdc, game.block[y][i].x * side, game.block[y][i].y * side, side, side, game.isSwitchOff * side, 0, side, side);
							break;
						}
					}
				}
			}

			//공 출력
			if (game.GamePlay != StageDeath && game.GamePlay != CustomDeath) { // 죽으면 출력 안하게
				imgBall.AlphaBlend(mdc, game.ball.x - rd, game.ball.y - rd, rd * 2, rd * 2, 0, 0, rd * 2, rd * 2, 255, AC_SRC_OVER); // 활성화공
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
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_TIMER: {
		// 리스폰
		if (game.GamePlay == StageDeath && game.animation.size() == 0) { // 뒤지고 애니메이션 끝나면 리스폰됨
			game.MakeVector();
			game.ball = { (float)BallStartLC.x, (float)BallStartLC.y, 0, 0, 0 };
			game.GamePlay = StagePlay;
		}
		else if (game.GamePlay == CustomDeath && game.animation.size() == 0) {
			game.MakeVector();
			game.ball = { (float)BallStartLC.x * side + 30, (float)BallStartLC.y * side + 30, 0, 0, 0 };
			game.GamePlay = CustomPlay;
		}

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

		//블럭/공 이동, 충돌체크
		if (game.GamePlay == StagePlay || game.GamePlay == CustomPlay || game.GamePlay == StageDeath || game.GamePlay == CustomDeath) {
			//공 이동
			game.MoveBall();

			game.ballrc = { (float)game.ball.x - rd, (float)game.ball.y - rd, (float)game.ball.x + rd, (float)game.ball.y + rd };

			// 공 충돌체크
			if (game.GamePlay == StagePlay || game.GamePlay == CustomPlay) { // 이미 뒤졌을땐 안돌아가게 하려고. 계속 돌아가면 애니메이션 벡터에 자꾸들어감
				// 바닥과 충돌
				if (game.ball.y + rd >= game.window.bottom) {
					game.animation.emplace_back(Block{ (int)game.ball.x - 90, (int)game.ball.y - 90, StageDeath, rand() % 4, 0 });
					game.Scheck = balldeath;
					if (game.GamePlay == StagePlay)
						game.GamePlay = StageDeath;
					else if (game.GamePlay == CustomPlay)
						game.GamePlay = CustomDeath;
				}

				// 블럭과 충돌
				else game.CrashExamin();
			}
		}

		InvalidateRect(hwnd, NULL, FALSE);
		break;
	}
	case WM_LBUTTONDOWN: {
		if (game.GamePlay == Start) {
			if (MouseLC.x <= 430 && MouseLC.y >= 515 && MouseLC.y <= 615) { // 스테이지 버튼
				game.Scheck = click;
				game.GamePlay = StageSelect;
			}
			else if (MouseLC.x <= 430 && MouseLC.y >= 763 && MouseLC.y <= 863) { // 맵툴 버튼
				game.Scheck = click;
				game.GamePlay = CustomMode;
				BallStartLC = { -1, -1 };
				game.isSwitchOff = 0;
				memset(game.Map, 0, sizeof(game.Map));
				selection = 0;
			}
		}
		else if (game.GamePlay == StageSelect) {
			if (MouseLC.x >= 93 && MouseLC.x <= 442 && MouseLC.y >= 365 && MouseLC.y <= 715) {
				game.Scheck = click; // 이제 여기에 클릭하면 1 2 3으로 해가지고 스테이지 고르면 파일 불러와서 벡터배열에 넣어주는 함수 짜서 넣으면 될 듯
				ifstream in{ "바운스볼 맵/Stage1.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> game.Map[y][x];
					}
				}

				in >> BallStartLC.x;
				in >> BallStartLC.y;
				in >> game.isSwitchOff;
				BallStartLC.x = BallStartLC.x * side + 30;
				BallStartLC.y = BallStartLC.y * side + 30;

				game.GamePlay = StageDeath;
				in.close();
			}
			else if (MouseLC.x >= 574 && MouseLC.x <= 923 && MouseLC.y >= 365 && MouseLC.y <= 715) {
				game.Scheck = click; // 이제 여기에 클릭하면 1 2 3으로 해가지고 스테이지 고르면 파일 불러와서 벡터배열에 넣어주는 함수 짜서 넣으면 될 듯
				ifstream in{ "바운스볼 맵/Stage2.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> game.Map[y][x];
					}
				}

				in >> BallStartLC.x;
				in >> BallStartLC.y;
				in >> game.isSwitchOff;
				BallStartLC.x = BallStartLC.x * side + 30;
				BallStartLC.y = BallStartLC.y * side + 30;

				game.GamePlay = StageDeath;
				in.close();
			}
			else if (MouseLC.x >= 1060 && MouseLC.x <= 1408 && MouseLC.y >= 365 && MouseLC.y <= 715) {
				game.Scheck = click; // 이제 여기에 클릭하면 1 2 3으로 해가지고 스테이지 고르면 파일 불러와서 벡터배열에 넣어주는 함수 짜서 넣으면 될 듯
				ifstream in{ "바운스볼 맵/Stage3.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> game.Map[y][x];
					}
				}

				in >> BallStartLC.x;
				in >> BallStartLC.y;
				in >> game.isSwitchOff;
				BallStartLC.x = BallStartLC.x * side + 30;
				BallStartLC.y = BallStartLC.y * side + 30;

				game.GamePlay = StageDeath;
				in.close();
			}
			else if (MouseLC.x >= 1368 && MouseLC.x <= 1448 && MouseLC.y >= 48 && MouseLC.y <= 128) {
				game.Scheck = click;
				game.GamePlay = Start;
			}
			else if (MouseLC.x >= 1490 && MouseLC.x <= 1500 && MouseLC.y >= 850 && MouseLC.y <= 900) {
				game.Scheck = click; // 이제 여기에 클릭하면 1 2 3으로 해가지고 스테이지 고르면 파일 불러와서 벡터배열에 넣어주는 함수 짜서 넣으면 될 듯
				ifstream in{ "바운스볼 맵/Stage4.txt" };

				for (int y = 0; y < 15; ++y) {
					for (int x = 0; x < 25; ++x) {
						in >> game.Map[y][x];
					}
				}

				in >> BallStartLC.x;
				in >> BallStartLC.y;
				in >> game.isSwitchOff;
				BallStartLC.x = BallStartLC.x * side + 30;
				BallStartLC.y = BallStartLC.y * side + 30;

				game.GamePlay = StageDeath;
				in.close();
			}
		}
		else if (game.GamePlay == StageStop) {
			if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 284 && MouseLC.y <= 381) { // 메인화면 버튼 위 커서 
				game.Scheck = click;
				game.GamePlay = Start;
			}
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 397 && MouseLC.y <= 494) { // 스테이지 버튼 위 커서 
				game.Scheck = click;
				game.GamePlay = StageSelect;
			}
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 509 && MouseLC.y <= 606) { // 재시작 버튼 위 커서
				game.Scheck = click;
				game.MakeVector();
				game.GamePlay = StageDeath;
				game.ball = { (float)BallStartLC.x, (float)BallStartLC.y, 0, 0, 0 }; // 재시작 전에걸로 하면 death로 바뀌고 애니메이션 끝나고 넘어가야돼서 걍 바로 리스폰시킴
			}
		}
		else if (game.GamePlay == CustomMode) {
			drag = true;
			//블럭 선택
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
			// 플레이 버튼
			else if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 164 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
				game.Scheck = click;
				if (BallStartLC.x == -1 || BallStartLC.y == -1) {
					TCHAR a[100];
					wsprintf(a, L"공 위치를 선정해주세요.");
					MessageBox(hwnd, a, L"알림", MB_OK);
					drag = false;
					break;
				}
				game.ball = { (float)BallStartLC.x * side + 30, (float)BallStartLC.y * side + 30, 0, 0, 0 };
				game.GamePlay = CustomPlay;
				game.MakeVector();
			}
			// 지우개 버튼
			else if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				selection = -1;
			}
			// 리셋 버튼
			else if (MouseLC.x >= 1325 && MouseLC.x <= 1325 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				memset(game.Map, 0, sizeof(game.Map));
				BallStartLC = { -1, -1 };
			}
			// 불러오기 버튼
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				memset(&OFN, 0, sizeof(OPENFILENAME)); //--- 구조체 초기화
				OFN.lStructSize = sizeof(OPENFILENAME);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = filter;
				OFN.lpstrFile = lpstrFile;
				OFN.nMaxFile = 256;
				OFN.lpstrInitialDir = L".";

				if (GetOpenFileNameW(&OFN) != 0) { //--- 파일 함수 호출
					TCHAR a[100];
					wsprintf(a, L"%s 파일을 여시겠습니까 ?", OFN.lpstrFile);
					MessageBox(hwnd, a, L"열기 선택", MB_OK);

					ifstream in{ OFN.lpstrFile };

					for (int y = 0; y < 15; ++y) {
						for (int x = 0; x < 25; ++x) {
							in >> game.Map[y][x];
						}
					}

					in >> BallStartLC.x;
					in >> BallStartLC.y;
					in >> game.isSwitchOff;

					in.close();
				}
				drag = false;
			}
			// 저장 버튼
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
				game.Scheck = click;
				memset(&OFN, 0, sizeof(OPENFILENAME)); //--- 구조체 초기화
				OFN.lStructSize = sizeof(OPENFILENAME);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = filter;
				OFN.lpstrFile = lpstrFile;
				OFN.nMaxFile = 256;
				OFN.lpstrInitialDir = L".";

				if (GetSaveFileNameW(&OFN) != 0) { //--- 파일 함수 호출
					TCHAR a[100];
					wsprintf(a, L"%s 위치에 파일을 저장하시겠습니까 ?", OFN.lpstrFile);
					MessageBox(hwnd, a, L"저장하기 선택", MB_OK);
					TCHAR b[100];
					wsprintf(b, L"%s.txt", OFN.lpstrFile);

					ofstream out{ b };


					// 맵툴배열 저장
					for (int y = 0; y < 15; ++y) {
						for (int x = 0; x < 25; ++x) {
							out << game.Map[y][x] << " ";
						}
						out << endl;
					}
					// 공 시작위치, 전기 상태 저장
					out << BallStartLC.x << " " << BallStartLC.y << " " << game.isSwitchOff << endl;

					out.close();
				}
				drag = false;
			}
		}
		else if (game.GamePlay == StageClear) {
			if (MouseLC.x >= 587 && MouseLC.x <= 587 + 674 && MouseLC.y >= 530 && MouseLC.y <= 530 + 155) {
				game.Scheck = click;
				game.GamePlay = StageSelect;
			}
		}
		break;
	}
	case WM_MOUSEMOVE: {
		if (game.GamePlay == Start || game.GamePlay == StageSelect || game.GamePlay == StageStop || game.GamePlay == StageClear || game.GamePlay == CustomMode) {
			MouseLC.x = LOWORD(lParam);
			MouseLC.y = HIWORD(lParam);
		}
		break;
	}
	case WM_LBUTTONUP: {
		if (drag == true)
			drag = false;
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}