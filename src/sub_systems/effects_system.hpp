#pragma once
#include "tiny_ecs.hpp"
#include "components.hpp"
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class WorldSystem;

// Rendering consts
const vec2 ICON_SIZE = { 23.f, 23.f };
const float ICON_SCALE = 3.f;
const float PADDING = 12.f;
const vec2 EFFECTS_POSITION = { CONTENT_WIDTH_PX * 0.35, CONTENT_HEIGHT_PX * 0.45 };


// Enums and Maps
enum class EFFECT_TYPE {
	POSITIVE = 0,
	NEGATIVE = 1
};

struct Effect {
	CYST_EFFECT_ID id;
	EFFECT_TYPE type;
	bool is_active = false;
};

static std::unordered_map <CYST_EFFECT_ID, TEXTURE_ASSET_ID> effect_to_texture = {
	{CYST_EFFECT_ID::DAMAGE   , TEXTURE_ASSET_ID::ICON_DAMAGE},
	{CYST_EFFECT_ID::SLOW     , TEXTURE_ASSET_ID::ICON_SLOW},
	{CYST_EFFECT_ID::FOV      , TEXTURE_ASSET_ID::ICON_FOV},
	{CYST_EFFECT_ID::NO_ATTACK, TEXTURE_ASSET_ID::ICON_AMMO},
};

static std::unordered_map <CYST_EFFECT_ID, int> effect_to_position = {
	{CYST_EFFECT_ID::DAMAGE   , 0},
	{CYST_EFFECT_ID::SLOW     , 1},
	{CYST_EFFECT_ID::NO_ATTACK, 2},
	{CYST_EFFECT_ID::FOV      , 3},
};

// Pos/neg and effect probabilities
const float POS_PROB = 0.7f;

static std::unordered_map<CYST_EFFECT_ID, double> pos_effect_weights = {
	{CYST_EFFECT_ID::DAMAGE, 0.4},
	{CYST_EFFECT_ID::HEAL, 0.4},
	{CYST_EFFECT_ID::CLEAR_SCREEN, 0.2},
};

static std::unordered_map<CYST_EFFECT_ID, double> neg_effect_weights = {
	{CYST_EFFECT_ID::SLOW, 0.25},
	{CYST_EFFECT_ID::FOV, 0.25},
	{CYST_EFFECT_ID::DIRECTION, 0.25},
	{CYST_EFFECT_ID::NO_ATTACK, 0.25}
};

class EffectsSystem{
public:
	Entity player;

	EffectsSystem(Entity player, std::default_random_engine& rng, std::unordered_map<std::string, Mix_Chunk*> soundChunks, WorldSystem& ws)
		: player(player), rng(rng), soundChunks(soundChunks), ws(ws) {
		// keep this in-sync with CYST_EFFECT_ID in components.hpp
		effects.push_back({ CYST_EFFECT_ID::DAMAGE,       EFFECT_TYPE::POSITIVE});
		effects.push_back({ CYST_EFFECT_ID::HEAL,         EFFECT_TYPE::POSITIVE});
		effects.push_back({ CYST_EFFECT_ID::CLEAR_SCREEN, EFFECT_TYPE::POSITIVE});
		effects.push_back({ CYST_EFFECT_ID::SLOW,      EFFECT_TYPE::NEGATIVE});
		effects.push_back({ CYST_EFFECT_ID::FOV,       EFFECT_TYPE::NEGATIVE});
		effects.push_back({ CYST_EFFECT_ID::DIRECTION, EFFECT_TYPE::NEGATIVE, true}); // set as active == disabled
		effects.push_back({ CYST_EFFECT_ID::NO_ATTACK, EFFECT_TYPE::NEGATIVE});

		for (const auto& pair : pos_effect_weights) {
			posWeights.push_back(pair.second);
		}
		for (const auto& pair : neg_effect_weights) {
			negWeights.push_back(pair.second);
		}
	};
	void apply_random_effect();

	~EffectsSystem();

private:
	// damage buff consts
	const float DAMAGE_MULTIPLIER = 2.5f;
	const float BULLET_SPEED_MULTIPLIER = 0.6f;
	const float ATTACK_DELAY_MULTIPLIER = 1.f;
	const float BULLET_SIZE_MULTIPLIER = 2.f;
	const vec4 DAMAGE_BUFF_PROJECTILE_COLOR = { 0.2f,0.1f,0.1f,1.f };
	const float DAMAGE_EFFECT_TIME = 5500;

	// heal buff consts
	const float MAX_HEALTH = 100.f; // TODO assuming 100 is max

	// no attack debuff consts
	const float NO_ATTACK_TIME = 4000.f;

	WorldSystem& ws;

	std::default_random_engine& rng;

	std::vector<Effect> effects;
	std::vector<double> posWeights;
	std::vector<double> negWeights;

	std::unordered_map<std::string, Mix_Chunk*> soundChunks;

	/*************************[ positive effects ]*************************/
	void handle_damage_effect();
	void handle_heal_effect();
	void handle_clear_screen();
	/*************************[ negative effects ]*************************/
	void handle_slow_effect();
	void handle_fov_effect();
	void handle_direction_effect();
	void handle_no_attack_effect();
	/*************************[ helpers ]*************************/
	int countActivePositive();
	int countActiveNegative();
	Effect& getEffect(CYST_EFFECT_ID id);
	void playSound(CYST_EFFECT_ID id);
	void displayEffect(Entity effect, CYST_EFFECT_ID id);
	void setActiveTimer(CYST_EFFECT_ID id, float timer);
};
