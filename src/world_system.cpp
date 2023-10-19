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
	void glfw_err_cb(int error, const char *desc) {
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

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Cytotoxic Cataclysm", nullptr, nullptr);
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

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	player_dead_sound = Mix_LoadWAV(audio_path("player_dead.wav").c_str());
	player_eat_sound = Mix_LoadWAV(audio_path("player_eat.wav").c_str());

	if (background_music == nullptr || player_dead_sound == nullptr || player_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
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

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_timer_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if(timer.timer_ms < min_timer_ms){
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.screen_darken_factor = 0;
            restart_game();
			return true;
		}
	}
	for (auto i : registry.healthValues.entities) {
		if (registry.players.has(i)) {
			continue;
		}
		if (registry.healthValues.get(i).currentHealthPercentage <= 0.0) {
			registry.remove_all_components_of(i);
		}
	}
	Player& playerObject = registry.players.get(player);
	playerObject.attack_timer -= elapsed_ms_since_last_update;

	//constantly checking if the current health value
	Health& playerHealthBar = registry.healthValues.get(player);
	if (playerHealthBar.currentHealthPercentage != playerHealthBar.targetHealthPercentage && playerHealthBar.timer_ms > 0) {
		playerHealthBar.timer_ms -= elapsed_ms_since_last_update;
		if (playerHealthBar.timer_ms < min_timer_ms && playerHealthBar.targetHealthPercentage <= 0.0) {
			min_timer_ms = playerHealthBar.timer_ms;
		}

		// Resume the static state of the Health Bar 
		if (playerHealthBar.timer_ms < 0) {
			playerHealthBar.timer_ms = HEALTH_BAR_UPDATE_TIME_SLAP;
			playerHealthBar.currentHealthPercentage = playerHealthBar.targetHealthPercentage;
			assert(playerHealthBar.currentHealthPercentage == playerHealthBar.targetHealthPercentage);
			
		}

		if (playerHealthBar.currentHealthPercentage <= 0.0) {
			screen.screen_darken_factor = 0;
			restart_game();
			return true;
		}
	}
	
	

	// reduce window brightness if deathTimer has progressed
	screen.screen_darken_factor = 1 - min_timer_ms / 3000;
	// Block velocity update for one step after collision to
	// avoid going out of border / going through enemy
	if (allow_accel) {
		movement();
	} else {
		allow_accel = true;
	}
	direction();
	enemy_movement();

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create a new player
	player = createPlayer(renderer, { 0, 0 });

	// Create map sections
	createRandomRegion(renderer, NUM_REGIONS);

	// Create multiple instances of the Red Enemy (new addition)
    int num_enemies = 5; // Adjust this number as desired
    for (int i = 0; i < num_enemies; ++i) {
        vec2 enemy_position = { 50.f + uniform_dist(rng) * (window_width_px - 100.f), 50.f + uniform_dist(rng) * (window_height_px - 100.f) };
        createRedEnemy(renderer, enemy_position);
    }
	for (int i = 0; i < num_enemies; ++i) {
		vec2 enemy_position = { 50.f + uniform_dist(rng) * (window_width_px - 100.f), 50.f + uniform_dist(rng) * (window_height_px - 100.f) };
		createGreenEnemy(renderer, enemy_position);
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	std::list<Entity> garbage{};
	bool delete_entity = false;
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Collision collision = collisionsRegistry.components[i];
		Motion& motion = registry.motions.get(entity);
		// When any moving object collides with the boundary, it gets bounced towards the 
		// reflected direction (similar to the physics model of reflection of light)
		if (collision.collision_type == COLLISION_TYPE::WITH_BOUNDARY) {
			vec2 normal_vec = -normalize(motion.position);
			// Only reflect when velocity is pointing out of the boundary to avoid being stuck
			if (dot(motion.velocity, normal_vec) < 0) {
				vec2 reflection = motion.velocity - 2 * dot(motion.velocity, normal_vec) * normal_vec;
				// Gradually lose some momentum each collision
				motion.velocity = 0.95f * reflection;
				allow_accel = false;
			}
		} else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_ENEMY) {
			// When player collides with enemy, only player gets knocked back,
			// towards its relative direction from the enemy
			Entity enemy_entity = collisionsRegistry.components[i].other_entity;
			Enemy enemy = registry.enemies.get(enemy_entity);
			Motion& enemy_motion = registry.motions.get(enemy_entity); 
			vec2 knockback_direction = normalize(motion.position - enemy_motion.position);
			motion.velocity = (enemy.max_velocity+1000) * knockback_direction;
			allow_accel = false;


		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_ENEMY) {
			// When bullet collides with enemy, only enemy gets knocked back,
			// towards its relative direction from the enemy
			Entity enemy_entity = collisionsRegistry.components[i].other_entity;
			Motion& enemy_motion = registry.motions.get(enemy_entity);
			vec2 knockback_direction = normalize(enemy_motion.position - motion.position);
			enemy_motion.velocity = MAX_VELOCITY * knockback_direction;
			Enemy& enemy = registry.enemies.get(enemy_entity);
			enemy.allow_accel = false;
			garbage.push_back(entity);
		}
		else if (collision.collision_type == COLLISION_TYPE::ENEMY_WITH_ENEMY) {
			// When two enemies collide, one enemy simply follows the movement of 
			// the other. One of the enemies is considered rigid body in this case
			Entity other_enemy_entity = collisionsRegistry.components[i].other_entity;
			Motion& other_enemy_motion = registry.motions.get(other_enemy_entity);
			motion.velocity = other_enemy_motion.velocity;
		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_BOUNDARY) {
			garbage.push_back(entity);
		}
	}
	for (auto i : garbage) {
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

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
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
void WorldSystem::enemy_movement() {
	for (Entity entity : registry.enemies.entities) {
		Enemy& enemy = registry.enemies.get(entity);
		if (enemy.allow_accel == false) {
			enemy.allow_accel= true;
			continue;
		}
		Motion& enemymotion = registry.motions.get(entity);
		vec2 playerposition = registry.motions.get(player).position;
		float angle = atan2(enemymotion.position.y - playerposition.y, enemymotion.position.x - playerposition.x);
		enemymotion.velocity.x += -cos(angle) *enemy.max_velocity*enemy.acceleration_unit;
		enemymotion.velocity.y += -sin(angle) *enemy.max_velocity*enemy.acceleration_unit;
		enemymotion.angle = angle+M_PI+0.8;

		float magnitude = length(enemymotion.velocity);

		if (magnitude > enemy.max_velocity || magnitude < -enemy.max_velocity) {
			enemymotion.velocity *= (enemy.max_velocity / magnitude);
		}
	}

}
void WorldSystem::movement() {
	Motion& playermovement = registry.motions.get(player);
	Player& playerobject = registry.players.get(player);

	if (keys_pressed[GLFW_MOUSE_BUTTON_LEFT]) {
		bullets();
	}

	//temporary: test health bar. decrease by 10%
	if (keys_pressed[GLFW_KEY_H]) {
		Health& playerHealth = registry.healthValues.get(player);
		playerHealth.targetHealthPercentage -= 1.0;
	}

	if (keys_pressed[GLFW_KEY_W]) {
		playermovement.velocity.y += playerobject.max_velocity* playerobject.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_S]) {
		playermovement.velocity.y -= playerobject.max_velocity * playerobject.acceleration_unit;
	}

	if ((!(keys_pressed[GLFW_KEY_S] || keys_pressed[GLFW_KEY_W])) || (keys_pressed[GLFW_KEY_S] && keys_pressed[GLFW_KEY_W])) {
		playermovement.velocity.y *= playerobject.deceleration_unit;
	}

	if (keys_pressed[GLFW_KEY_D]) {
		playermovement.velocity.x += playerobject.max_velocity * playerobject.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_A]) {
		playermovement.velocity.x -= playerobject.max_velocity * playerobject.acceleration_unit;
	}

	if ((!(keys_pressed[GLFW_KEY_D] || keys_pressed[GLFW_KEY_A])) || (keys_pressed[GLFW_KEY_D] && keys_pressed[GLFW_KEY_A])) {
		playermovement.velocity.x *= playerobject.deceleration_unit;
	}

	float magnitude = length(playermovement.velocity);

	if (magnitude > playerobject.max_velocity || magnitude < -playerobject.max_velocity) {
		playermovement.velocity *= (playerobject.max_velocity / magnitude);
	}
}

void WorldSystem::on_mouse_move(vec2 pos) {
	if (registry.deathTimers.has(player)) {
		return;
	}
	mouse = pos;
}

void WorldSystem::direction() {
	Motion playerdirection = registry.motions.get(player);

	float right = (float)window_width_px;
	float bottom = (float)window_height_px;

	float angle = atan2(-bottom/2 + mouse.y, right/2 - mouse.x) + M_PI+0.70;

	registry.motions.get(player).angle = angle;
}	

void WorldSystem::bullets() {
	Player& playerObject = registry.players.get(player);
	if (playerObject.attack_timer > 0) {
		return;
	}
	Motion& playerpos = registry.motions.get(player);


	float radius = 10;
	Entity bullet = createBullet({ playerpos.position.x + 60 * cos(playerpos.angle), playerpos.position.y + 60 * sin(playerpos.angle) },
		{ radius, radius });
	float brightness = 1.0;
	registry.colors.insert(bullet, { brightness, brightness, brightness });
	playerObject.attack_timer = 200.f;
}


