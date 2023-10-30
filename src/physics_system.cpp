// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Transform& transform)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(transform.scale.x), abs(transform.scale.y) };
}

struct CollisionCircle {
	vec2 position;
	float radius;
	CollisionCircle(vec2 position, float radius) : 
		position(position), 
		radius(radius) 
	{}
};

// A brief explanation of the collision detection algorithm used in functions below:
// For a textured moving object, try to fit some number of circles within its bounding
// box. These circles together represent the collision region of the entity. if any
// collision circle of one entity collides with any collision circle of another entity,
// then these two entities collide. This is a more accurate approximation when the
// entity's bounding box is not square
std::vector<CollisionCircle> get_collision_circles(const Transform& transform)
{
	std::vector<CollisionCircle> res;
	vec2 bounding_box = get_bounding_box(transform);
	// If the bounding box is square, use a single circle
	if (abs(bounding_box.x - bounding_box.y) < 0.0001f) {
		res.push_back(CollisionCircle(transform.position, bounding_box.x / 2));
	} else {
		// Otherwise partition the rectangle into multiple circles
		float shorter_edge = min(bounding_box.x, bounding_box.y); 
		float longer_edge = max(bounding_box.x, bounding_box.y);
		Transformation transformation;
		transformation.rotate((bounding_box.x < bounding_box.y) ? (M_PI / 2 + transform.angle) : transform.angle);
		// Start from one side of the rectangle and go along the longer edge
		float circle_pos = -(longer_edge / 2.f - shorter_edge / 2.f);
		vec2 pos_offset;
		while (circle_pos < longer_edge / 2.f - shorter_edge / 2.f) {
			pos_offset = vec2(circle_pos, 0);
			pos_offset = transformation.mat * vec3(pos_offset, 1.f);
			res.push_back(CollisionCircle(
				transform.position + vec2(pos_offset.x, pos_offset.y),
				shorter_edge / 2.f
			));
			circle_pos += shorter_edge / 4.f;
		}
		pos_offset = vec2(longer_edge / 2.f - shorter_edge / 2.f, 0);
		pos_offset = transformation.mat * vec3(pos_offset, 1.f);
		res.push_back(CollisionCircle(
			transform.position + vec2(pos_offset.x, pos_offset.y),
			shorter_edge / 2.f
		));
	}
	return res;
}

bool collides(const Transform& transform1, const Transform& transform2)
{
	std::vector<CollisionCircle> circles1 = get_collision_circles(transform1);
	std::vector<CollisionCircle> circles2 = get_collision_circles(transform2);
	for (CollisionCircle circle1 : circles1) {
		for (CollisionCircle circle2: circles2) {
			float distance = length(circle1.position - circle2.position);
			if (distance < circle1.radius + circle2.radius) {
				return true;
			}
		}
	}
	return false;
}

bool collides_with_boundary(const Transform& transform)
{
	std::vector<CollisionCircle> collision_circles = get_collision_circles(transform);
	for (CollisionCircle circle : collision_circles) {
		if (length(circle.position) > MAP_RADIUS - circle.radius) {
			return true;
		}
	}
	return false;
}

// Adds collision events to be handled in world_system's resolve_collisions()
void collisionhelper(Entity entity_1, Entity entity_2) {
	// Bullet Collisions
	if (registry.weapons.has(entity_1)) {
		if (registry.weapons.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::BULLET_WITH_BULLET, entity_2);
		} else if (registry.players.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::BULLET_WITH_PLAYER, entity_2);
		} else if (registry.enemies.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::BULLET_WITH_ENEMY, entity_2);
		}
	// Player Collisions
	} else if (registry.players.has(entity_1)) {
		if (registry.enemies.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::PLAYER_WITH_ENEMY, entity_2);
		}
	// Enemy Collisions
	} else if (registry.enemies.has(entity_1)) {
		if (registry.enemies.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::ENEMY_WITH_ENEMY, entity_2);
		}
	}
}

// Step movement for all entities with Motion component
void step_movement(float elapsed_ms) {
	// Move NPC based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_container = registry.motions;
	for (uint i = 0; i < motion_container.size(); i++)
	{
		Entity entity = motion_container.entities[i];
		assert(registry.transforms.has(entity));
		Transform& transform = registry.transforms.get(entity);
		Motion& motion = motion_container.components[i];
		float step_seconds = elapsed_ms / 1000.f;
		transform.position += motion.velocity * step_seconds;

		if (registry.animations.has(entity) && registry.players.has(entity)) {
			bool speed_above_threshold = (abs(length(motion.velocity)) - play_animation_threshold) > 0.0f;
			Animation& animation = registry.animations.get(entity);
			if (!speed_above_threshold && animation.total_frame != (int)ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING) {
				RenderSystem::animationSys_switchAnimation(entity, ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING, animation_geo_map_general, animation_texture_map_general, 120);
			}
			else if (speed_above_threshold && animation.total_frame != (int)ANIMATION_FRAME_COUNT::IMMUNITY_MOVING && animation.total_frame != (int)ANIMATION_FRAME_COUNT::IMMUNITY_DYING) {
				RenderSystem::animationSys_switchAnimation(entity, ANIMATION_FRAME_COUNT::IMMUNITY_MOVING, animation_geo_map_general, animation_texture_map_general, 30);
			}
		}
		
	}
}

// Check collision for all entities with Motion component
void check_collision() {
	auto& motion_container = registry.motions;
	// Check for collisions between all moving entities
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Entity entity_i = motion_container.entities[i];
		assert(registry.transforms.has(entity_i));
		Transform& transform_i = registry.transforms.get(entity_i);

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < motion_container.components.size(); j++)
		{
			Entity entity_j = motion_container.entities[j];
			assert(registry.transforms.has(entity_j));
			Transform& transform_j = registry.transforms.get(entity_j);
			if (collides(transform_i, transform_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				// If collision between player and enemy, always add the collision component under player entity
				collisionhelper(entity_i, entity_j);
				collisionhelper(entity_j, entity_i);
			}
		}

		// Check for collisions with the map boundary
		if (collides_with_boundary(transform_i)) {
			if (registry.weapons.has(entity_i)) {
				registry.collisions.emplace_with_duplicates(entity_i, COLLISION_TYPE::BULLET_WITH_BOUNDARY);
			}
			else {
				registry.collisions.emplace_with_duplicates(entity_i, COLLISION_TYPE::WITH_BOUNDARY);
			}
		}
	}
}

void PhysicsSystem::step(float elapsed_ms)
{
	step_movement(elapsed_ms);
	check_collision();
}
