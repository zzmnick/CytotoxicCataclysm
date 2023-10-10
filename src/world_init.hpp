#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hard coded to the dimensions of the entity texture
const float IMMUNITY_BB_WIDTH = 2.0f * 60.f;
const float IMMUNITY_BB_HEIGHT = 2.0f * 60.f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// the random regions
void createRandomRegion(RenderSystem* renderer, size_t num_regions);
