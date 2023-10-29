#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

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

	// Check for collisions
	void resolve_collisions();

	// Should the game be over ?
	bool is_over()const;
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
	float current_speed;
	Entity player;
	bool isPaused;

	// UI references
	Entity healthbar;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* player_dead_sound;
	Mix_Chunk* player_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	bool allow_accel;

	// Step different sub-systems
	void step_deathTimer(ScreenState& screen, float elapsed_ms);
	void step_health(float elapsed_ms);
	void step_invincibility(float elapsed_ms);
	void step_attack(float elapsed_ms);
	void step_dash(float elapsed_ms);
};
