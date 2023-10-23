#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hard coded to the dimensions of the entity texture
const float IMMUNITY_BB_WIDTH = 3.0f * 30.f;
const float IMMUNITY_BB_HEIGHT = 3.0f * 30.f;
const float RED_ENEMY_BB_WIDTH = 2.0f * 41.f;
const float RED_ENEMY_BB_HEIGHT = 2.0f * 40.f;
const float GREEN_ENEMY_BB_WIDTH = 4.0f * 40.f;
const float GREEN_ENEMY_BB_HEIGHT = 4.0f * 40.f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the red_enemy
Entity createRedEnemy(RenderSystem* renderer, vec2 pos);

Entity createGreenEnemy(RenderSystem* renderer, vec2 pos);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// the random regions
void createRandomRegion(RenderSystem* renderer, size_t num_regions);

Entity createBullet(vec2 pos, vec2 size);

