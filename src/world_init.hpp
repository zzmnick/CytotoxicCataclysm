#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hard coded to the dimensions of the entity texture
const vec2 IMMUNITY_TEXTURE_SIZE = { 30.f, 30.f };
const vec2 RED_ENEMY_TEXTURE_SIZE = { 41.f, 40.f };
const vec2 GREEN_ENEMY_TEXTURE_SIZE = { 40.f, 40.f };
const vec2 YELLOW_ENEMY_TEXTURE_SIZE = { 63.f, 60.f };
const vec2 HEALTHBAR_TEXTURE_SIZE = { 530.f, 80.f };
const vec2 BACTERIOPHAGE_TEXTURE_SIZE = { 750.f, 900.f };
const vec2 DIALOG_TEXTURE_SIZE = { 1920.f, 1080.f };

const vec2 STATUSBAR_SCALE = { 1.f, 1.f };

/*************************[ characters ]*************************/
// the player
Entity createPlayer(vec2 pos);
// enemies
Entity createRedEnemy(vec2 pos);
Entity createGreenEnemy(vec2 pos);
Entity createYellowEnemy(vec2 pos);
Entity createBoss(RenderSystem* renderer, vec2 pos);

/*************************[ environment ]*************************/
// the random regions
void createRandomRegions(size_t num_regions);
Entity createBullet(Entity shooter, vec2 scale, vec4 color,vec2 offset, float angleoffset);

/*************************[ UI ]*************************/
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 scale);
// create healthbar and its frame. Returns healthbar entity
Entity createHealthbar(vec2 position, vec2 scale);
