// internal
#include "render_system.hpp"
#include "common.hpp"
#include <SDL.h>

#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &viewProjection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;

	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED || render_request.used_effect == EFFECT_ASSET_ID::PLAYER)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::PLAYER)
		{
			GLint light_up_uloc = glGetUniformLocation(program, "isHit");
			assert(light_up_uloc >= 0);
			glUniform1i(light_up_uloc, registry.invincibility.has(entity));

			GLuint time_uloc = glGetUniformLocation(program, "time");
			glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));

			gl_has_errors();
		}
	}
	else if(render_request.used_effect == EFFECT_ASSET_ID::BULLET){
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();

	}else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint viewProjection_loc = glGetUniformLocation(currProgram, "viewProjection");
	glUniformMatrix3fv(viewProjection_loc, 1, GL_FALSE, (float *)&viewProjection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawBackground(const mat3& viewProjection)
{
	auto& region_container = registry.regions;
	for (uint i = 0; i < region_container.components.size(); i++) {
		Region& region = region_container.components[i];
		Entity entity = region_container.entities[i];
		Transform transform;
		transform.rotate(region.angle);
		vec2 scalar(1.f);
		scalar *= (MAP_RADIUS * 1.5f);
		transform.scale(scalar);

		assert(registry.renderRequests.has(entity));
		const RenderRequest& render_request = registry.renderRequests.get(entity);

		const GLuint used_effect_enum = (GLuint)render_request.used_effect;
		assert(used_effect_enum == (GLuint)EFFECT_ASSET_ID::REGION);
		const GLuint program = (GLuint)effects[used_effect_enum];

		// Setting shaders
		glUseProgram(program);
		gl_has_errors();

		assert(render_request.used_geometry == GEOMETRY_BUFFER_ID::REGION_TRIANGLE);
		const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
		const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

		// Setting vertex and index buffers
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		gl_has_errors();

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)render_request.used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		// Get number of indices from index buffer, which has elements uint16_t
		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		gl_has_errors();

		GLsizei num_indices = size / sizeof(uint16_t);
		// GLsizei num_triangles = num_indices / 3;

		// Setting uniform values to the currently bound program
		GLuint transform_loc = glGetUniformLocation(program, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
		GLuint projection_loc = glGetUniformLocation(program, "viewProjection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&viewProjection);
		GLuint mapRadius_loc = glGetUniformLocation(program, "mapRadius");
		glUniform1f(mapRadius_loc, MAP_RADIUS);
		GLuint spawnRadius_loc = glGetUniformLocation(program, "spawnRadius");
		glUniform1f(spawnRadius_loc, SPAWN_REGION_RADIUS);
		GLuint edgeThickness_loc = glGetUniformLocation(program, "edgeThickness");
		glUniform1f(edgeThickness_loc, EDGE_FADING_THICKNESS);
		gl_has_errors();
		// Drawing of num_indices/3 triangles specified in the index buffer
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
		gl_has_errors();
	}
}

void RenderSystem::drawHealthBar() {
	

	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::HEALTH_RECTANGLE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::HEALTH_RECTANGLE];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	const GLuint healthBar_program = effects[(GLuint)EFFECT_ASSET_ID::HEALTHBAR];
	GLint in_position_loc = glGetAttribLocation(healthBar_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	glUseProgram(healthBar_program);
	gl_has_errors();
	
	getPlayerHealth();
	float C_t_Percentage = playerHealth.currentHealthPercentage * playerHealth.timer_ms / HEALTH_BAR_UPDATE_TIME_SLAP + (1.0 - playerHealth.timer_ms / HEALTH_BAR_UPDATE_TIME_SLAP) * playerHealth.targetHealthPercentage;
	if (C_t_Percentage <= 0.0) C_t_Percentage = 0.0;
	//printf("C_t: %f\n", C_t);
	playerHealth.currentHealthPercentage = C_t_Percentage;
	GLint HealthBar_Length_Scale_loc = glGetUniformLocation(healthBar_program, "HealthBar_Length_Scale");
	glUniform1f(HealthBar_Length_Scale_loc, C_t_Percentage / 100.0);
	gl_has_errors();

	

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
	return;
}

void RenderSystem::drawHealthBarFrame() {
	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::HEALTHBARFRAME_RECTANGLE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::HEALTHBARFRAME_RECTANGLE];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	const GLuint healthBarFrame_program = effects[(GLuint)EFFECT_ASSET_ID::STATICWINDOW];
	GLint in_position_loc = glGetAttribLocation(healthBarFrame_program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(healthBarFrame_program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer( in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3)); 
	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	glUseProgram(healthBarFrame_program);
	gl_has_errors();

	GLuint texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::HEALTHBARFRAME];
	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	//GLint color_uloc = glGetUniformLocation(healthBarFrame_program, "fcolor");
	//const vec3 color = vec3(1);
	//glUniform3fv(color_uloc, 1, (float*)&color);
	//gl_has_errors();


	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
	return;
}

// draw the intermediate texture to the screen
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the screen texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::SCREEN]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0, 0, 0, 1);	// black default background
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint screen_program = effects[(GLuint)EFFECT_ASSET_ID::SCREEN];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(screen_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(screen_program, "screen_darken_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.screen_darken_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(screen_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1);	// black default background
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
	mat3 view = createViewMatrix();

	// For testing, delete feature flag later
	bool playerCamera = true;

	mat3 viewProjection = playerCamera ? projection_2D * view : projection_2D;

	// Draw background
	drawBackground(viewProjection);

	// Draw all textured meshes that have a position and size component
	for (Entity entity : registry.renderRequests.entities)
	{
		if (!registry.motions.has(entity))
			continue;
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize
		
		// ################# TODO #################
		// can cull entities here if outside of camera
		// if not it should still culll during fragment shader?

		drawTexturedMesh(entity, viewProjection);
	}


	//Draw Fixed screen items
	drawHealthBar();
	drawHealthBarFrame();

	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// Orthographic Projection
// basically converts Viewing Coordinate System -> Device Coordinate System
mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_width_px;
	float bottom = (float) window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	//float tx = -(right + left) / (right - left);
	//float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {0.f, 0.f, 1.f}};
}

mat3 RenderSystem::createViewMatrix()
{
	offset = vec2(0.f, 0.f);

	auto& motion_container = registry.motions;
	for (uint i = 0; i < motion_container.size(); i++)
	{
		Entity entity = motion_container.entities[i];
		if (registry.players.has(entity))
		{
			Motion& motion = motion_container.components[i];
			offset.x = -motion.position.x;
			offset.y = motion.position.y;
			break;
		}
	}
	return { {1.f, 0.f, 0.f}, {0.f, -1.f, 0.f}, {offset.x, offset.y, 1.f} };
}

void RenderSystem::getPlayerHealth() {
	auto& health_container = registry.healthValues;
	for (uint i = 0; i < health_container.size(); i++)
	{
		Entity entity = health_container.entities[i];
		if (registry.players.has(entity))
		{
			
			playerHealth = health_container.components[i];
			//return true;
			//return health_container.components[i];
		}
	}
	//return false;
	//throw std::invalid_argument("No health value links to the player entity");
}