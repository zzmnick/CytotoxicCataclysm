#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);

private:
	Entity player; // Keep reference to player entity
	void move_enemies();
};
