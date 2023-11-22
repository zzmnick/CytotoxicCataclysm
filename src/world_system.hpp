#pragma once

// internal
#include "common.hpp"
#include "render_system.hpp"
#include "./sub_systems/dialog_system.hpp"
#include "./sub_systems/effects_system.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);


	void update_camera();

	// Check for collisions
	void resolve_collisions();

	// Should the game be over ?
	bool is_over()const;

	void startEntityDeath(Entity entity);

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);

	// User input handlers
	void control_movement(float elapsed_ms);
	void control_direction();
	void control_action();

	void player_shoot();
	void player_dash();

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	// Game state
	RenderSystem* renderer;
	EffectsSystem* effects_system;
	float current_speed;
	Entity player;
	bool isPaused;

	// UI references
	Entity healthbar;

	// sound references and handler
	std::unordered_map<std::string, Mix_Music*> backgroundMusic;
	std::unordered_map<std::string, Mix_Chunk*> soundChunks;
	std::unordered_map<std::string, int> chunkToChannel;
	void handle_shooting_sound_effect();
	bool isShootingSoundQueued;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	DialogSystem* dialog_system = nullptr;
	
	// Maintain enemy counts
	std::unordered_map<ENEMY_ID, int> enemyCounts;
	std::unordered_map<ENEMY_ID, int> maxEnemies;

	bool allow_accel;

	// Step different sub-systems
	void step_deathTimer(ScreenState& screen, float elapsed_ms);
	void step_health();
	void step_healthbar(float elapsed_ms);
	void step_invincibility(float elapsed_ms);
	void step_attack(float elapsed_ms);
	void step_dash(float elapsed_ms);
	void step_enemySpawn(float elapsed_ms);
	void step_timer_with_callback(float elapsed_ms);

	void spawnEnemiesNearInterestPoint(vec2 player_position);
	void spawnEnemyOfType(ENEMY_ID type, vec2 player_position, vec2 player_velocity);
	int getMaxEnemiesForType(ENEMY_ID type);

	void remove_garbage();

	void clear_game_state();
	void load_game();
	void save_game();
	json serializeGameState();

	Entity& getPlayerBelonging(PLAYER_BELONGING_ID id);
};
