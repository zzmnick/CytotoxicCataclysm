#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

#include <random>


// These are hard coded to the dimensions of the entity texture
const vec2 IMMUNITY_TEXTURE_SIZE = { 30.f, 30.f };
const vec2 RED_ENEMY_TEXTURE_SIZE = { 41.f, 40.f };
const vec2 GREEN_ENEMY_TEXTURE_SIZE = { 40.f, 40.f };
const vec2 YELLOW_ENEMY_TEXTURE_SIZE = { 63.f, 60.f };
const vec2 HEALTHBAR_TEXTURE_SIZE = { 530.f, 80.f };
const vec2 BACTERIOPHAGE_TEXTURE_SIZE = { 750.f, 900.f };
const vec2 DIALOG_TEXTURE_SIZE = { 1920.f, 1080.f };
const vec2 CYST_TEXTURE_SIZE = { 23.f, -22.f };
const vec2 SWORD_SIZE = { 500.f, 500.f };
const vec2 FRIEND_TEXTURE_SIZE = { 30.f, 30.f };



const float IMMUNITY_TEXTURE_ANGLE = M_PI / 4;

const vec2 STATUSBAR_SCALE = { 1.f, 1.f };

/*************************[ characters ]*************************/
// the player
Entity createPlayer(vec2 pos);
Entity createDashing(Entity& playerEntity);
Entity createSword(RenderSystem* renderer, Entity& playerEntity);
Entity createGun(Entity player);
// enemies
Entity createRedEnemy(vec2 pos, float health = 100.0);
Entity createGreenEnemy(vec2 pos, float health = 200.0);
Entity createYellowEnemy(vec2 pos, float health = 50.0);
Entity createBossClone(vec2 pos, float health = 10.0);
Entity createBoss(RenderSystem* renderer, vec2 pos, float health = 1000.0);
Entity createSecondBoss(RenderSystem* renderer, vec2 pos, float health = 250.0);


/*************************[ environment ]*************************/
// the random regions
void createRandomRegions(size_t num_regions, std::default_random_engine& rng);
void createRandomCysts(std::default_random_engine& rng);
void createCyst(vec2 pos, float health = 50.0);
Entity createBullet(Entity shooter, vec2 scale, vec4 color);

/*************************[ UI ]*************************/
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 scale);
// create healthbar and its frame. Returns healthbar entity
Entity createHealthbar(vec2 position, vec2 scale);

/*************************[ other ]*************************/
Entity createCamera(vec2 pos);


/*************************[ load ]*************************/
void loadRegions(const json& regionsData);
void loadEnemies(const json& enemiesData);
