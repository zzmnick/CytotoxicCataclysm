#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hard coded to the dimensions of the entity texture
const vec2 IMMUNITY_TEXTURE_SIZE = { 30.f, 30.f };
const vec2 RED_ENEMY_TEXTURE_SIZE = { 41.f, 40.f };
const vec2 GREEN_ENEMY_TEXTURE_SIZE = { 40.f, 40.f };
const vec2 HEALTHBAR_TEXTURE_SIZE = { 530.f, 80.f };

const vec2 STATUSBAR_SCALE = { 1.f, 1.f };

/*************************[ characters ]*************************/
// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// enemies
Entity createRedEnemy(RenderSystem* renderer, vec2 pos);
Entity createGreenEnemy(RenderSystem* renderer, vec2 pos);

/*************************[ environment ]*************************/
// the random regions
void createRandomRegions(RenderSystem* renderer, size_t num_regions);
Entity createBullet(Entity shooter, vec2 scale, vec4 color);

/*************************[ UI ]*************************/
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 scale);
// create healthbar and its frame. Returns healthbar entity
Entity createHealthbar(vec2 position, vec2 scale);
