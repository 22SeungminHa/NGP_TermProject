#pragma once

enum Direction {
	dirRight, dirLeft, dirDown, dirUp
};
enum BlockType {
	JumpBk, RStraightBk, LStraightBk,
	RectBHBk, CircleBHBk, RectWHBk, CircleWHBk,
	ClimbBK, MusicBk,
	Star,
	// 여기부터 Fuction에 안들어감
	BasicBk, SwitchBk, ElectricBk
};
enum Game {
	Start,

	StageSelect,
	StagePlay,
	StageStop,
	StageClear,
	StageDeath,

	CustomMode,
	CustomPlay,
	CustomDeath
};
enum SoundCheck {
	X, ballcrach, telpo, eatstar, balldeath, click, gameclear, music
};

enum class KEY_TYPE
{
	UP = VK_UP,
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,
	SHIFT = VK_SHIFT,
	CTRL = VK_CONTROL,
	ESCAPE = VK_ESCAPE,
	SPACE = VK_SPACE,

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

enum KEY_STATE : char
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