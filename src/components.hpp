#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

#pragma region Enums

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
	GREEN = 0,
	RED = GREEN + 1,
	ORANGE = RED + 1,
	ENEMY_COUNT = ORANGE + 1
};
const int enemy_type_count = (int)ENEMY_ID::ENEMY_COUNT;

enum class BOSS_ID {
	BACTERIOPHAGE = 0,
	ECOLI = BACTERIOPHAGE + 1,
	BOSS_COUNT = ECOLI + 1
};
const int boss_type_count = (int)BOSS_ID::BOSS_COUNT;

enum class COLLISION_TYPE {
	WITH_BOUNDARY = 0,
	PLAYER_WITH_ENEMY = WITH_BOUNDARY + 1,
	ENEMY_WITH_ENEMY = PLAYER_WITH_ENEMY + 1,
	BULLET_WITH_ENEMY = ENEMY_WITH_ENEMY + 1,
	BULLET_WITH_PLAYER = BULLET_WITH_ENEMY + 1,
	BULLET_WITH_BULLET = BULLET_WITH_PLAYER + 1,
	BULLET_WITH_BOUNDARY = BULLET_WITH_BULLET + 1
};

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
	HEALTHBAR_FRAME = CUTANEOUS_BG + 1,
	TEXTURE_COUNT = HEALTHBAR_FRAME + 1 // TEXTURE_COUNT indicates that no txture is needed
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
	SCREEN = TEXTURED + 1,
	REGION = SCREEN + 1,
	EFFECT_COUNT = REGION + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	DEBUG_LINE = SPRITE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	REGION_TRIANGLE = SCREEN_TRIANGLE + 1,
	STATUSBAR_RECTANGLE = REGION_TRIANGLE + 1,
	BULLET = STATUSBAR_RECTANGLE + 1,
	GEOMETRY_COUNT = BULLET + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class RENDER_ORDER {
	BACKGROUND = 0,
	OBJECTS_BK = BACKGROUND + 1,
	OBJECTS_FR = OBJECTS_BK + 1,
	UI = OBJECTS_FR + 1,
	RENDER_ORDER_COUNT = UI + 1
};
const int render_order_count = (int)RENDER_ORDER::RENDER_ORDER_COUNT;

// map between region theme and the corresponding texture asset
static std::unordered_map <REGION_THEME_ID, TEXTURE_ASSET_ID> region_texture_map = {
	{REGION_THEME_ID::NERVOUS, TEXTURE_ASSET_ID::NERVOUS_BG},
	{REGION_THEME_ID::RESPIRATORY, TEXTURE_ASSET_ID::RESPIRATORY_BG},
	{REGION_THEME_ID::URINARY, TEXTURE_ASSET_ID::URINARY_BG},
	{REGION_THEME_ID::MUSCULAR, TEXTURE_ASSET_ID::MUSCULAR_BG},
	{REGION_THEME_ID::SKELETAL, TEXTURE_ASSET_ID::SKELETAL_BG},
	{REGION_THEME_ID::CUTANEOUS, TEXTURE_ASSET_ID::CUTANEOUS_BG}
};
#pragma endregion

#pragma region Components

// Player component
struct Player
{
	float attack_timer = ATTACK_DELAY;


};

// Enemy component
struct Enemy {};

// Data relevant to the shape of entities
struct Transform {
	vec2 position = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
	float angle = 0.f;
	bool is_screen_coord = false;
};

// Data relevant to the movement of entities
struct Motion {
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
	float max_velocity = 400.f;
	float acceleration_unit = 0.05f;
	float deceleration_unit = 0.9f;
	bool allow_accel = true;
};

// Identifies actions allowed by the player
struct Action {
	bool move_up = true;
	bool move_down = true;
	bool move_right = true;
	bool move_left = true;
	bool rotate = true;
	bool dash = true;
	bool shoot = true;
};

// Stucture to store collision information
struct Collision {
	// Note, the first object is stored in the ECS container.entities
	COLLISION_TYPE collision_type;
	Entity other_entity; // the second object involved in the collision
	Collision(COLLISION_TYPE collision_type, Entity& other_entity) {
		this->other_entity = other_entity;
		this->collision_type = collision_type;
	};
	Collision(COLLISION_TYPE collision_type) {
		assert((collision_type == COLLISION_TYPE::WITH_BOUNDARY || collision_type == COLLISION_TYPE::BULLET_WITH_BOUNDARY) &&
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
struct ScreenState {
	float screen_darken_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent {
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying player
struct DeathTimer
{
	float timer_ms = DEATH_EFFECT_DURATION;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl)
struct ColoredVertex {
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex {
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh {
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = { 1,1 };
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_ORDER order = RENDER_ORDER::RENDER_ORDER_COUNT;
};

// Stucture to store body region (section) information
struct Region {
	REGION_THEME_ID theme = REGION_THEME_ID::REGION_THEME_COUNT;
	REGION_GOAL_ID goal = REGION_GOAL_ID::REGION_GOAL_COUNT;
	ENEMY_ID enemy = ENEMY_ID::ENEMY_COUNT;
	BOSS_ID boss = BOSS_ID::BOSS_COUNT;
	bool is_cleared = false;
};

struct Health {
	float health_pct = 100.f;
	float previous_health_pct = 100.f;
	float timer_ms = HEALTH_BAR_UPDATE_TIME_SLAP;
};

struct Invincibility {
	float timer_ms = 700.f;
};

struct Weapon {
	float damage = 10.f;
};

struct Dash
{
	float timer_ms = 400.f;
	float active_dash_ms = 0.f;
	float dash_speed = 5.f;
};
#pragma endregion
