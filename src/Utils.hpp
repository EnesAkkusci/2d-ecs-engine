#pragma once
#include <SDL.h>

enum Tag {
	Player,
	Enemy,
	Obstacle,
	Score,
	Death,
	PlayerInteractionCast,
	Collectible,
	Scientist,
	PasswordManager,
	Lever,
	Bridge,
	Battery
};

enum Collider {
	Box,
	Circle,
};

const unsigned int VECTOR_INIT = 300;
const unsigned int MAX_COMPONENTS = 32;
const unsigned int MAX_TAGS = 32;
const unsigned int MAX_MOUSE_BUTTONS = 8;

const int MAX_FPS = 20000;
const int MIN_MS_PER_FRAME = 1000 / MAX_FPS;

const int SCALE_FACTOR_32 = 2;
const int SCALE_FACTOR_16 = 10;

const int PLAYER_INTERACTION_WIDTH = 90;
const int PLAYER_INTERACTION_HEIGHT = PLAYER_INTERACTION_WIDTH;
