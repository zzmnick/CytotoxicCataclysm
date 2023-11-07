// internal
#include "world_init.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// stlib
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

Entity createPlayer(vec2 pos)
{
	auto entity = Entity();

	// Setting initial bullet_transform values
	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.angle = 0.f;
	transform.scale = IMMUNITY_TEXTURE_SIZE * 3.f;
	transform.angle_offset = M_PI + 0.7f;
	registry.motions.insert(entity, { { 0.f, 0.f } });

	// Create an (empty) Player component to be able to refer to all players
	registry.players.emplace(entity);
	registry.dashes.emplace(entity);
	registry.weapons.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::IMMUNITY_BLINKING,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_BLINKING,
			RENDER_ORDER::OBJECTS_FR });
	Animation& animation = registry.animations.emplace(entity);
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING;
	animation.update_period_ms = 120;

	// Creat a brand new health with 100% health value
	registry.healthValues.emplace(entity);

	// Add color for player
	registry.colors.insert(entity, { 1.f,1.f,1.f,1.f });

	return entity;
}

Entity createBoss(RenderSystem* renderer, vec2 pos) {
	// Create boss components
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BACTERIOPHAGE);
	registry.meshPtrs.emplace(entity, &mesh);
	// Assuming boss is a type of enemy
	Enemy& new_enemy = registry.enemies.emplace(entity);
	Transform& transform = registry.transforms.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	registry.healthValues.emplace(entity);
	// Setting initial components values
	new_enemy.type = ENEMY_ID::BOSS;
	transform.position = pos;
	transform.angle = M_PI;
	transform.scale = BACTERIOPHAGE_TEXTURE_SIZE * 0.8f;
	motion.max_velocity = 250.f; // TODO: Dummy boss for now, change this later
	// Add to render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BACTERIOPHAGE,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::BACTERIOPHAGE,
		  RENDER_ORDER::BOSS });
	return entity;
}

Entity createRedEnemy(vec2 pos) {
	// Create enemy components
	auto entity = Entity();
	
	Enemy& new_enemy = registry.enemies.emplace(entity);
	

	Transform& transform = registry.transforms.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	Health& health = registry.healthValues.emplace(entity);

	// Setting initial components values
	new_enemy.type = ENEMY_ID::RED;
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

Entity createGreenEnemy(vec2 pos) {
	// Create enemy components
	auto entity = Entity();
	Enemy& new_enemy = registry.enemies.emplace(entity);
	
	Transform& transform = registry.transforms.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	Health& health = registry.healthValues.emplace(entity);

	// Setting initial components values
	new_enemy.type = ENEMY_ID::GREEN;
	transform.position = pos;
	transform.angle = M_PI;
	transform.scale = GREEN_ENEMY_TEXTURE_SIZE * 4.f;
	transform.angle_offset = M_PI + 0.8;
	motion.max_velocity = 200;
	health.previous_health_pct = 200.0;

	// Add tp render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GREEN_ENEMY_MOVING,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_GREEN_ENEMY_MOVING,
			RENDER_ORDER::OBJECTS_FR });
	Animation& animation = registry.animations.emplace(entity);
	animation.update_period_ms *= 2;
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::GREEN_ENEMY_MOVING;
	return entity;
}


Entity createYellowEnemy(vec2 pos) {
	// Create enemy components
	auto entity = Entity();
	Enemy& new_enemy = registry.enemies.emplace(entity);

	Transform& transform = registry.transforms.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	Health& health = registry.healthValues.emplace(entity);
	registry.weapons.emplace(entity);
	registry.noRotates.emplace(entity);

	// Setting initial components values
	new_enemy.type = ENEMY_ID::YELLOW;
	transform.position = pos;
	transform.scale = YELLOW_ENEMY_TEXTURE_SIZE * 1.5f;
	transform.angle_offset = M_PI;
	motion.max_velocity = 0.0f;
	health.previous_health_pct = 50.0;

	// Add tp render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::YELLOW_ENEMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::OBJECTS_FR });
	return entity;
}

void createRandomRegions(size_t num_regions) {
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

		// Calculate interest point for the region
		float interest_distance = MAP_RADIUS * 0.6;  // 75% of MAP_RADIUS
		float center_angle = angle + (M_PI * 2 / num_regions) / 2; // Center of the angle span for the region
		vec2 interest_point;
		interest_point.x = interest_distance * cos(center_angle);
		interest_point.y = interest_distance * sin(center_angle);
		
		// Store the interest point in the Region component
		region.interest_point = interest_point;

		std::cout << "Region " << i+1 << ": Interest Point (X, Y) = (" << interest_point.x << ", " << interest_point.y << ")\n";


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
Entity createBullet(Entity shooter, vec2 scale, vec4 color,vec2 offset, float angleoffset) {
	assert(registry.transforms.has(shooter));
	Transform& shooter_transform = registry.transforms.get(shooter);

	// Create bullet's components
	auto bullet_entity = Entity();
	registry.projectiles.emplace(bullet_entity);
	Transform& bullet_transform = registry.transforms.emplace(bullet_entity);
	Motion& bullet_motion = registry.motions.emplace(bullet_entity);
	registry.colors.insert(bullet_entity, color);

	// Set initial position and velocity
	bullet_transform.position = {
		shooter_transform.position.x + offset.x * cos(shooter_transform.angle),
		shooter_transform.position.y + offset.y * sin(shooter_transform.angle)
	};
	bullet_transform.scale = scale;
	bullet_transform.angle = 0.0;
	bullet_motion.velocity = { cos(shooter_transform.angle - angleoffset) * 500, sin(shooter_transform.angle - angleoffset) * 500 };

	// Add bullet to render request
	registry.renderRequests.insert(
		bullet_entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::BULLET,
			RENDER_ORDER::OBJECTS_BK });

	return bullet_entity;
}
