#pragma once
#include"client_pch.h"

#define INPUT INSTANCE(CInputManager)

enum class KEY_TYPE
{
	UP = VK_UP,
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,
	SHIFT = VK_SHIFT,
	CTRL = VK_CONTROL,
	ESCAPE = VK_ESCAPE,

	LBUTTON = VK_LBUTTON,
	RBUTTON = VK_RBUTTON,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	Q = 'Q',
	E = 'E',
	Z = 'Z',
	C = 'C',
};

enum class KEY_STATE
{
	NONE,
	PRESS,
	DOWN,
	UP,
	END
};

enum
{
	KEY_TYPE_COUNT = static_cast<UINT>(UINT8_MAX + 1),
	KEY_STATE_COUNT = static_cast<UINT>(KEY_STATE::END),
};

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

