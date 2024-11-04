#pragma once

enum Direction {
	dirRight, dirLeft, dirDown, dirUp
};
enum BlockType {
	JumpBk, RStraightBk, LStraightBk,
	RectBHBk, CircleBHBk, RectWHBk, CircleWHBk,
	ClimbBK, MusicBk,
	Star,
	// ������� Fuction�� �ȵ�
	BasicBk, LightBk, SwitchBk, ElectricBk
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
enum Key {
	Space, LeftDown, RightDown, LeftUp, RightUp, ESC
};