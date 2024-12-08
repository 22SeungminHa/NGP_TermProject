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
	StageWaiting,
	StageDeath,

	CustomMode,
	CustomSelect,
	CustomSelect2,
	CustomPlay,
	CustomDeath
};

enum SoundCheck {
	X, ballcrach, telpo, eatstar, balldeath, click, gameclear, music
};

enum class KEY_TYPE
{
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,
	ESCAPE = VK_ESCAPE,
	SPACE = VK_SPACE,
	L     = 'L',

	LBUTTON = VK_LBUTTON,
	RBUTTON = VK_RBUTTON
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
	KEY_TYPE_COUNT = static_cast<UINT>(UINT8_MAX + 1)
};