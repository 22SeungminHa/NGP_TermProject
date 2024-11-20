#include "client_pch.h"
#include "InputManager.h"

void CInputManager::Initialize(HWND hwnd)
{
	mHwnd = hwnd;
	mStates.fill(KEY_STATE::NONE);
}

void CInputManager::Update()
{
	HWND hwnd = ::GetActiveWindow();
	if (mHwnd != hwnd)
	{
		mStates.fill(KEY_STATE::NONE);
		return;
	}

	BYTE asciiKeys[KEY_TYPE_COUNT]{};
	if (::GetKeyboardState(asciiKeys) == false) {
		return;
	}

	for (UINT16 key = 0; key < KEY_TYPE_COUNT; key++)
	{
		KEY_STATE& state = mStates[key];
		if (asciiKeys[key] & 0x80)
		{
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN) {
				state = KEY_STATE::PRESS;
			}
			else {
				state = KEY_STATE::DOWN;
			}
		}
		else
		{
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN) {
				state = KEY_STATE::UP;
			}
			else {
				state = KEY_STATE::NONE;
			}
		}
	}

	GetCursorPos(&mMousePos);
}