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
	enemy_dash(elapsed_ms);
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
			}
			else if (enemyAttribute.type == ENEMY_ID::FRIENDBOSS) {
				Dash& enemyDash = registry.dashes.get(entity);
				if (enemyDash.active_dash_ms > 0.f) {
					continue;
				}



				angle = atan2(enemytransform.position.y - playerposition.y, enemytransform.position.x - playerposition.x);
				enemytransform.angle = angle + enemytransform.angle_offset;
				enemymotion.velocity.x += -cos(angle+M_PI/2) * elapsed_ms * enemymotion.acceleration_unit;
				enemymotion.velocity.y += -sin(angle+M_PI/2) * elapsed_ms * enemymotion.acceleration_unit;



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
					createBullet(entity, enemyWeapon.size, enemyWeapon.color);


				}
				else if (enemy.type == ENEMY_ID::FRIENDBOSS) {
					float decision = (static_cast<float>(rand()) / RAND_MAX); //This generates num between 0 and 1
					if (decision <= 0.7f) {
						createBullet(entity, enemyWeapon.size, enemyWeapon.color);

					}
					else {
						enemy_special_attack(entity);
					}



				}
				else {
					createBullet(entity, { 13.f, 13.f }, { 0.718f, 1.f, 0.f, 1.f });
				}
				enemyWeapon.attack_timer = enemyWeapon.attack_delay;
			}
		}
		enemyWeapon.attack_timer = max(enemyWeapon.attack_timer - elapsed_ms, 0.f);

	}
}

void AISystem::enemy_dash(float elapsed_ms) {
	vec2 playerposition = registry.transforms.get(player).position;
	for (Entity entity : registry.dashes.entities) {
		Transform enemy = registry.transforms.get(entity);
		float distance = length(playerposition - enemy.position);
		if (registry.enemies.has(entity) && distance <= CONTENT_WIDTH_PX / 2) {
			Dash& enemyDash = registry.dashes.get(entity);
			if (enemyDash.timer_ms <= 0.f) {
				enemyDash.timer_ms = 700.f;
				enemyDash.active_dash_ms = 50.f;


			}

			if (enemyDash.active_dash_ms > 0.f) {
				vec2 dashDirection;
				if (distance > 300.f) {
					dashDirection = normalize(vec2(cos(enemy.angle), sin(enemy.angle)));
					
				}
				else {
					dashDirection = normalize(vec2(cos(enemy.angle - M_PI / 2), sin(enemy.angle - M_PI / 2)));


				}
				Motion& enemymotion = registry.motions.get(entity);
				
				enemymotion.velocity += enemyDash.max_dash_velocity * dashDirection;
			}

			enemyDash.timer_ms = max(enemyDash.timer_ms - elapsed_ms, 0.f);
			enemyDash.active_dash_ms = max(enemyDash.active_dash_ms - elapsed_ms, 0.f);
		}

	}

}


void AISystem::enemy_special_attack(Entity enemy) {
	Transform& enemytransform = registry.transforms.get(enemy);
	Transform& playertransform = registry.transforms.get(player);

	float decision = (static_cast<float>(rand()) / RAND_MAX); //This generates num between 0 and 1

	if (decision <= 0.9f){ //90% of the time the boss will scattershot
		spread_attack(enemy);

	}
	else {
		clone_attack(enemy, 5);
	}

}

void AISystem::spread_attack(Entity enemy) {
	Transform& enemytransform = registry.transforms.get(enemy);

	for (int i = 0; i < 6; i++) {
		enemytransform.angle += 1;
		createBullet(enemy, { 13.f, 13.f }, { 0.718f, 1.f, 0.f, 1.f });
	}

}

void AISystem::clone_attack(Entity enemy, int clones) {
	Transform& playertransform = registry.transforms.get(player);

	for (int i = 0; i < clones; i++) {
		playertransform.angle += 1;
		float xpos = playertransform.position.x + cos(playertransform.angle) * 700;
		float ypos = playertransform.position.y + sin(playertransform.angle) * 700;
		createBossClone({ xpos,ypos });

	}

}
