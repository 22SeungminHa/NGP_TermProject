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
	// ������� Fuction�� �ȵ�
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