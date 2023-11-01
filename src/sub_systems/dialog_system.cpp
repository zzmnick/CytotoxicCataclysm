// internal
#include "dialog_system.hpp"
#include "world_init.hpp"

const float ACTION_DELAY = 3000.f;	// Time during which user is trying the action

DialogSystem::DialogSystem(std::unordered_map<int, int>& keys_pressed, const vec2& mouse)
	: keys_pressed(keys_pressed), mouse(mouse) {
	rendered_entity = Entity();
	current_dialog_idx = 0;
	current_status = DIALOG_STATUS::DISPLAY;

	populate_dialogs();
}

void DialogSystem::add_regular_dialog(TEXTURE_ASSET_ID asset) {
	dialogs.push_back({
		asset, 0.f, {0,0},
		[this]() {
			return ((std::unordered_map<int, int>&) this->keys_pressed)[GLFW_MOUSE_BUTTON_LEFT];
		}
		});
}

void DialogSystem::add_tutorial_dialogs() {
	dialogs.push_back({
		TEXTURE_ASSET_ID::TUTORIAL_MOVEMENT, ACTION_DELAY, {0,0},
		[this]() {
			auto keys = (std::unordered_map<int, int>&) this->keys_pressed;
			return (keys[GLFW_KEY_W] || keys[GLFW_KEY_A] || keys[GLFW_KEY_S] || keys[GLFW_KEY_D]);
		}
		});
	dialogs.push_back({
		TEXTURE_ASSET_ID::TUTORIAL_ROTATE, ACTION_DELAY, {0,0},
		[this]() {
			return (length(this->mouse - this->previous_mouse) > 300.f);
		}
		});
	dialogs.push_back({
		TEXTURE_ASSET_ID::TUTORIAL_SHOOT, ACTION_DELAY, {0,0},
		[this]() {
			return ((std::unordered_map<int, int>&) this->keys_pressed)[GLFW_MOUSE_BUTTON_LEFT];
		}
		});
	dialogs.push_back({
		TEXTURE_ASSET_ID::TUTORIAL_PAUSE, 0.f, {0,0},
		[this]() {
			return ((std::unordered_map<int, int>&) this->keys_pressed)[GLFW_KEY_ESCAPE];
		}
		});
	add_regular_dialog(TEXTURE_ASSET_ID::TUTORIAL_END);
}

void DialogSystem::populate_dialogs() {
	add_regular_dialog(TEXTURE_ASSET_ID::DIALOG_INTRO1);
	add_regular_dialog(TEXTURE_ASSET_ID::DIALOG_INTRO2);
	add_regular_dialog(TEXTURE_ASSET_ID::DIALOG_INTRO3);
	add_tutorial_dialogs();
}

DialogSystem::~DialogSystem() {
	registry.remove_all_components_of(rendered_entity);
};

bool DialogSystem::is_finished() {
	return current_dialog_idx == dialogs.size();
}

bool DialogSystem::is_paused() {
	return current_status == DIALOG_STATUS::ACTION_TIMER;
}

bool DialogSystem::step(float elapsed_ms) {
	if (!is_finished()) {
		Stage& current_stage = dialogs[current_dialog_idx];
		switch (current_status) {
		case (DIALOG_STATUS::DISPLAY): {
			Transform& transform = registry.transforms.emplace(rendered_entity);
			transform.position = current_stage.instruction_position;
			transform.scale = { DIALOG_TEXTURE_SIZE.x, DIALOG_TEXTURE_SIZE.y };
			transform.is_screen_coord = true;

			registry.renderRequests.insert(
				rendered_entity,
				{ dialogs[current_dialog_idx].instruction_asset,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE,
					RENDER_ORDER::UI });

			keys_pressed.clear();
			current_status = DIALOG_STATUS::AWAIT_ACTION;
			return false;
			break;
		}
		case DIALOG_STATUS::AWAIT_ACTION:
			if (current_stage.is_action_performed()) {
				registry.transforms.remove(rendered_entity);
				registry.renderRequests.remove(rendered_entity);
				current_status = DIALOG_STATUS::ACTION_TIMER;
			}
			return false;
			break;
		case DIALOG_STATUS::ACTION_TIMER:
			current_stage.action_timer -= elapsed_ms;
			if (current_stage.action_timer <= 0) {
				current_status = DIALOG_STATUS::DISPLAY;
				current_dialog_idx++;
			}
			break;
		default:
			break;
		}
		previous_mouse = mouse;
	}
	return true;
}
