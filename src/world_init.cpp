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

	// Setting initial bullet_transform values
	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.angle = 0.f;
	transform.scale = IMMUNITY_TEXTURE_SIZE * 3.f;
	registry.motions.insert(entity, { { 0.f, 0.f } });

	// Create an (empty) Player component to be able to refer to all players
	registry.players.emplace(entity);
	registry.dashes.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::IMMUNITY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::OBJECTS_FR });

	// Creat a brand new health with 100% health value
	registry.healthValues.emplace(entity);

	// Add color for player
	registry.colors.insert(entity, { 1.f,1.f,1.f,1.f });

	return entity;
}

Entity createRedEnemy(RenderSystem* renderer, vec2 pos) {
	// Create enemy components
	auto entity = Entity();
	registry.enemies.emplace(entity);
	Transform& transform = registry.transforms.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	Health& health = registry.healthValues.emplace(entity);
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial components values
	transform.position = pos;
	transform.angle = M_PI;
	transform.scale = RED_ENEMY_TEXTURE_SIZE * 2.f;
	motion.max_velocity = 400;
	health.previous_health_pct = 200.0;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::RED_ENEMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::OBJECTS_FR });

	return entity;
}

Entity createGreenEnemy(RenderSystem* renderer, vec2 pos) {
	// Create enemy components
	auto entity = Entity();
	registry.enemies.emplace(entity);
	Transform& transform = registry.transforms.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	Health& health = registry.healthValues.emplace(entity);
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial components values
	transform.position = pos;
	transform.angle = M_PI;
	transform.scale = GREEN_ENEMY_TEXTURE_SIZE * 4.f;
	motion.max_velocity = 200;
	health.previous_health_pct = 200.0;

	// Add tp render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GREEN_ENEMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::OBJECTS_FR });

	return entity;
}
void createRandomRegions(RenderSystem* renderer, size_t num_regions) {
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
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::REGION_TRIANGLE);
		registry.meshPtrs.emplace(entity, &mesh);
		Region& region = registry.regions.emplace(entity);

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
		std::uniform_int_distribution<int> int_dist(0, enemy_type_count - 1);
		ENEMY_ID enemy = static_cast<ENEMY_ID>(int_dist(rng));
		region.enemy = enemy;

		// Add region to renderRequests and transforms
		registry.renderRequests.insert(
			entity,
			{ region_texture_map[region.theme],
				EFFECT_ASSET_ID::REGION,
				GEOMETRY_BUFFER_ID::REGION_TRIANGLE,
				RENDER_ORDER::BACKGROUND }
		);
		registry.transforms.insert(
			entity,
			{ { 0.f, 0.f }, { MAP_RADIUS * 1.5f, MAP_RADIUS * 1.5f }, angle, false }
		);

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
			GEOMETRY_BUFFER_ID::DEBUG_LINE,
			RENDER_ORDER::UI });

	// Create bullet_transform
	Transform& transform = registry.transforms.emplace(entity);
	transform.angle = 0.f;
	transform.position = position;
	transform.scale = scale;
	transform.is_screen_coord = true;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createHealthbar(vec2 position, vec2 scale) {
	Entity bar = Entity();
	Entity frame = Entity();

	// Add the components to the renderRequest in order
	registry.renderRequests.insert(
		bar,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::STATUSBAR_RECTANGLE,
			RENDER_ORDER::UI });
	registry.renderRequests.insert(
		frame,
		{ TEXTURE_ASSET_ID::HEALTHBAR_FRAME,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI });

	// Create bullet_transform for bar and frame
	vec2 frameScale = HEALTHBAR_TEXTURE_SIZE * scale;
	registry.transforms.insert(frame, { position, frameScale, 0.f, true });
	vec2 barPosition = position - vec2(frameScale.x * 0.25, 0.f);
	vec2 barScale = frameScale * vec2(0.72, 0.18);
	registry.transforms.insert(bar, { barPosition, barScale, 0.f, true });

	// Create color for bar
	registry.colors.insert(bar, { 0.f,1.f,0.f,1.f });

	return bar;
}

// Can be used for either player or enemy
Entity createBullet(Entity shooter, vec2 scale, vec4 color) {
	assert(registry.transforms.has(shooter));
	Transform& shooter_transform = registry.transforms.get(shooter);

	// Create bullet's components
	auto bullet_entity = Entity();
	registry.weapons.emplace(bullet_entity);
	Transform& bullet_transform = registry.transforms.emplace(bullet_entity);
	Motion& bullet_motion = registry.motions.emplace(bullet_entity);
	registry.colors.insert(bullet_entity, color);

	// Set initial position and velocity
	bullet_transform.position = {
		shooter_transform.position.x + 60 * cos(shooter_transform.angle),
		shooter_transform.position.y + 60 * sin(shooter_transform.angle)
	};
	bullet_transform.scale = scale;
	bullet_transform.angle = 0.0;
	bullet_motion.velocity = { cos(shooter_transform.angle - 0.70) * 500, sin(shooter_transform.angle - 0.70) * 500 };

	// Add bullet to render request
	registry.renderRequests.insert(
		bullet_entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::BULLET,
			RENDER_ORDER::OBJECTS_BK });

	return bullet_entity;
}
