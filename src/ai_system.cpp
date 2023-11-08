// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// update player entity
	auto players = registry.players.entities;
	if (!players.empty()) {
		player = players.back();
	}
	move_enemies(elapsed_ms);
	enemy_shoot(elapsed_ms);
}

void AISystem::move_enemies(float elapsed_ms) {
	for (Entity entity : registry.enemies.entities) {
		if (registry.motions.has(entity)) {
			assert(registry.transforms.has(entity));
			Transform& enemytransform = registry.transforms.get(entity);
			Motion& enemymotion = registry.motions.get(entity);
			if (enemymotion.allow_accel == false) {
				enemymotion.allow_accel = true;
				continue;
			}
			// Boss chases player when player is close. More sophisticated AI for boss will be added later
			Enemy& enemyAttribute = registry.enemies.get(entity);
			vec2 playerposition = registry.transforms.get(player).position;
			float angle;
			if (enemyAttribute.type == ENEMY_ID::BOSS) {
				float distance = length(playerposition - enemytransform.position);
				vec2 interest_point = registry.regions.components[0].interest_point;
				if (distance > CONTENT_WIDTH_PX / 2) {					
					angle = atan2(enemytransform.position.y - interest_point.y, enemytransform.position.x - interest_point.x);
				} else {
					angle = atan2(enemytransform.position.y - playerposition.y, enemytransform.position.x - playerposition.x);	
				}
				enemytransform.angle = angle + enemytransform.angle_offset;
				enemymotion.velocity.x += -cos(angle) * elapsed_ms * enemymotion.acceleration_unit;
				enemymotion.velocity.y += -sin(angle) * elapsed_ms * enemymotion.acceleration_unit;
				float interest_distance = length(enemytransform.position - interest_point);
				if (distance > CONTENT_WIDTH_PX / 2 && interest_distance < 50.f) {
					enemymotion.velocity = { 0, 0 };
				}
			} else {
				angle = atan2(enemytransform.position.y - playerposition.y, enemytransform.position.x - playerposition.x);
				enemymotion.velocity.x += -cos(angle) * elapsed_ms * enemymotion.acceleration_unit;
				enemymotion.velocity.y += -sin(angle) * elapsed_ms * enemymotion.acceleration_unit;
				enemytransform.angle = angle + enemytransform.angle_offset;
			}

			float magnitude = length(enemymotion.velocity);

			if (magnitude > enemymotion.max_velocity || magnitude < -enemymotion.max_velocity) {
				enemymotion.velocity *= (enemymotion.max_velocity / magnitude);
			}
		}
	}

}


void AISystem::enemy_shoot(float elapsed_ms) {
	for (Entity entity : registry.weapons.entities) {
		Weapon& enemyWeapon = registry.weapons.get(entity);
		vec2 playerposition = registry.transforms.get(player).position;
		vec2 enemyposition = registry.transforms.get(entity).position;
		float distance = length(playerposition - enemyposition);
		if (registry.enemies.has(entity) && distance<= CONTENT_WIDTH_PX/2) {
			if (enemyWeapon.attack_timer <= 0) {
				Enemy& enemy = registry.enemies.get(entity);
				if (enemy.type == ENEMY_ID::BOSS) {
					createBullet(entity, { 20.f, 20.f }, { 1.f, 0.58f, 0.f, 1.f });

				}
				else {
					createBullet(entity, { 10.f, 10.f }, { 0.f, .98f, 1.f, 1.f });
				}
				enemyWeapon.attack_timer = enemyWeapon.attack_delay;
			}
		}
		enemyWeapon.attack_timer = max(enemyWeapon.attack_timer - elapsed_ms, 0.f);
			

	}

}