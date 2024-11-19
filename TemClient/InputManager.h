#pragma once
#include"client_pch.h"

#define INPUT INSTANCE(CInputManager)

CRITICAL_SECTION keyEventCS{};

class CInputManager
{
	MAKE_SINGLETON(CInputManager)

private:
	HWND mHwnd{};
	std::array<KEY_STATE, KEY_TYPE_COUNT> mStates{};

	POINT mMousePos{};

public:
	void Initialize(HWND hwnd);
	void Update();

	bool IsKeyPress(KEY_TYPE key) { return GetState(key) == KEY_STATE::PRESS; }
	bool IsKeyDown(KEY_TYPE key) { return GetState(key) == KEY_STATE::DOWN; }
	bool IsKeyUp(KEY_TYPE key) { return GetState(key) == KEY_STATE::UP; }

	POINT GetMousePosition() const { return mMousePos; }

private:
	inline KEY_STATE GetState(KEY_TYPE key) { return mStates[static_cast<UINT>(key)]; }
};

