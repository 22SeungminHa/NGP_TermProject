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