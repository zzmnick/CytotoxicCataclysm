#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BACTERIOPHAGE, mesh_path("bacteriophage.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("empty.png"),
		textures_path("immunity.png"),
		textures_path("red_enemy.png"),
		textures_path("green_enemy.png"),
		textures_path("nervous_bg.png"),
		textures_path("respiratory_bg.png"),
		textures_path("urinary_bg.png"),
		textures_path("muscular_bg.png"),
		textures_path("skeletal_bg.png"),
		textures_path("cutaneous_bg.png"),
		textures_path("healthbarframe.png"),
		textures_path("gun.png"),
		textures_path("immunity_moving.png"),
		textures_path("immunity_dying.png"),
		textures_path("green_enemy_moving.png"),
		textures_path("green_enemy_dying.png"),
		textures_path("immunity_blink.png"),
		textures_path("bacteriophage.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("textured"),
		shader_path("screen"),
		shader_path("region"),
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the screen
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	mat3 createViewMatrix();
	vec2 offset;

	//animation system
	void initAnimation(GEOMETRY_BUFFER_ID gid, ANIMATION_FRAME_COUNT fcount);
	void animationSys_step(float elapsed_ms);
	void animationSys_init();
	static void animationSys_switchAnimation(Entity& entity, 
		ANIMATION_FRAME_COUNT animationType, 
		std::unordered_map<ANIMATION_FRAME_COUNT, 
		GEOMETRY_BUFFER_ID> geo_map, std::unordered_map<ANIMATION_FRAME_COUNT, TEXTURE_ASSET_ID> tex_map,
		int update_period_ms);
	
private:
	Entity player; // Keep reference to player entity

	// Internal drawing functions for each entity type
	void drawEntity(
		Entity entity,
		const RenderRequest& render_request,
		const mat3& transform,
		const mat3& viewProjection
	);
	// void drawBackground(const mat3& viewProjection);
	void drawToScreen();
	void setUniformShaderVars(
		Entity entity,
		const mat3& transform,
		const mat3& viewProjection
	);
	void setTexturedShaderVars(Entity entity);
	void setColouredShaderVars(Entity entity);
	void setRegionShaderVars(Entity entity);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
	
};


bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
