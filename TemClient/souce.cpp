#include "stdafx.h"
#include "ClientManager.h"

// �ִ� vx = 21
// �ִ� vy = 40
// ax = 7
// x�� �ִ� �̵��Ÿ� = 180
// x�� �ּ� �̵��Ÿ� = 90
// �¿��������� vx = 60
// ��º��� vy = -65
// ������ ���� �� vy = 5 �� vy = 5.1

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
	const wchar_t* fontPath = L"���ɵ���ü+OTF.otf";
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
		// �̹��� �ε�
		{
			imgBall.Load(TEXT("�ٿ�� PNG/��.png"));
			imgBasicBlock.Load(TEXT("�ٿ�� PNG/�⺻����.png"));
			imgFuctionBlock.Load(TEXT("�ٿ�� PNG/��ɺ���.png"));
			imgSwitchBk.Load(TEXT("�ٿ�� PNG/���⽺��ġ����.png"));
			imgElectricBk.Load(TEXT("�ٿ�� PNG/�������.png"));

			imgStarAni.Load(TEXT("�ٿ�� PNG/�� ��������Ʈ.png"));
			imgDeadAni.Load(TEXT("�ٿ�� PNG/�� ��������Ʈ.png"));

			imgStartScreen.Load(TEXT("�ٿ�� PNG/����ȭ��.png"));
			imgStageScreen.Load(TEXT("�ٿ�� PNG/��������.png"));
			imgStopScreen.Load(TEXT("�ٿ�� PNG/�Ͻ�����.png"));
			imgClearScreen.Load(TEXT("�ٿ�� PNG/����Ŭ����.png"));
			imgPlayScreen.Load(TEXT("�ٿ�� PNG/�����÷��̹��.png"));
			imgMaptoolScreen.Load(TEXT("�ٿ�� PNG/����.png"));

			imgHomeButton.Load(TEXT("�ٿ�� PNG/Ȩ��ư.png"));
			imgResetButton.Load(TEXT("�ٿ�� PNG/����_����.png"));
			imgLoadButton.Load(TEXT("�ٿ�� PNG/����_�ҷ�����.png"));
			imgSaveButton.Load(TEXT("�ٿ�� PNG/����_����.png"));
			imgEraseButton.Load(TEXT("�ٿ�� PNG/����_���찳.png"));
			imgPlayButton.Load(TEXT("�ٿ�� PNG/����_�÷���.png"));

			imgBlockList.Load(TEXT("�ٿ�� PNG/����_���� ���� ���.png"));
			imgOutline.Load(TEXT("�ٿ�� PNG/����_���� ���� �׵θ�.png"));
		}
		// ���� �ε�
		{
			result = FMOD::System_Create(&ssystem); //--- ���� �ý��� ����
			if (result != FMOD_OK)
				exit(0);
			ssystem->init(32, FMOD_INIT_NORMAL, extradriverdata); //--- ���� �ý��� �ʱ�ȭ
			ssystem->createSound("Sound/ball.ogg", FMOD_LOOP_OFF, 0, &ballCrach_Sound); //--- �� Ƣ��� �Ҹ�
			ssystem->createSound("Sound/telpo.mp3", FMOD_LOOP_OFF, 0, &Telpo_Sound); //--- �� �����ϴ� �Ҹ� (����, ����Ȧ)
			ssystem->createSound("Sound/eatStar.mp3", FMOD_LOOP_OFF, 0, &EatStar_Sound); //--- �� ������ ���� �Ҹ�
			ssystem->createSound("Sound/balldeath.wav", FMOD_LOOP_OFF, 0, &ballDeath_Sound); //--- �� ������ ���� �Ҹ�
			ssystem->createSound("Sound/Click.mp3", FMOD_LOOP_OFF, 0, &Click_Sound); //--- Ŭ��
			ssystem->createSound("Sound/GameClear.mp3", FMOD_LOOP_OFF, 0, &GameClear_Sound); //--- ���� Ŭ����
			ssystem->createSound("Sound/musicbk.mp3", FMOD_LOOP_OFF, 0, &MusicBk_Sound); //--- ���� Ŭ����
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

		//���� ���� ��ġ
		if (game.GamePlay == CustomMode && drag == true && MouseLC.x >= 21 && MouseLC.x <= 21 + 1200 && MouseLC.y >= 21 && MouseLC.y <= 21 + 720) {
			if (selection > 0) {// ������ ���õǾ��� ���
				if ((MouseLC.x - 21) / 48 == BallStartLC.x && (MouseLC.y - 21) / 48 == BallStartLC.y) // ���� ���� ���
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
			else if (selection == 0) { // ���� ���õǾ��� ���
				if (game.Map[(MouseLC.y - 21) / 48][(MouseLC.x - 21) / 48]) // ������ ���� ���
					game.Map[(MouseLC.y - 21) / 48][(MouseLC.x - 21) / 48] = 0;
				BallStartLC = { (MouseLC.x - 21) / 48, (MouseLC.y - 21) / 48 };
			}
			else {// ���찳�� ���õǾ��� ���
				if ((MouseLC.x - 21) / 48 == BallStartLC.x && (MouseLC.y - 21) / 48 == BallStartLC.y) // �� �����
					BallStartLC = { -1, -1 };
				else
					game.Map[(MouseLC.y - 21) / 48][(MouseLC.x - 21) / 48] = 0; // ���� �����
			}
		}

		// ���� ���� ȭ��
		if (game.GamePlay == Start) {
			if (MouseLC.x <= 410 && MouseLC.y >= 593 && MouseLC.y <= 693)
				imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom); // �������� ��� �� Ŀ��
			else if (MouseLC.x <= 410 && MouseLC.y >= 717 && MouseLC.y <= 817)
				imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 3000, 0, game.window.right, game.window.bottom); // Ŀ���� ��� �� Ŀ��
			else
				imgStartScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // �⺻ ����ȭ��
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
				imgHomeButton.Draw(mdc, 1368, 48, 80, 80, 80, 0, 80, 80); // Ȩ��ư �� Ŀ��
			else
				imgHomeButton.Draw(mdc, 1368, 48, 80, 80, 0, 0, 80, 80); // �⺻ Ȩ��ư
		}
		else if (game.GamePlay == CustomMode) {
			imgMaptoolScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);

			//��ư
			if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 164 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78)
				imgPlayButton.Draw(mdc, 1239, 16, 164, 78, 164, 0, 164, 78); // �÷��̹�ư �� Ŀ��
			else
				imgPlayButton.Draw(mdc, 1239, 16, 164, 78, 0, 0, 164, 78); // �⺻ �÷��̹�ư
			if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78)
				imgSaveButton.Draw(mdc, 1410, 16, 78, 78, 78, 0, 78, 78); // �����ư �� Ŀ��
			else
				imgSaveButton.Draw(mdc, 1410, 16, 78, 78, 0, 0, 78, 78); // �⺻ �����ư
			if ((MouseLC.x >= 1239 && MouseLC.x <= 1239 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78))
				imgEraseButton.Draw(mdc, 1239, 105, 78, 78, 78, 0, 78, 78); // ���찳��ư �� Ŀ��
			else
				imgEraseButton.Draw(mdc, 1239, 105, 78, 78, 0, 0, 78, 78); // �⺻ ���찳��ư
			if (MouseLC.x >= 1325 && MouseLC.x <= 1325 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78)
				imgResetButton.Draw(mdc, 1325, 105, 78, 78, 78, 0, 78, 78); // ���¹�ư �� Ŀ��
			else
				imgResetButton.Draw(mdc, 1325, 105, 78, 78, 0, 0, 78, 78); // �⺻ ���¹�ư
			if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78)
				imgLoadButton.Draw(mdc, 1410, 105, 78, 78, 78, 0, 78, 78); // �ҷ������ư �� Ŀ��
			else
				imgLoadButton.Draw(mdc, 1410, 105, 78, 78, 0, 0, 78, 78); // �⺻ �ҷ������ư

			// ���� ���
			for (int i = 0; i < 28; i++) {
				if (i == selection)
					imgOutline.Draw(mdc, 17 + 60 * (i % 14) + 7 * (i % 14), 756 + 68 * (i / 14), 60, 60, 60, 0, 60, 60); // �����׵θ�
				else
					imgOutline.Draw(mdc, 17 + 60 * (i % 14) + 7 * (i % 14), 756 + 68 * (i / 14), 60, 60, 0, 0, 60, 60); // �����׵θ�
			}
			imgBlockList.Draw(mdc, 20, 759, 925, 122, 0, 0, 925, 122); // ����

			//��
			for (int i = 0; i < 15; i++) {
				for (int j = 0; j < 25; j++) {
					if (game.Map[i][j]) {
						if (game.list[game.Map[i][j] - 1].type < BasicBk) { // ��ɺ��� 
							imgFuctionBlock.Draw(mdc, 21 + j * 48, 21 + i * 48, 48, 48, game.list[game.Map[i][j] - 1].type * side, 0, side, side);
						}
						else {
							switch (game.list[game.Map[i][j] - 1].type) {
							case BasicBk: // �⺻����
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

			//��
			if (BallStartLC.x >= 0 && BallStartLC.y >= 0)
				imgBall.Draw(mdc, 21 + BallStartLC.x * 48 + 14, 21 + BallStartLC.y * 48 + 14, 20, 20, 0, 0, rd * 2, rd * 2);

			//���õ� ��
			if (selection > 0) { // ����
				if (game.list[selection - 1].type < BasicBk) { // ��ɺ��� 
					imgFuctionBlock.Draw(mdc, 1315, 307, 80, 80, game.list[selection - 1].type * side, 0, side, side);
				}
				else {
					switch (game.list[selection - 1].type) {
					case BasicBk: // �⺻����
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
			else if (selection == 0) // ��
				imgBall.Draw(mdc, 1315 + 15, 307 + 15, 50, 50, 0, 0, rd * 2, rd * 2);
			else // ���찳
				imgEraseButton.Draw(mdc, 1315, 307, 78, 78, 0, 0, 78, 78);
		}

		// ���� �÷��� ȭ��
		else if (game.GamePlay == StagePlay || game.GamePlay == StageClear || game.GamePlay == StageStop || game.GamePlay == CustomPlay || game.GamePlay == StageDeath || game.GamePlay == CustomDeath) { // �׾��� ��ƼŬ �ִϸ��̼� ���� �� �׸����� �߰���
			imgPlayScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom);

			//���� ���
			for (int y = 0; y < 15; ++y) {
				for (int i = 0; i < game.block[y].size(); ++i) {
					if (game.block[y][i].type < BasicBk) { // ��ɺ��� 
						imgFuctionBlock.Draw(mdc, game.block[y][i].x * side, game.block[y][i].y * side, side, side, game.block[y][i].type * side, 0, side, side);
					}
					else {
						switch (game.block[y][i].type) {
						case BasicBk: // �⺻����
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

			//�� ���
			if (game.GamePlay != StageDeath && game.GamePlay != CustomDeath) { // ������ ��� ���ϰ�
				imgBall.AlphaBlend(mdc, game.ball.x - rd, game.ball.y - rd, rd * 2, rd * 2, 0, 0, rd * 2, rd * 2, 255, AC_SRC_OVER); // Ȱ��ȭ��
			}

			// ��ƼŬ ���
			for (int i = 0; i < game.animation.size(); i++) {
				switch (game.animation[i].type) {
				case StageDeath: // �̳� ���ļ� �� �̰ž�
					imgDeadAni.AlphaBlend(mdc, game.animation[i].x, game.animation[i].y, 180, 180, 180 * (game.animation[i].ani / 2), 180 * game.animation[i].subtype, 180, 180, 170, AC_SRC_OVER);
					break;
				case Star:
					imgStarAni.AlphaBlend(mdc, game.animation[i].x, game.animation[i].y, 180, 180, 180 * (game.animation[i].ani / 2), 180 * game.animation[i].subtype, 180, 180, 170, AC_SRC_OVER);
					break;
				}
				game.animation[i].ani++;
				if (game.animation[i].ani == 40) {
					if (game.animation[i].type == Star) { // �ִϸ��̼� ������ clear�� �ٲ�
						game.starcnt--;
						if (game.starcnt == 0) {
							if (game.GamePlay == StagePlay || game.GamePlay == StageDeath) {// �� �԰� �׾��� ���� Ŭ����ǰ�,,, ���ÿ� �Ͼ�� WM_TIMER������ ���ư��� �Ƹ� death�� ���� �ɰŶ� �������Ű����ѵ� ���׳��鹹,, �ƽ������
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

			// ȭ�� ���
			if (game.GamePlay == StageStop) {
				if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 284 && MouseLC.y <= 381)
					imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 1500, 0, game.window.right, game.window.bottom); // ����ȭ�� ��ư �� Ŀ��
				else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 397 && MouseLC.y <= 494)
					imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 3000, 0, game.window.right, game.window.bottom); // �������� ��ư �� Ŀ��
				else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 509 && MouseLC.y <= 606)
					imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 4500, 0, game.window.right, game.window.bottom); // ����� �� Ŀ��
				else
					imgStopScreen.Draw(mdc, 0, 0, game.window.right, game.window.bottom, 0, 0, game.window.right, game.window.bottom); // �⺻ ����ȭ��
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
		// ������
		if (game.GamePlay == StageDeath && game.animation.size() == 0) { // ������ �ִϸ��̼� ������ ��������
			game.MakeVector();
			game.ball = { (float)BallStartLC.x, (float)BallStartLC.y, 0, 0, 0 };
			game.GamePlay = StagePlay;
		}
		else if (game.GamePlay == CustomDeath && game.animation.size() == 0) {
			game.MakeVector();
			game.ball = { (float)BallStartLC.x * side + 30, (float)BallStartLC.y * side + 30, 0, 0, 0 };
			game.GamePlay = CustomPlay;
		}

		// �� ���� ȿ���� ���
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

		//����/�� �̵�, �浹üũ
		if (game.GamePlay == StagePlay || game.GamePlay == CustomPlay || game.GamePlay == StageDeath || game.GamePlay == CustomDeath) {
			//�� �̵�
			game.MoveBall();

			game.ballrc = { (float)game.ball.x - rd, (float)game.ball.y - rd, (float)game.ball.x + rd, (float)game.ball.y + rd };

			// �� �浹üũ
			if (game.GamePlay == StagePlay || game.GamePlay == CustomPlay) { // �̹� �������� �ȵ��ư��� �Ϸ���. ��� ���ư��� �ִϸ��̼� ���Ϳ� �ڲٵ�
				// �ٴڰ� �浹
				if (game.ball.y + rd >= game.window.bottom) {
					game.animation.emplace_back(Block{ (int)game.ball.x - 90, (int)game.ball.y - 90, StageDeath, rand() % 4, 0 });
					game.Scheck = balldeath;
					if (game.GamePlay == StagePlay)
						game.GamePlay = StageDeath;
					else if (game.GamePlay == CustomPlay)
						game.GamePlay = CustomDeath;
				}

				// ������ �浹
				else game.CrashExamin();
			}
		}

		InvalidateRect(hwnd, NULL, FALSE);
		break;
	}
	case WM_LBUTTONDOWN: {
		if (game.GamePlay == Start) {
			if (MouseLC.x <= 410 && MouseLC.y >= 593 && MouseLC.y <= 693) { // �������� ��ư
				game.Scheck = click;
				game.GamePlay = StageSelect;
			}
			else if (MouseLC.x <= 410 && MouseLC.y >= 717 && MouseLC.y <= 817) { // ���� ��ư
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
				game.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ������ ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage1.txt" };

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
				game.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ������ ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage2.txt" };

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
				game.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ������ ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage3.txt" };

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
				game.Scheck = click; // ���� ���⿡ Ŭ���ϸ� 1 2 3���� �ذ����� �������� ������ ���� �ҷ��ͼ� ���͹迭�� �־��ִ� �Լ� ¥�� ������ �� ��
				ifstream in{ "�ٿ�� ��/Stage4.txt" };

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
			if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 284 && MouseLC.y <= 381) { // ����ȭ�� ��ư �� Ŀ�� 
				game.Scheck = click;
				game.GamePlay = Start;
			}
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 397 && MouseLC.y <= 494) { // �������� ��ư �� Ŀ�� 
				game.Scheck = click;
				game.GamePlay = StageSelect;
			}
			else if (MouseLC.x >= 928 && MouseLC.x <= 1217 && MouseLC.y >= 509 && MouseLC.y <= 606) { // ����� ��ư �� Ŀ��
				game.Scheck = click;
				game.MakeVector();
				game.GamePlay = StageDeath;
				game.ball = { (float)BallStartLC.x, (float)BallStartLC.y, 0, 0, 0 }; // ����� �����ɷ� �ϸ� death�� �ٲ�� �ִϸ��̼� ������ �Ѿ�ߵż� �� �ٷ� ��������Ŵ
			}
		}
		else if (game.GamePlay == CustomMode) {
			drag = true;
			//���� ����
			if (MouseLC.y >= 756 && MouseLC.y <= 756 + 60) {
				game.Scheck = click;
				for (int i = 0; i < 14; i++) {
					if (MouseLC.x >= 17 + 60 * i + 7 * i && MouseLC.x <= 17 + 60 * i + 7 * i + 60)
						selection = i;
				}
			}
			else if (MouseLC.y >= 756 + 60 + 7 && MouseLC.y <= 756 + 60 + 7 + 60) {
				game.Scheck = click;
				for (int i = 0; i < 14; i++) {
					if (MouseLC.x >= 17 + 60 * i + 7 * i && MouseLC.x <= 17 + 60 * i + 7 * i + 60)
						selection = i + 14;
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
				game.MakeVector();
			}
			// ���찳 ��ư
			else if (MouseLC.x >= 1239 && MouseLC.x <= 1239 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				selection = -1;
			}
			// ���� ��ư
			else if (MouseLC.x >= 1325 && MouseLC.x <= 1325 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				memset(game.Map, 0, sizeof(game.Map));
				BallStartLC = { -1, -1 };
			}
			// �ҷ����� ��ư
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 105 && MouseLC.y <= 105 + 78) {
				game.Scheck = click;
				memset(&OFN, 0, sizeof(OPENFILENAME)); //--- ����ü �ʱ�ȭ
				OFN.lStructSize = sizeof(OPENFILENAME);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = filter;
				OFN.lpstrFile = lpstrFile;
				OFN.nMaxFile = 256;
				OFN.lpstrInitialDir = L".";

				if (GetOpenFileNameW(&OFN) != 0) { //--- ���� �Լ� ȣ��
					TCHAR a[100];
					wsprintf(a, L"%s ������ ���ðڽ��ϱ� ?", OFN.lpstrFile);
					MessageBox(hwnd, a, L"���� ����", MB_OK);

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
			// ���� ��ư
			else if (MouseLC.x >= 1410 && MouseLC.x <= 1410 + 78 && MouseLC.y >= 16 && MouseLC.y <= 16 + 78) {
				game.Scheck = click;
				memset(&OFN, 0, sizeof(OPENFILENAME)); //--- ����ü �ʱ�ȭ
				OFN.lStructSize = sizeof(OPENFILENAME);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = filter;
				OFN.lpstrFile = lpstrFile;
				OFN.nMaxFile = 256;
				OFN.lpstrInitialDir = L".";

				if (GetSaveFileNameW(&OFN) != 0) { //--- ���� �Լ� ȣ��
					TCHAR a[100];
					wsprintf(a, L"%s ��ġ�� ������ �����Ͻðڽ��ϱ� ?", OFN.lpstrFile);
					MessageBox(hwnd, a, L"�����ϱ� ����", MB_OK);
					TCHAR b[100];
					wsprintf(b, L"%s.txt", OFN.lpstrFile);

					ofstream out{ b };


					// �����迭 ����
					for (int y = 0; y < 15; ++y) {
						for (int x = 0; x < 25; ++x) {
							out << game.Map[y][x] << " ";
						}
						out << endl;
					}
					// �� ������ġ, ���� ���� ����
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