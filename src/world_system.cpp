// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "sub_systems/dialog_system.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include <unordered_map>
#include <iostream>

std::unordered_map < int, int > keys_pressed;
vec2 mouse;
float MAX_VELOCITY = 400;
const float SPAWN_RANGE = MAP_RADIUS *0.6f; // Example value; adjust as needed
const int MAX_RED_ENEMIES = 10; // Example value; adjust as needed
const int MAX_GREEN_ENEMIES = 5; // Example value; adjust as needed
const float SCREEN_RADIUS = sqrt(CONTENT_WIDTH_PX * CONTENT_WIDTH_PX + CONTENT_HEIGHT_PX * CONTENT_HEIGHT_PX) / 2.f; // Half of screen diagonal
const float ENEMY_SPAWN_PADDING = 50.f; // Padding to ensure off-screen spawn
float enemy_spawn_cooldown = 5.f;
const float INDIVIDUAL_SPAWN_INTERVAL = 1.0f;
std::vector<ENEMY_ID> enemyTypes = { ENEMY_ID::RED, ENEMY_ID::GREEN }; // Add more types as needed



// Create the world
WorldSystem::WorldSystem() {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
	allow_accel = true;
	enemyCounts[ENEMY_ID::RED] = 0;
    enemyCounts[ENEMY_ID::GREEN] = 0;
	maxEnemies[ENEMY_ID::RED] = MAX_RED_ENEMIES;
    maxEnemies[ENEMY_ID::GREEN] = MAX_GREEN_ENEMIES;
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	for (const auto& pair : soundChunks) {
		if (pair.second != nullptr) {
			Mix_FreeChunk(pair.second);
		}
	}
	for (const auto& pair : backgroundMusic) {
		if (pair.second != nullptr) {
			Mix_FreeMusic(pair.second);
		}
	}
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	if (USE_FULLSCREEN) {
		// Fullscreen window
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		window = glfwCreateWindow(CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX, "Cytotoxic Cataclysm", monitor, nullptr);

		// When fullscreen we attempt to set the video mode to use a refresh rate of TARGET_REFRESH_RATE
		glfwSetWindowMonitor(window, monitor, 0, 0, CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX, TARGET_REFRESH_RATE);
		int actual_refresh_rate = glfwGetVideoMode(monitor)->refreshRate;
		if (actual_refresh_rate != TARGET_REFRESH_RATE) {
			printf("Warning: target refresh rate, %d, does not match actual refresh rate, %d\n", TARGET_REFRESH_RATE, actual_refresh_rate);
		}
	}
	else {
		// Windowed mode
		window = glfwCreateWindow(CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX, "Cytotoxic Cataclysm", nullptr, nullptr);
	}

	// Create the main window (for rendering, keyboard, and mouse input)
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_callback = [](GLFWwindow* wnd, int button, int action, int mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(button, 0, action, mods); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	// TODO: For Voxel Revolution.wav must credit as below:
	/*
		"Voxel Revolution" Kevin MacLeod (incompetech.com)
		Licensed under Creative Commons: By Attribution 4.0 License
		http://creativecommons.org/licenses/by/4.0/
	*/

	backgroundMusic["main"] = Mix_LoadMUS(audio_path("music/Voxel Revolution.wav").c_str());
	soundChunks["player_hit"] = Mix_LoadWAV(audio_path("sound/sfx_sounds_damage1.wav").c_str());
	soundChunks["player_dash"] = Mix_LoadWAV(audio_path("sound/sfx_sound_nagger2.wav").c_str());
	soundChunks["player_death"] = Mix_LoadWAV(audio_path("sound/sfx_sounds_falling3.wav").c_str());
	soundChunks["player_shoot_1"] = Mix_LoadWAV(audio_path("sound/sfx_wpn_laser8.wav").c_str());
	soundChunks["enemy_hit"] = Mix_LoadWAV(audio_path("sound/sfx_movement_footsteps5.wav").c_str());
	soundChunks["enemy_death"] = Mix_LoadWAV(audio_path("sound/sfx_deathscream_android8.wav").c_str());

	// Check for failures
	for (const auto& pair : backgroundMusic) {
		if (pair.second == nullptr) {
			fprintf(stderr, "Failed to load music make sure the data directory is present");
			return nullptr;
		}
	}
	for (const auto& pair : soundChunks) {
		if (pair.second == nullptr) {
			fprintf(stderr, "Failed to load sound chunk \'%s\' make sure the data directory is present", pair.first.c_str());
			return nullptr;
		}
	}

	// Assign channels
	Mix_AllocateChannels(6);
	chunkToChannel["player_hit"] = 0;
	chunkToChannel["player_dash"] = 1;
	chunkToChannel["player_death"] = 2;
	chunkToChannel["player_shoot_1"] = 3;
	chunkToChannel["enemy_hit"] = 4;
	chunkToChannel["enemy_death"] = 5;

	// Adjust music and CHUNK volume in range [0,128]
	Mix_VolumeMusic(45);
	Mix_VolumeChunk(soundChunks["player_shoot_1"], 60);
	Mix_VolumeChunk(soundChunks["player_dash"], 45);
	Mix_VolumeChunk(soundChunks["enemy_hit"], 45);


	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_FadeInMusic(backgroundMusic["main"], -1, 2000);
	fprintf(stderr, "Loaded music\n");

	// Create world entities that don't reset
	createRandomRegions(NUM_REGIONS);
	healthbar = createHealthbar({ -CONTENT_WIDTH_PX * 0.35, -CONTENT_HEIGHT_PX * 0.45 }, STATUSBAR_SCALE);

	// Create dialog_system
	if (SHOW_DIALOGS) {
		dialog_system = new DialogSystem(keys_pressed, mouse);
	}

	// Set all states to default
	restart_game();
}

void WorldSystem::step_deathTimer(ScreenState& screen, float elapsed_ms) {
	float min_timer_ms = DEATH_EFFECT_DURATION;
	for (uint i = 0; i < registry.deathTimers.components.size(); i++) {
		DeathTimer& timer = registry.deathTimers.components[i];
		Entity entity = registry.deathTimers.entities[i];
		// progress timer
		timer.timer_ms -= elapsed_ms;

		if (entity == player) {
			if (timer.timer_ms <= 0) {
				// Player is dead -> restart
				registry.deathTimers.remove(entity);
				screen.screen_darken_factor = 0;
				restart_game();
				return;
			}
			else {
				// Player is dying -> set screen darken factor
				screen.screen_darken_factor = 1 - timer.timer_ms / DEATH_EFFECT_DURATION;
			}
		}
		else {
			if (timer.timer_ms <= 0) {
				// Enemy is dead -> remove
				ENEMY_ID type = registry.enemies.get(entity).type;
    			enemyCounts[type]--;
				registry.remove_all_components_of(entity);
			}
		}
	}
}

void WorldSystem::step_health(float elapsed_ms) {
	for (uint i = 0; i < registry.healthValues.components.size(); i++) {
		Health& health = registry.healthValues.components[i];
		Entity entity = registry.healthValues.entities[i];
		if (health.previous_health_pct != health.health_pct) {
			health.timer_ms -= elapsed_ms;
			if (health.timer_ms <= 0) {
				health.previous_health_pct = health.health_pct;
				health.timer_ms = HEALTH_BAR_UPDATE_TIME_SLAP;
			}
			if (health.previous_health_pct <= 0) {
				if (!registry.deathTimers.has(entity)) {
					if (entity == player) {
						isPaused = true;
						Mix_PlayChannel(chunkToChannel["player_death"], soundChunks["player_death"], 0);
						/////////////////////////////
						RenderSystem::animationSys_switchAnimation(player, 
							ANIMATION_FRAME_COUNT::IMMUNITY_DYING, 
							animation_geo_map_general, 
							animation_texture_map_general,
							ceil(DEATH_EFFECT_DURATION / (int)ANIMATION_FRAME_COUNT::IMMUNITY_DYING));
						
						//temporary: since enemies cannot be killed at the moment
						for (int i = 0; i < registry.enemies.entities.size(); i++) {
							Entity& enemyEntity = registry.enemies.entities[i];
							Enemy& enemy = registry.enemies.components[i];
							if (enemy.type == ENEMY_ID::GREEN) {
								RenderSystem::animationSys_switchAnimation(enemyEntity,
									ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING,
									animation_geo_map_general, 
									animation_texture_map_general, 
									ceil(DEATH_EFFECT_DURATION / (int)ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING)
								);

							}
							
						}
						registry.deathTimers.emplace(entity);
					}
					else {
						// Immobilize enemy
						assert(registry.motions.has(entity));
						registry.motions.remove(entity);
						Mix_PlayChannel(chunkToChannel["enemy_death"], soundChunks["enemy_death"], 0);

						Enemy& enemy = registry.enemies.get(entity);
						int buffer = 20; // buffer to fix miss-timing between animation and death
						if (enemy.type == ENEMY_ID::GREEN) {
							RenderSystem::animationSys_switchAnimation(entity, 
								ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING,
								animation_geo_map_general, 
								animation_texture_map_general,
								ceil((DEATH_EFFECT_DURATION_ENEMY + buffer) / (int)ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING)
							);

						}
						DeathTimer& dt = registry.deathTimers.emplace(entity);
						dt.timer_ms = DEATH_EFFECT_DURATION_ENEMY;
					}
				}
			}
		}
		if (entity == player) {
			// Interpolate to get value of displayed healthbar
			float healthbar_scale = 0.f;
			float normalized_time = health.timer_ms / HEALTH_BAR_UPDATE_TIME_SLAP;
			float new_health_pct = health.previous_health_pct * normalized_time + health.health_pct * (1.f - normalized_time);
			healthbar_scale = max(0.f, new_health_pct / 100.f);

			// Update the scale of the healthbar
			assert(registry.transforms.has(healthbar));
			Transform& transform = registry.transforms.get(healthbar);
			transform.scale.x = HEALTHBAR_TEXTURE_SIZE.x * 0.72f * STATUSBAR_SCALE.x * healthbar_scale;

			// Update the color of the healthbar
			assert(registry.colors.has(healthbar));
			vec4& color = registry.colors.get(healthbar);
			color.g = healthbar_scale;
			color.r = 1.f - healthbar_scale;
		}
	}
}

void WorldSystem::step_invincibility(float elapsed_ms) {
	if (registry.invincibility.has(player)) {
		assert(registry.colors.has(player));
		vec4& color = registry.colors.get(player);

		// Reduce timer and change set player flashing
		float& timer = registry.invincibility.get(player).timer_ms;
		timer -= elapsed_ms;
		if (timer <= 0) {
			registry.invincibility.remove(player);
			color.a = 1.f;
		}
		else {
			// Flash once every 200ms
			color.a = mod(ceil(timer / 100.f), 2.f);
		}
	}
}

void WorldSystem::step_attack(float elapsed_ms) {
	assert(registry.players.has(player));
	Player& playerObject = registry.players.get(player);
	playerObject.attack_timer = max(playerObject.attack_timer - elapsed_ms, 0.f);
}

void WorldSystem::step_dash(float elapsed_ms) {
	assert(registry.dashes.has(player));
	Player& playerObject = registry.players.get(player);
	playerObject.attack_timer -= elapsed_ms;
	Dash& playerDash = registry.dashes.get(player);
	playerDash.active_dash_ms -= elapsed_ms;
	playerDash.timer_ms -= elapsed_ms;
}

void WorldSystem::spawnEnemyOfType(ENEMY_ID type, vec2 player_position, vec2 player_velocity) {
    std::default_random_engine rng(std::random_device{}());
    std::uniform_real_distribution<float> angle_randomness(-M_PI/4, M_PI/4);  // 45 degrees randomness

    // Determine the angle for spawning based on player's velocity
    float player_movement_angle = atan2(player_velocity.y, player_velocity.x);
    float random_offset = angle_randomness(rng);
    float angle = length(player_velocity) > 0.001f 
        ? player_movement_angle + random_offset
        : uniform_dist(rng) * 2 * M_PI;

    // Calculate spawn position around the player, but off-screen
    vec2 offset = {cos(angle) * (SCREEN_RADIUS + ENEMY_SPAWN_PADDING), sin(angle) * (SCREEN_RADIUS + ENEMY_SPAWN_PADDING)};
    vec2 spawn_position = player_position + offset;

    switch (type) {
        case ENEMY_ID::RED:
            std::cout << "Spawning Red Enemy at: (" << spawn_position.x << ", " << spawn_position.y << ")" << std::endl;
            createRedEnemy(spawn_position);
            enemyCounts[ENEMY_ID::RED]++;
            break;
        case ENEMY_ID::GREEN:
            std::cout << "Spawning Green Enemy at: (" << spawn_position.x << ", " << spawn_position.y << ")" << std::endl;
            createGreenEnemy(spawn_position);
            enemyCounts[ENEMY_ID::GREEN]++;
            break;
        default:
            break;
    }
}

void WorldSystem::spawnEnemiesNearInterestPoint(vec2 player_position) {
	vec2 player_velocity = registry.motions.get(player).velocity;

    std::uniform_int_distribution<int> type_dist(0, enemyTypes.size() - 1);
    ENEMY_ID randomType = enemyTypes[type_dist(rng)];

    if (enemyCounts[randomType] < getMaxEnemiesForType(randomType)) {
        spawnEnemyOfType(randomType, player_position, player_velocity);
    }

}

int WorldSystem::getMaxEnemiesForType(ENEMY_ID type) {
    auto it = maxEnemies.find(type);
    if (it != maxEnemies.end()) {
        return it->second;
    }
    return 0;
}

float calculateSpawnProbability(vec2 player_position) {
    float nearest_distance = FLT_MAX;
    for (const Region& region : registry.regions.components) {
        float distance = length(player_position - region.interest_point);
        nearest_distance = std::min(nearest_distance, distance);
    }
    // Convert distance to a probability (closer to interest point = higher probability)
    return 1.0f - (nearest_distance / SPAWN_RANGE);
}

void WorldSystem::step_enemySpawn(float elapsed_ms) {
    float elapsed_seconds = elapsed_ms / 1000.f; // Convert milliseconds to seconds

    // Decrease the spawn cooldown timer
    enemy_spawn_cooldown -= elapsed_seconds;

    if (enemy_spawn_cooldown <= 0.f) {
        vec2 player_position = registry.transforms.get(player).position;
        float spawn_probability = calculateSpawnProbability(player_position);
        
        std::uniform_real_distribution<float> spawn_chance(0.f, 1.f);
        if (spawn_chance(rng) < spawn_probability) {
            spawnEnemiesNearInterestPoint(player_position);
        }

        // Reset the cooldown timer for the next individual enemy spawn
        enemy_spawn_cooldown = INDIVIDUAL_SPAWN_INTERVAL;
    }
}

void WorldSystem::remove_garbage() {
	Transform player_transform = registry.transforms.get(player);
	vec2 player_pos = player_transform.position;
	// Remove off-screen bullets to improve performance
	for (Entity bullet : registry.weapons.entities) {
		Transform transform = registry.transforms.get(bullet);
		if (length(transform.position - player_pos) > SCREEN_RADIUS + 200.f) {
			registry.remove_all_components_of(bullet);
		}
	}
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];

	// Step dialog if active
	if (dialog_system != nullptr) {
		if (dialog_system->is_finished()) {
			delete dialog_system;
			dialog_system = nullptr;
		}
		else {
			isPaused = !dialog_system->step(elapsed_ms_since_last_update);
		}
	}
	step_deathTimer(screen, elapsed_ms_since_last_update);
	
	if (isPaused) return false;
	/*************************[ gameplay ]*************************/
	// Code bellow this line will happen only if not paused


	step_health(elapsed_ms_since_last_update);
	step_invincibility(elapsed_ms_since_last_update);
	step_attack(elapsed_ms_since_last_update);
	step_dash(elapsed_ms_since_last_update);
	step_enemySpawn(elapsed_ms_since_last_update);

	// Block velocity update for one step after collision to
	// avoid going out of border / going through enemy
	if (allow_accel) {
		control_movement(elapsed_ms_since_last_update);
	}
	else {
		allow_accel = true;
	}
	control_direction();
	control_action();

	handle_shooting_sound_effect();

	remove_garbage();
	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	/*************************[ cleanup ]*************************/
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");
	// Remove all entities that we created
	// All that have a motion
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.transforms.entities.back());
	// Reset enemy counts
    for (auto &count : enemyCounts) {
        count.second = 0;
    }
	// Debugging for memory/component leaks
	registry.list_all_components();

	/*************************[ setup new world ]*************************/
	// Reset the game state
	current_speed = 1.f;
	isPaused = false;
	isShootingSoundQueued = false;
	// Create a new player
	player = createPlayer({ 0, 0 });
	// hardcode the boss position to upper right region, randomize later
	Region boss_region = registry.regions.components[0];
	createBoss(renderer, boss_region.interest_point);
}

// Compute collisions between entities
void WorldSystem::resolve_collisions() {
	// Loop over all collisions detected by the physics system
	std::list<Entity> garbage{};
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Collision collision = collisionsRegistry.components[i];
		Transform& transform = registry.transforms.get(entity);
		Motion& motion = registry.motions.get(entity);
		// When any moving object collides with the boundary, it gets bounced towards the 
		// reflected direction (similar to the physics model of reflection of light)
		if (collision.collision_type == COLLISION_TYPE::WITH_BOUNDARY) {
			vec2 normal_vec = -normalize(transform.position);
			// Only reflect when velocity is pointing out of the boundary to avoid being stuck
			if (dot(motion.velocity, normal_vec) < 0) {
				vec2 reflection = motion.velocity - 2 * dot(motion.velocity, normal_vec) * normal_vec;
				// Gradually lose some momentum each collision
				motion.velocity = 0.95f * reflection;
				allow_accel = false;
			}
		}
		else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_ENEMY
			&& !registry.invincibility.has(entity)) {
			registry.invincibility.emplace(entity);

			// When player collides with enemy, only player gets knocked back,
			// towards its relative direction from the enemy
			Entity enemy_entity = collision.other_entity;
			Transform& enemy_transform = registry.transforms.get(enemy_entity);
			Motion& enemy_motion = registry.motions.get(enemy_entity);
			vec2 knockback_direction = normalize(transform.position - enemy_transform.position);
			motion.velocity = (enemy_motion.max_velocity + 1000) * knockback_direction;
			allow_accel = false;

			// Update player health
			Health& playerHealth = registry.healthValues.get(entity);
			playerHealth.health_pct -= 10.0;

			Mix_PlayChannel(chunkToChannel["player_hit"], soundChunks["player_hit"], 0);
		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_ENEMY) {
			// When bullet collides with enemy, only enemy gets knocked back,
			// towards its relative direction from the enemy
			Entity enemy_entity = collision.other_entity;
			Enemy enemyAttrib = registry.enemies.get(enemy_entity);
			if (enemyAttrib.type != ENEMY_ID::BOSS) {
				Transform& enemy_transform = registry.transforms.get(enemy_entity);
				Motion& enemy_motion = registry.motions.get(enemy_entity);
				vec2 knockback_direction = normalize(enemy_transform.position - transform.position);
				enemy_motion.velocity = enemy_motion.max_velocity * knockback_direction;
				enemy_motion.allow_accel = false;
			}
			garbage.push_back(entity);

			// Update enemy health
			Health& enemyHealth = registry.healthValues.get(enemy_entity);
			if (enemyAttrib.type == ENEMY_ID::BOSS) {
				enemyHealth.health_pct -= 2.0;
			} else {
				enemyHealth.health_pct -= 10.0;
			}

			Mix_PlayChannel(chunkToChannel["enemy_hit"], soundChunks["enemy_hit"], 0);
		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_BOUNDARY) {
			garbage.push_back(entity);
		}
	}
	for (Entity i : garbage) {
		registry.remove_all_components_of(i);
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}


// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT

	// Mute/Unmute music
	if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
		Mix_PausedMusic() ? Mix_ResumeMusic() : Mix_PauseMusic();
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		restart_game();
	}

	// Pausing game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		isPaused = !isPaused;
	}

	// temporary: shorctut for when testing
	// Quit the program: press 'Q' when game is paused
	if (isPaused && action == GLFW_RELEASE && key == GLFW_KEY_Q) {
		exit(EXIT_SUCCESS);
	}

	// temporary: test health bar. decrease by 10%
	if (action == GLFW_RELEASE && key == GLFW_KEY_H) {
		Health& playerHealth = registry.healthValues.get(player);
		playerHealth.health_pct -= 1.0;
	}

	if (action == GLFW_PRESS) {
		keys_pressed[key] = 1;
	}
	if (action == GLFW_RELEASE) {
		keys_pressed[key] = 0;
	}
	// Debugging
	if (key == GLFW_KEY_F) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 pos) {
	if (registry.deathTimers.has(player)) {
		return;
	}
	mouse = pos;
}

void WorldSystem::control_movement(float elapsed_ms) {
	Motion& playermovement = registry.motions.get(player);
	Dash& playerDash = registry.dashes.get(player);

	// Vertical movement
	if (keys_pressed[GLFW_KEY_W]) {
		playermovement.velocity.y += elapsed_ms * playermovement.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_S]) {
		playermovement.velocity.y -= elapsed_ms * playermovement.acceleration_unit;
	}
	if ((!(keys_pressed[GLFW_KEY_S] || keys_pressed[GLFW_KEY_W])) || (keys_pressed[GLFW_KEY_S] && keys_pressed[GLFW_KEY_W])) {
		playermovement.velocity.y *= pow(playermovement.deceleration_unit, elapsed_ms);
	}

	// Horizontal movement
	if (keys_pressed[GLFW_KEY_D]) {
		playermovement.velocity.x += elapsed_ms * playermovement.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_A]) {
		playermovement.velocity.x -= elapsed_ms * playermovement.acceleration_unit;
	}
	if ((!(keys_pressed[GLFW_KEY_D] || keys_pressed[GLFW_KEY_A])) || (keys_pressed[GLFW_KEY_D] && keys_pressed[GLFW_KEY_A])) {
		playermovement.velocity.x *= pow(playermovement.deceleration_unit, elapsed_ms);
	}

	float magnitude = length(playermovement.velocity);
	//If player is dashing then it can go over the max velocity
	if (playerDash.active_dash_ms <= 0) {
		if (magnitude > playermovement.max_velocity || magnitude < -playermovement.max_velocity) {
			playermovement.velocity *= (playermovement.max_velocity / magnitude);
		}
	}
}

void WorldSystem::control_action() {
	if (keys_pressed[GLFW_MOUSE_BUTTON_LEFT]) {
		player_shoot();
	}
	if (keys_pressed[GLFW_KEY_LEFT_CONTROL]) {
		player_dash();
	}
}



void WorldSystem::control_direction() {
	assert(registry.transforms.has(player));

	float right = (float)CONTENT_WIDTH_PX;
	float bottom = (float)CONTENT_HEIGHT_PX;
	float angle = atan2(-bottom / 2 + mouse.y, right / 2 - mouse.x) + M_PI + 0.7f;
	registry.transforms.get(player).angle = angle;
}

void WorldSystem::player_shoot() {
	Player& playerObject = registry.players.get(player);
	if (playerObject.attack_timer <= 0) {
		createBullet(player, { 10.f, 10.f }, { 1.f, 1.2f, 0.2f, 1.f });
		playerObject.attack_timer = ATTACK_DELAY;
	}
}

void WorldSystem::player_dash() {
	Dash& playerDash = registry.dashes.get(player);
	Motion& playerMovement = registry.motions.get(player);
	if (playerDash.timer_ms > 0) { //make sure player dash cooldown is 0 if not don't allow them to dash
		return;
	}
	playerMovement.velocity *= playerDash.dash_speed;
	playerDash.timer_ms = 600.f;
	playerDash.active_dash_ms = 100.f;

	Mix_PlayChannel(chunkToChannel["player_dash"], soundChunks["player_dash"], 0);
}

void WorldSystem::handle_shooting_sound_effect() {
	Player& playerObject = registry.players.get(player);

	auto play_or_queue_sound = [&]() {
		if (!Mix_Playing(chunkToChannel["player_shoot_1"])) {
			// duck volume if another sound is playing
			int volume = Mix_Playing(-1) ? 40 : 128;
			Mix_Volume(chunkToChannel["player_shoot_1"], volume);
			Mix_PlayChannel(chunkToChannel["player_shoot_1"], soundChunks["player_shoot_1"], 0);
		}
		else {
			// Queue sound if it's already playing
			isShootingSoundQueued = true;
		}
		};

	if (playerObject.attack_timer == ATTACK_DELAY) {
		// We know player just attacked because attack_timer reset
		play_or_queue_sound();
	}
	else if (isShootingSoundQueued && !Mix_Playing(chunkToChannel["player_shoot_1"])) {
		// If attack_timer ticked down too much it's probably too late to play a sound
		if (playerObject.attack_timer >= ATTACK_DELAY / 3.0) {
			play_or_queue_sound();
		}
		isShootingSoundQueued = false;
	}
}
