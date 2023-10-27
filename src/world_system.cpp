// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include <unordered_map>

std::unordered_map < int, int > keys_pressed;
vec2 mouse;
float MAX_VELOCITY = 400;


// Create the world
WorldSystem::WorldSystem() {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
	allow_accel = true;
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (player_dead_sound != nullptr)
		Mix_FreeChunk(player_dead_sound);
	if (player_eat_sound != nullptr)
		Mix_FreeChunk(player_eat_sound);
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

	background_music = Mix_LoadMUS(audio_path("Voxel Revolution.wav").c_str());
	player_dead_sound = Mix_LoadWAV(audio_path("player_dead.wav").c_str());
	player_eat_sound = Mix_LoadWAV(audio_path("player_eat.wav").c_str());

	if (background_music == nullptr || player_dead_sound == nullptr || player_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("Voxel Revolution.wav").c_str(),
			audio_path("player_dead.wav").c_str(),
			audio_path("player_eat.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Create world entities that don't reset
	createRandomRegions(renderer, NUM_REGIONS);
	healthbar = createHealthbar({ -CONTENT_WIDTH_PX * 0.35, -CONTENT_HEIGHT_PX * 0.45 }, STATUSBAR_SCALE);

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
				registry.remove_all_components_of(entity);
			}
			else {
				// Enemy is dying -> dying animation
				// TODO: @Shirley this may be a position to trigger or step the dying animation
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
					}
					else {
						// Immobilize enemy
						assert(registry.motions.has(entity));
						registry.motions.remove(entity);
					}
					registry.deathTimers.emplace(entity);
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
			color.a = mod(floor(timer / 100.f), 2.f);
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
// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];

	step_deathTimer(screen, elapsed_ms_since_last_update);
	if (isPaused) return false;
	step_health(elapsed_ms_since_last_update);
	step_invincibility(elapsed_ms_since_last_update);
	step_attack(elapsed_ms_since_last_update);
	step_dash(elapsed_ms_since_last_update);

	// Block velocity update for one step after collision to
	// avoid going out of border / going through enemy
	if (allow_accel) {
		control_movement();
	}
	else {
		allow_accel = true;
	}
	control_direction();
	control_action();

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
	// Debugging for memory/component leaks
	registry.list_all_components();

	/*************************[ setup new world ]*************************/
	// Reset the game state
	current_speed = 1.f;
	isPaused = false;
	// Create a new player
	player = createPlayer(renderer, { 0, 0 });

	// Create multiple instances of the Red Enemy (new addition)
	int num_enemies = 5; // Adjust this number as desired
	for (int i = 0; i < num_enemies; ++i) {
		vec2 enemy_position = { 50.f + uniform_dist(rng) * (CONTENT_WIDTH_PX - 100.f), 50.f + uniform_dist(rng) * (CONTENT_HEIGHT_PX - 100.f) };
		createRedEnemy(renderer, enemy_position);
	}
	for (int i = 0; i < num_enemies; ++i) {
		vec2 enemy_position = { 50.f + uniform_dist(rng) * (CONTENT_WIDTH_PX - 100.f), 50.f + uniform_dist(rng) * (CONTENT_HEIGHT_PX - 100.f) };
		createGreenEnemy(renderer, enemy_position);
	}
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
		else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_ENEMY &&
				 !registry.invincibility.has(entity)) {
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
		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_ENEMY) {
			// When bullet collides with enemy, only enemy gets knocked back,
			// towards its relative direction from the enemy
			Entity enemy_entity = collision.other_entity;
			Transform& enemy_transform = registry.transforms.get(enemy_entity);
			Motion& enemy_motion = registry.motions.get(enemy_entity);
			vec2 knockback_direction = normalize(enemy_transform.position - transform.position);
			enemy_motion.velocity = MAX_VELOCITY * knockback_direction;
			enemy_motion.allow_accel = false;
			garbage.push_back(entity);
			
			// Update enemy health
			Health& enemyHealth = registry.healthValues.get(enemy_entity);
			enemyHealth.health_pct -= 10.0;
		}
		else if (collision.collision_type == COLLISION_TYPE::ENEMY_WITH_ENEMY) {
			// When two enemies collide, one enemy simply follows the movement of 
			// the other. One of the enemies is considered rigid body in this case
			Entity other_enemy_entity = collision.other_entity;
			Motion& other_enemy_motion = registry.motions.get(other_enemy_entity);
			motion.velocity = other_enemy_motion.velocity;
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

void WorldSystem::control_movement() {
	Motion& playermovement = registry.motions.get(player);
	Player& playerObject = registry.players.get(player);
	Dash& playerDash = registry.dashes.get(player);

	// Vertical movement
	if (keys_pressed[GLFW_KEY_W]) {
		playermovement.velocity.y += playermovement.max_velocity * playermovement.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_S]) {
		playermovement.velocity.y -= playermovement.max_velocity * playermovement.acceleration_unit;
	}
	if ((!(keys_pressed[GLFW_KEY_S] || keys_pressed[GLFW_KEY_W])) || (keys_pressed[GLFW_KEY_S] && keys_pressed[GLFW_KEY_W])) {
		playermovement.velocity.y *= playermovement.deceleration_unit;
	}

	// Horizontal movement
	if (keys_pressed[GLFW_KEY_D]) {
		playermovement.velocity.x += playermovement.max_velocity * playermovement.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_A]) {
		playermovement.velocity.x -= playermovement.max_velocity * playermovement.acceleration_unit;
	}
	if ((!(keys_pressed[GLFW_KEY_D] || keys_pressed[GLFW_KEY_A])) || (keys_pressed[GLFW_KEY_D] && keys_pressed[GLFW_KEY_A])) {
		playermovement.velocity.x *= playermovement.deceleration_unit;
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
	float angle = atan2(-bottom / 2 + mouse.y, right / 2 - mouse.x) + M_PI + 0.70;
	registry.transforms.get(player).angle = angle;
}

void WorldSystem::player_shoot() {
	Player& playerObject = registry.players.get(player);
	if (playerObject.attack_timer <= 0) {
		createBullet(player, { 10.f, 10.f }, {1.f, 0.2f, 0.2f, 1.f});
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

}
