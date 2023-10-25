// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// update player entity
	auto players = registry.players.entities;
	if (!players.empty()) {
		player = players.back();
	}
	move_enemies();
}

void AISystem::move_enemies() {
	for (Entity entity : registry.enemies.entities) {
		if (registry.motions.has(entity)) {
			assert(registry.transforms.has(entity));
			Transform& enemytransform = registry.transforms.get(entity);
			Motion& enemymotion = registry.motions.get(entity);
			if (enemymotion.allow_accel == false) {
				enemymotion.allow_accel = true;
				continue;
			}
			vec2 playerposition = registry.transforms.get(player).position;
			float angle = atan2(enemytransform.position.y - playerposition.y, enemytransform.position.x - playerposition.x);
			enemymotion.velocity.x += -cos(angle) * enemymotion.max_velocity * enemymotion.acceleration_unit;
			enemymotion.velocity.y += -sin(angle) * enemymotion.max_velocity * enemymotion.acceleration_unit;
			enemytransform.angle = angle + M_PI + 0.8;

			float magnitude = length(enemymotion.velocity);

			if (magnitude > enemymotion.max_velocity || magnitude < -enemymotion.max_velocity) {
				enemymotion.velocity *= (enemymotion.max_velocity / magnitude);
			}
		}
	}

}