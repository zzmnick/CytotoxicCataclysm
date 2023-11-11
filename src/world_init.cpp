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
	Weapon& weapon = registry.weapons.emplace(entity);
	weapon.angle_offset = 0.7f;
	weapon.offset = { 60.f,60.f };
	registry.collideEnemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::IMMUNITY_BLINKING,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_BLINKING,
			RENDER_ORDER::OBJECTS_FR });
	Animation& animation = registry.animations.emplace(entity);
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING;
	animation.update_period_ms = 120;

	// Create a brand new health with 100 health value
	registry.healthValues.emplace(entity);

	// Add color for player
	registry.colors.insert(entity, { 1.f,1.f,1.f,1.f });
	createDashing(entity);
	return entity;
}

Entity createDashing(Entity& playerEntity) {
	auto placeHolder = Entity();
	PlayerBelonging& pb = registry.playerBelongings.emplace(placeHolder);
	pb.id = PLAYER_BELONGING_ID::DASHING;
	registry.renderRequests.insert(
		placeHolder,
		{ TEXTURE_ASSET_ID::DASHING,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_DASHING,
			RENDER_ORDER::OBJECTS_BK });

	Animation& animation = registry.animations.emplace(placeHolder);
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::DASHING;
	animation.update_period_ms = 50;

	//same as player's
	Motion& playerMotion_copy = registry.motions.get(playerEntity);
	registry.motions.insert(placeHolder, playerMotion_copy);

	Transform& playerTrans_copy = registry.transforms.get(playerEntity);
	registry.transforms.insert(placeHolder, playerTrans_copy);

	vec4& color = registry.colors.emplace(placeHolder);
	color = dashing_default_color;

	return placeHolder;
}

Entity createBoss(RenderSystem* renderer, vec2 pos) {
	// Create boss components
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BACTERIOPHAGE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	Health& health = registry.healthValues.emplace(entity);

	// Setting initial components values
	new_enemy.type = ENEMY_ID::BOSS;

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.angle = M_PI;
	transform.scale = BACTERIOPHAGE_TEXTURE_SIZE * 0.8f;
	motion.max_velocity = 250.f; // TODO: Dummy boss for now, change this later
	health.health = 500.f;
	transform.angle_offset = M_PI / 2;

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
	new_enemy.type = ENEMY_ID::RED;

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.angle = M_PI;
	transform.scale = RED_ENEMY_TEXTURE_SIZE * 2.f;

	Motion& motion = registry.motions.emplace(entity);
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
	new_enemy.type = ENEMY_ID::GREEN;

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.angle = M_PI;
	transform.scale = GREEN_ENEMY_TEXTURE_SIZE * 4.f;
	transform.angle_offset = M_PI + 0.8;

	Motion& motion = registry.motions.emplace(entity);
	motion.max_velocity = 200;
	health.health = 200.0;

	// Add to render_request
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

	registry.noRotates.emplace(entity);
	registry.collidePlayers.emplace(entity);

	Weapon& weapon = registry.weapons.emplace(entity);
	weapon.attack_delay = 900.f;

	Enemy& new_enemy = registry.enemies.emplace(entity);
	new_enemy.type = ENEMY_ID::YELLOW;

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.scale = YELLOW_ENEMY_TEXTURE_SIZE * 1.5f;
	transform.angle_offset = M_PI;

	Motion& motion = registry.motions.emplace(entity);
	motion.max_velocity = 0.0f;

	Health& health = registry.healthValues.emplace(entity);
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

void createRandomRegions(size_t num_regions, std::default_random_engine& rng){
	assert(region_theme_count >= num_regions);
	assert(region_goal_count >= num_regions);

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
		float interest_distance = MAP_RADIUS * 0.6;
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

void createRandomCysts(std::default_random_engine& rng) {
	const float ANGLE = (M_PI * 2 / NUM_REGIONS);
	const int TOTAL_CYSTS = 100;

	const float LOWER_RADIUS = 0.18f * MAP_RADIUS;
	const float UPPER_RADIUS = 0.98f * MAP_RADIUS;
	const float MEAN_POINT   = 0.27f * MAP_RADIUS; // actual mean is approx = max_radius - mean_point

	std::exponential_distribution<float> radius_distribution(1.f / MEAN_POINT);
	std::uniform_real_distribution<float> angle_distribution(0.f, ANGLE);

	std::vector<vec2> positions;
	// generate cysts
	for (int i = 0; i < registry.regions.components.size(); i++) {
		for (int j = 0; j < TOTAL_CYSTS / registry.regions.components.size(); j++) {
			// generate radius in the correct bounds
			float radius;
			do {
				radius = MAP_RADIUS - radius_distribution(rng);
			} while (radius > UPPER_RADIUS || radius < LOWER_RADIUS);

			// generate angle
			float angle = angle_distribution(rng) + ANGLE * i;

			// calculate position and check closeness
			vec2 pos = vec2(cos(angle), sin(angle)) * radius;
			bool tooClose = false;
			for (auto p : positions) {
				if (distance(pos, p) < SCREEN_RADIUS / 2) {
					tooClose = true;
					break;
				}
			}
			if (tooClose) {
				--j;
				continue;
			}

			// finally create cyst
			createCyst(pos);
			positions.push_back(pos);
		}
	}
}

void createCyst(vec2 pos) {
	auto cyst_entity = Entity();
	registry.cysts.emplace(cyst_entity);
	registry.healthValues.insert(cyst_entity, {50.f});

	// Motion component only needed for collision check, set all to 0
	Motion& motion = registry.motions.emplace(cyst_entity);
	motion.velocity = { 0.f, 0.f };
	motion.max_velocity = 0.f;
	motion.acceleration_unit = 0.f;
	motion.deceleration_unit = 0.f;
	motion.allow_accel = false;

	Transform& transform = registry.transforms.emplace(cyst_entity);
	transform.position = pos;
	transform.scale = CYST_TEXTURE_SIZE * 3.f;

	Animation& animation = registry.animations.emplace(cyst_entity);
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::CYST_SHINE;
	animation.update_period_ms = 80;
	animation.loop_interval = 1500.f;

	registry.renderRequests.insert(
		cyst_entity,
		{ TEXTURE_ASSET_ID::CYST_SHINE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_CYST_SHINE,
			RENDER_ORDER::OBJECTS_FR });
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

	registry.healthbar.emplace(bar);

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

	// Create transform for bar and frame
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
	// Create bullet's components
	auto bullet_entity = Entity();
	registry.projectiles.emplace(bullet_entity);
	Transform& bullet_transform = registry.transforms.emplace(bullet_entity);
	Motion& bullet_motion = registry.motions.emplace(bullet_entity);
	registry.colors.insert(bullet_entity, color);

	// Set initial position and velocity
	Transform& shooter_transform = registry.transforms.get(shooter);
	Weapon& weapon = registry.weapons.get(shooter);
	bullet_transform.position = {
		shooter_transform.position.x + weapon.offset.x * cos(shooter_transform.angle),
		shooter_transform.position.y + weapon.offset.y * sin(shooter_transform.angle)
	};
	bullet_transform.scale = scale;
	bullet_transform.angle = 0.0;
	bullet_motion.velocity = { cos(shooter_transform.angle - weapon.angle_offset) * weapon.bullet_speed, sin(shooter_transform.angle - weapon.angle_offset) * weapon.bullet_speed };

	if (registry.collideEnemies.has(shooter)) {
		registry.collideEnemies.emplace(bullet_entity);
	}
	if (registry.collidePlayers.has(shooter)) {
		registry.collidePlayers.emplace(bullet_entity);
	}

	// Add bullet to render request
	registry.renderRequests.insert(
		bullet_entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::BULLET,
			RENDER_ORDER::OBJECTS_BK });

	return bullet_entity;
}

Entity createCamera(vec2 pos) {
	Entity camera = Entity();
	registry.camera.insert(camera, { pos });
	return camera;
}
