// internal
#include "world_init.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// stlib
#include <vector>
#include <random>
#include <algorithm>

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ IMMUNITY_BB_WIDTH, IMMUNITY_BB_HEIGHT });

	// Create an (empty) Player component to be able to refer to all players
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::IMMUNITY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createRedEnemy(RenderSystem* renderer, vec2 pos) {

    auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 3.14f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ RED_ENEMY_BB_WIDTH, RED_ENEMY_BB_HEIGHT });

	// Create an enemy
	registry.enemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::RED_ENEMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

    return entity;
}

void createRandomRegion(RenderSystem* renderer, size_t num_regions) {
	assert(region_theme_count >= num_regions);
	assert(region_goal_count >= num_regions);

	// C++ random number generator
	std::default_random_engine rng = std::default_random_engine(std::random_device()());

	std::vector<REGION_THEME_ID> unused_themes;
	for (uint i = 0; i < region_theme_count; i++) {
		unused_themes.push_back(static_cast<REGION_THEME_ID>(i));
	}
	std::shuffle(unused_themes.begin(), unused_themes.end(), rng);
	std::vector<REGION_GOAL_ID> unused_goals;
	for (uint i = 0; i < region_goal_count; i++) {
		unused_goals.push_back(static_cast<REGION_GOAL_ID>(i));
	}
	std::shuffle(unused_goals.begin(), unused_goals.end(), rng);
	std::vector<BOSS_ID> unused_bosses;
	for (uint i = 0; i < boss_type_count; i++) {
		unused_bosses.push_back(static_cast<BOSS_ID>(i));
	}
	std::shuffle(unused_bosses.begin(), unused_bosses.end(), rng);

	float angle = 0.f;

	for (int i = 0; i < num_regions; i++) {
		auto entity = Entity();
		// Store a reference to the potentially re-used mesh object
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::REGION_TRIANGLE);
		registry.meshPtrs.emplace(entity, &mesh);

		Region& region = registry.regions.emplace(entity);
		region.angle = angle;
		
		// Set region unique theme
		region.theme = unused_themes.back();
		unused_themes.pop_back();
		// Set region unique goal
		region.goal = unused_goals.back();
		unused_goals.pop_back();
		// Set region boss (if needs a boss)
		if (region.goal == REGION_GOAL_ID::CURE || region.goal == REGION_GOAL_ID::CANCER_CELL) {
			region.boss = unused_bosses.back();
			unused_bosses.pop_back();
		}
		// Set region regular enemy (non-unique)
		std::uniform_int_distribution<int> int_dist (0, enemy_type_count - 1);
		ENEMY_ID enemy = static_cast<ENEMY_ID>(int_dist(rng));
		region.enemy = enemy;
		
		// Add region to renderRequests
		registry.renderRequests.insert(
			entity,
			{ region_texture_map[region.theme],
				EFFECT_ASSET_ID::REGION,
				GEOMETRY_BUFFER_ID::REGION_TRIANGLE });

		// Update angle
		angle += (M_PI * 2 / num_regions);
	}
}

Entity createLine(vec2 position, vec2 scale) {
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}