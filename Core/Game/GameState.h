#pragma once

struct World;

struct GameState {
	void Enter(const World* world);
	void Exit();

	World* const getWorld() { return world; }
private:
	World* world;
};