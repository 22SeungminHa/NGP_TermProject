#pragma once

enum State {
	Normal, CantEat, ShortTelpo, Dash, HighJmp
};
enum Direction {
	dirRight, dirLeft, dirDown, dirUp
};
enum BlockType {
	JumpBk, RStraightBk, LStraightBk,
	RectBHBk, CircleBHBk, RectWHBk, CircleWHBk,
	BreakBk, ClimbBK, MusicBk,
	MvBkStopBk, MoveBk,
	Star,
	// 여기부터 Fuction에 안들어감
	BasicBk, LauncherBk, OnceMvBk, LightBk, Bullet, Item, SwitchBk, ElectricBk
};
enum Game {
	Start,

	StageSelect,
	StagePlay,
	StageStop,
	StageClear,
	StageDeath,

	SurvivalReady,
	SurvivalPlay,
	SurvivalStop,
	SurvivalDeath,

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