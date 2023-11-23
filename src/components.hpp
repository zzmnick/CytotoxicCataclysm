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
	TEMP1 = DOUBLE_DAMAGE + 1,
	TEMP2 = TEMP1 + 1,
	REGION_GOAL_COUNT = TEMP2 + 1

};
const int region_goal_count = (int)REGION_GOAL_ID::REGION_GOAL_COUNT;

enum class ENEMY_ID {
	BOSS = 0,
	FRIENDBOSS = BOSS + 1,
	FRIENDBOSSCLONE = FRIENDBOSS + 1,
	GREEN = FRIENDBOSSCLONE + 1,
	RED = GREEN + 1,
	YELLOW = RED + 1,
	ENEMY_COUNT = YELLOW + 1
};
const int enemy_type_count = (int)ENEMY_ID::ENEMY_COUNT;

enum class BOSS_ID {
	BACTERIOPHAGE = 0,
	FRIEND = BACTERIOPHAGE + 1,
	BOSS_COUNT = FRIEND + 1
};
const int boss_type_count = (int)BOSS_ID::BOSS_COUNT;

enum class COLLISION_TYPE {
	WITH_BOUNDARY = 0,
	PLAYER_WITH_ENEMY = WITH_BOUNDARY + 1,
	PLAYER_WITH_CYST = PLAYER_WITH_ENEMY + 1,
	ENEMY_WITH_ENEMY = PLAYER_WITH_CYST + 1,
	BULLET_WITH_ENEMY = ENEMY_WITH_ENEMY + 1,
	BULLET_WITH_PLAYER = BULLET_WITH_ENEMY + 1,
	BULLET_WITH_BULLET = BULLET_WITH_PLAYER + 1,
	BULLET_WITH_BOUNDARY = BULLET_WITH_BULLET + 1,
	BULLET_WITH_CYST = BULLET_WITH_BOUNDARY + 1,
	SWORD_WITH_ENEMY = BULLET_WITH_CYST + 1,
	SWORD_WITH_CYST = SWORD_WITH_ENEMY + 1
};

enum class CYST_EFFECT_ID {
	// POSITIVE EFFECTS
	DAMAGE = 0,
	HEAL = DAMAGE + 1,
	CLEAR_SCREEN = HEAL + 1,

	// NEGATIVE EFFECTS
	SLOW = CLEAR_SCREEN + 1,
	FOV = SLOW + 1,
	DIRECTION = FOV + 1,
	NO_ATTACK = DIRECTION + 1,
	EFFECT_COUNT = NO_ATTACK + 1
};
const int cyst_effect_count = static_cast<int>(CYST_EFFECT_ID::EFFECT_COUNT);
const int cyst_neg_start = static_cast<int>(CYST_EFFECT_ID::SLOW);

enum class GAME_STATE {
	START_MENU = 0,
	PAUSE_MENU = START_MENU + 1,
	DIALOG = PAUSE_MENU + 1,
	RUNNING = DIALOG + 1
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
	EMPTY = 0,
	IMMUNITY = EMPTY + 1,
	RED_ENEMY = IMMUNITY + 1,
	GREEN_ENEMY = RED_ENEMY + 1,
	YELLOW_ENEMY = GREEN_ENEMY+ 1,
	HEALTHBAR_FRAME = YELLOW_ENEMY + 1,
	GUN = HEALTHBAR_FRAME + 1,
	CYST = GUN + 1,
	IMMUNITY_MOVING = CYST + 1,
	IMMUNITY_DYING = IMMUNITY_MOVING + 1,
	GREEN_ENEMY_MOVING = IMMUNITY_DYING +1,
	GREEN_ENEMY_DYING = GREEN_ENEMY_MOVING + 1,
	IMMUNITY_BLINKING = GREEN_ENEMY_DYING + 1,
	CYST_SHINE = IMMUNITY_BLINKING + 1,
	BACTERIOPHAGE = CYST_SHINE + 1,
	DASHING = BACTERIOPHAGE + 1,
	FRIEND = DASHING + 1,

	// Background
	NERVOUS_BG = FRIEND + 1,
	RESPIRATORY_BG = NERVOUS_BG + 1,
	URINARY_BG = RESPIRATORY_BG + 1,
	MUSCULAR_BG = URINARY_BG + 1,
	SKELETAL_BG = MUSCULAR_BG + 1,
	CUTANEOUS_BG = SKELETAL_BG + 1,

	// UI
	DIALOG_INTRO1 = CUTANEOUS_BG + 1,
	DIALOG_INTRO2 = DIALOG_INTRO1 + 1,
	DIALOG_INTRO3 = DIALOG_INTRO2 + 1,
	TUTORIAL_MOVEMENT = DIALOG_INTRO3 + 1,
	TUTORIAL_ROTATE = TUTORIAL_MOVEMENT + 1,
	// TUTORIAL_DASH = TUTORIAL_ROTATE + 1,
	TUTORIAL_SHOOT = TUTORIAL_ROTATE + 1,
	TUTORIAL_PAUSE = TUTORIAL_SHOOT + 1,
	TUTORIAL_END = TUTORIAL_PAUSE + 1,
	ICON_DAMAGE = TUTORIAL_END + 1,
	ICON_SLOW = ICON_DAMAGE + 1,
	ICON_FOV = ICON_SLOW + 1,
	ICON_AMMO = ICON_FOV + 1,
	START_MENU_TITLE = ICON_AMMO + 1,
	PAUSE_MENU_TITLE = START_MENU_TITLE + 1,
	MENU_START = PAUSE_MENU_TITLE + 1,
	MENU_LOAD = MENU_START + 1,
	MENU_EXIT = MENU_LOAD + 1,
	MENU_RESUME = MENU_EXIT + 1,
	MENU_SAVE = MENU_RESUME + 1,
	MENU_MUTE = MENU_SAVE + 1,
	MENU_UNMUTE = MENU_MUTE + 1,
	CROSSHAIR = MENU_UNMUTE + 1,
	TEXTURE_COUNT = CROSSHAIR + 1 // TEXTURE_COUNT indicates that no txture is needed
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
	EMPTY = 0,
	SPRITE = EMPTY + 1,
	DEBUG_LINE = SPRITE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	REGION_TRIANGLE = SCREEN_TRIANGLE + 1,
	STATUSBAR_RECTANGLE = REGION_TRIANGLE + 1,
	BULLET = STATUSBAR_RECTANGLE + 1,
	SPRITESHEET_IMMUNITY_MOVING = BULLET + 1,
	SPRITESHEET_GREEN_ENEMY_MOVING = SPRITESHEET_IMMUNITY_MOVING + 1,
	SPRITESHEET_GREEN_ENEMY_DYING = SPRITESHEET_GREEN_ENEMY_MOVING + 1,
	SPRITESHEET_IMMUNITY_DYING = SPRITESHEET_GREEN_ENEMY_DYING + 1,
	SPRITESHEET_IMMUNITY_BLINKING = SPRITESHEET_IMMUNITY_DYING + 1,
	SPRITESHEET_CYST_SHINE = SPRITESHEET_IMMUNITY_BLINKING + 1,
	SPRITESHEET_DASHING = SPRITESHEET_CYST_SHINE + 1,
	BACTERIOPHAGE = SPRITESHEET_DASHING + 1,
	SWORD = BACTERIOPHAGE + 1,
	MENU_BACKGROUND = SWORD + 1,
	GEOMETRY_COUNT = MENU_BACKGROUND + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class RENDER_ORDER {
	BACKGROUND = 0,
	OBJECTS_BK = BACKGROUND + 1,
	PLAYER = OBJECTS_BK + 1,
	OBJECTS_FR = PLAYER + 1,
	BOSS = OBJECTS_FR + 1,
	UI = BOSS + 1,
	MENU = UI + 1,
	RENDER_ORDER_COUNT = MENU + 1
};
const int render_order_count = (int)RENDER_ORDER::RENDER_ORDER_COUNT;

enum class ANIMATION_FRAME_COUNT {
	IMMUNITY_MOVING = 10,
	IMMUNITY_DYING = 5,
	GREEN_ENEMY_DYING = 7,
	GREEN_ENEMY_MOVING = 4,
	IMMUNITY_BLINKING = 2,
	CYST_SHINE = 4,
	DASHING = 8,
};

enum MENU_OPTION {
    START_GAME = 0,
    LOAD_GAME = START_GAME + 1,
    EXIT_GAME = LOAD_GAME + 1,
    RESUME_GAME = EXIT_GAME + 1,
    SAVE_GAME = RESUME_GAME + 1,
    MUTE_SOUND = SAVE_GAME + 1,
    UNMUTE_SOUND = MUTE_SOUND + 1,
    EXIT_CURR_PLAY = UNMUTE_SOUND + 1,
	NONE = EXIT_CURR_PLAY + 1
};

static std::unordered_map <ANIMATION_FRAME_COUNT, GEOMETRY_BUFFER_ID> animation_geo_map_general = {
	{ANIMATION_FRAME_COUNT::IMMUNITY_DYING, GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_DYING},
	{ANIMATION_FRAME_COUNT::IMMUNITY_MOVING, GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_MOVING},
	{ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING, GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_BLINKING},
	{ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING, GEOMETRY_BUFFER_ID::SPRITESHEET_GREEN_ENEMY_DYING},
	{ANIMATION_FRAME_COUNT::GREEN_ENEMY_MOVING, GEOMETRY_BUFFER_ID::SPRITESHEET_GREEN_ENEMY_MOVING},
	{ANIMATION_FRAME_COUNT::CYST_SHINE, GEOMETRY_BUFFER_ID::SPRITESHEET_CYST_SHINE},
	{ANIMATION_FRAME_COUNT::DASHING, GEOMETRY_BUFFER_ID::SPRITESHEET_DASHING},
};

static std::unordered_map <ANIMATION_FRAME_COUNT, TEXTURE_ASSET_ID> animation_texture_map_general = {
	{ANIMATION_FRAME_COUNT::IMMUNITY_DYING, TEXTURE_ASSET_ID::IMMUNITY_DYING},
	{ANIMATION_FRAME_COUNT::IMMUNITY_MOVING, TEXTURE_ASSET_ID::IMMUNITY_MOVING},
	{ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING, TEXTURE_ASSET_ID::IMMUNITY_BLINKING},
	{ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING, TEXTURE_ASSET_ID::GREEN_ENEMY_DYING},
	{ANIMATION_FRAME_COUNT::GREEN_ENEMY_MOVING, TEXTURE_ASSET_ID::GREEN_ENEMY_MOVING},
	{ANIMATION_FRAME_COUNT::CYST_SHINE, TEXTURE_ASSET_ID::CYST_SHINE},
	{ANIMATION_FRAME_COUNT::DASHING, TEXTURE_ASSET_ID::DASHING},
};

static std::unordered_map <GEOMETRY_BUFFER_ID, ANIMATION_FRAME_COUNT> animation_frame_count_map = {
	{GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_DYING, ANIMATION_FRAME_COUNT::IMMUNITY_DYING},
	{GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_MOVING, ANIMATION_FRAME_COUNT::IMMUNITY_MOVING},
	{GEOMETRY_BUFFER_ID::SPRITESHEET_GREEN_ENEMY_DYING, ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING},
	{GEOMETRY_BUFFER_ID::SPRITESHEET_GREEN_ENEMY_MOVING, ANIMATION_FRAME_COUNT::GREEN_ENEMY_MOVING},
	{GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_BLINKING, ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING},
	{GEOMETRY_BUFFER_ID::SPRITESHEET_CYST_SHINE, ANIMATION_FRAME_COUNT::CYST_SHINE},
	{GEOMETRY_BUFFER_ID::SPRITESHEET_DASHING, ANIMATION_FRAME_COUNT::DASHING}
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
#pragma endregion

#pragma region Components

// Player component
struct Player
{

};

struct Camera {
	vec2 position;
};

// Enemy component
struct Enemy {
	ENEMY_ID type;
	float angle = 0.f;
	float sword_attack_cd = 0.f;
};

// Data relevant to the shape of entities
struct Transform {
	vec2 position = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
	float angle = 0.f;
	bool is_screen_coord = false;
	float angle_offset = 0.f;
};

// Data relevant to the movement of entities
struct Motion {
	vec2 velocity = { 0.f, 0.f };
	float max_velocity = 400.f;
	float acceleration_unit = 1.0f;
	float deceleration_unit = 0.995f;
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

enum class PLAYER_BELONGING_ID {
	DASHING = 0,
	SWORD = DASHING + 1,
	GUN = SWORD + 1,
	PLAYER_BELONGING_COUNT = GUN + 1,
};
const int player_belonging_count = (int)PLAYER_BELONGING_ID::PLAYER_BELONGING_COUNT;

struct PlayerBelonging {
	PLAYER_BELONGING_ID id;
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
	bool limit_fov = false;
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
	static bool loadFromOBJFile(std::string obj_path, std::vector<TexturedVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size, 
		std::vector<ColoredVertex>& out_color_vertices, bool containColorVertices);
	vec2 original_size = { 1,1 };
	std::vector<TexturedVertex> texture_vertices;
	std::vector<uint16_t> vertex_indices;
	std::vector<ColoredVertex> color_vertices;
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
	vec2 interest_point;
};

struct Health {
	float health = 100.f;
};

struct PlayerHealthbar {
	float previous_health = 100.f;
	float timer_ms = HEALTH_BAR_UPDATE_TIME_SLAP;
};

struct Invincibility {
	float timer_ms = 700.f;
};

struct Weapon {
	float damage = 10.f;
	float attack_timer = ATTACK_DELAY;
	float attack_delay = ATTACK_DELAY;
	float angle_offset = 0.0f;
	float bullet_speed = 500.f;
	vec4 color = { 1.f, 1.2f, 0.2f, 1.f };
	vec2 size = { 10.f, 10.f };
	vec2 offset = { 0.f, 0.f };
};

struct Projectile {
	float damage = 10.f;
	
};

struct Animation {
	int total_frame = 1;
	int curr_frame = 0;
	float timer_ms = 0.f;
	int update_period_ms = 30;
	bool pause_animation = false;

	float loop_interval = 0.f; // how often to repeat the animation if > 0; otherwise loop right away
};

struct NoRotate {

};

struct Dash {
	float timer_ms = 400.f;
	float active_dash_ms = 0.f;
	float dash_speed = 5.f;
	float max_dash_velocity = 2000.f;
};

struct Cyst {
	float health = 25.f;
	float sword_attack_cd = 0.f;
};

struct CollidePlayer {

};

struct CollideEnemy {

};

struct TimedEvent {
	float timer_ms = 10000.f;
	std::function<void()> callback;
};

struct MenuElem {

};

struct MenuButton {
	MENU_OPTION option;
	bool highlight;
};

struct Melee {
	vec2 offset = {0.f, 0.f};
	vec2 pivot = {0.f, 0.f};
	float damage = 20.f;
};
#pragma endregion
