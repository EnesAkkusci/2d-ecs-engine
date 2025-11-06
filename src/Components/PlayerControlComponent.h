#pragma once

enum PlayerState {
	IDLE_SOUTH,
	IDLE_EAST,
	IDLE_NORTH,
	IDLE_WEST,

	WALK_SOUTH,
	WALK_EAST,
	WALK_NORTH,
	WALK_WEST,

	NUM_PLAYER_STATE
};

struct PlayerControlComponent {
	float playerMoveSpeed;
	PlayerState state;

	PlayerControlComponent(
		float _moveSpeed = 0,
		PlayerState _state = IDLE_SOUTH
	) :
		playerMoveSpeed(_moveSpeed),
		state(_state)
	{}
};
