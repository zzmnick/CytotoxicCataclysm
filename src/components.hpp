#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{

};

// Enemy component
struct Enemy
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	float angle = 0.f;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
};

enum class COLLISION_TYPE {
	WITH_BOUNDARY = 0,
	PLAYER_WITH_ENEMY = WITH_BOUNDARY + 1,
	ENEMY_WITH_ENEMY = PLAYER_WITH_ENEMY + 1
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	COLLISION_TYPE collision_type;
	Entity other_entity; // the second object involved in the collision
	Collision(COLLISION_TYPE collision_type, Entity& other_entity) {
		this->other_entity = other_entity; 
		this->collision_type = collision_type;
	};
	Collision(COLLISION_TYPE collision_type) {
		assert(collision_type == COLLISION_TYPE::WITH_BOUNDARY && 
			   "other_entity must be specified unless colliding with boundary");
		this->collision_type = collision_type;
	}
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float screen_darken_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying player
struct DeathTimer
{
	float timer_ms = 3000.f;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

enum class REGION_THEME_ID {
	NERVOUS = 0,
	RESPIRATORY = NERVOUS + 1,
	URINARY = RESPIRATORY + 1,
	MUSCULAR = URINARY + 1,
	SKELETAL = MUSCULAR + 1,
	CUTANEOUS = SKELETAL + 1,
	REGION_THEME_COUNT = CUTANEOUS + 1
};
const int region_theme_count = (int)REGION_THEME_ID::REGION_THEME_COUNT;

enum class REGION_GOAL_ID {
	CURE = 0,
	CANCER_CELL = CURE + 1,
	MELEE_WEAPON = CANCER_CELL + 1,
	DOUBLE_DAMAGE = MELEE_WEAPON + 1,
	REGION_GOAL_COUNT = DOUBLE_DAMAGE + 1
};
const int region_goal_count = (int)REGION_GOAL_ID::REGION_GOAL_COUNT;

enum class ENEMY_ID {
	ORANGE = 0,
	ENEMY_COUNT = ORANGE + 1
};
const int enemy_type_count = (int)ENEMY_ID::ENEMY_COUNT;

enum class BOSS_ID {
	SALMONELLA = 0,
	ECOLI = SALMONELLA + 1,
	BOSS_COUNT = ECOLI + 1
};
const int boss_type_count = (int)BOSS_ID::BOSS_COUNT;

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	IMMUNITY = 0,
	RED_ENEMY = IMMUNITY + 1,
	GREEN_ENEMY = RED_ENEMY + 1,
	NERVOUS_BG = GREEN_ENEMY + 1,
	RESPIRATORY_BG = NERVOUS_BG + 1,
	URINARY_BG = RESPIRATORY_BG + 1,
	MUSCULAR_BG = URINARY_BG + 1,
	SKELETAL_BG = MUSCULAR_BG + 1,
	CUTANEOUS_BG = SKELETAL_BG + 1,
	HEALTHBARFRAME = CUTANEOUS_BG + 1,
	TEXTURE_COUNT = HEALTHBARFRAME + 1 // TEXTURE_COUNT indicates that no txture is needed
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
	SCREEN = TEXTURED + 1,
	REGION = SCREEN + 1,
	HEALTHBAR = REGION + 1,
	STATICWINDOW = HEALTHBAR + 1,
	EFFECT_COUNT = STATICWINDOW + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	DEBUG_LINE = SPRITE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	REGION_TRIANGLE = SCREEN_TRIANGLE + 1,
	HEALTH_RECTANGLE = REGION_TRIANGLE + 1,
	HEALTHBARFRAME_RECTANGLE = HEALTH_RECTANGLE + 1,
	GEOMETRY_COUNT = HEALTHBARFRAME_RECTANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

// map between region theme and the corresponding texture asset
static std::unordered_map <REGION_THEME_ID, TEXTURE_ASSET_ID> region_texture_map = {
	{REGION_THEME_ID::NERVOUS, TEXTURE_ASSET_ID::NERVOUS_BG},
	{REGION_THEME_ID::RESPIRATORY, TEXTURE_ASSET_ID::RESPIRATORY_BG},
	{REGION_THEME_ID::URINARY, TEXTURE_ASSET_ID::URINARY_BG},
	{REGION_THEME_ID::MUSCULAR, TEXTURE_ASSET_ID::MUSCULAR_BG},
	{REGION_THEME_ID::SKELETAL, TEXTURE_ASSET_ID::SKELETAL_BG},
	{REGION_THEME_ID::CUTANEOUS, TEXTURE_ASSET_ID::CUTANEOUS_BG}
};

// Stucture to store body region (section) information
struct Region
{
	float angle;
	REGION_THEME_ID theme = REGION_THEME_ID::REGION_THEME_COUNT;
	REGION_GOAL_ID goal = REGION_GOAL_ID::REGION_GOAL_COUNT;
	ENEMY_ID enemy = ENEMY_ID::ENEMY_COUNT;
	BOSS_ID boss = BOSS_ID::BOSS_COUNT;
	bool is_cleared = false;
};

struct Health
{
	float currentHealthPercentage = 100.f;
	float targetHealthPercentage = 100.f;
	float timer_ms = HEALTH_BAR_UPDATE_TIME_SLAP;
};
