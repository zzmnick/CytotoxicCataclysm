#pragma once

// internal
#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// stlib
#include <unordered_map>

enum class DIALOG_STATUS {
	DISPLAY = 0,
	AWAIT_ACTION = DISPLAY + 1,
	ACTION_TIMER = AWAIT_ACTION + 1
};


struct Stage {
	TEXTURE_ASSET_ID instruction_asset;
	float action_timer = 0.f;
	vec2 instruction_position{ 0, 0 };
	// a lambda function which checks if required actions is performed
	// default action is to do left click
	std::function<bool()> is_action_performed = []() {
		throw std::runtime_error("Tutorial action not implemented.");
		return true;
		};
};

class DialogSystem {
public:
	DialogSystem(std::unordered_map<int, int>& keys_pressed, const vec2& mouse);
	~DialogSystem();
	bool is_finished();
	bool is_paused();
	bool step(float elapsed_ms);

private:
	std::vector<Stage> dialogs;
	std::unordered_map<int, int>& keys_pressed;
	const vec2& mouse;
	vec2 previous_mouse = { 0,0 };
	const float ACTION_DELAY = 3000.f;	// Time during which user is trying the action
	uint current_dialog_idx;
	DIALOG_STATUS current_status;
	Entity rendered_entity;

	void add_regular_dialog(TEXTURE_ASSET_ID asset);
	void add_tutorial_dialogs();
	void populate_dialogs();
};