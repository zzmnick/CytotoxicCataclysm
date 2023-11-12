#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);

private:
	Entity player; // Keep reference to player entity
	void move_enemies(float elapsed_ms);
	void enemy_shoot(float elapsed_ms);
};
