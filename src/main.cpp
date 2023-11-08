
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world_system;
	RenderSystem render_system;
	PhysicsSystem physics_system;
	AISystem ai_system;

	// Initializing window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}
	glfwSetWindowTitle(window, "Cytotoxic Cataclysm");

	// initialize the main systems
	render_system.init(window);
	world_system.init(&render_system);
	render_system.animationSys_init();

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		bool isNotPaused = world_system.step(elapsed_ms);
		
		if (isNotPaused) {
			physics_system.step(elapsed_ms);
			world_system.resolve_collisions();
			ai_system.step(elapsed_ms);
			render_system.animationSys_step(elapsed_ms);
			world_system.update_camera();
		}
		
		render_system.draw();
	}

	return EXIT_SUCCESS;
}
