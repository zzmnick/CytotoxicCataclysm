#version 330

// From vertex shader
in vec2 texcoord;
in vec2 vertcoord;
in vec2 worldcoord;

// Application data
uniform sampler2D sampler0;
uniform float spawnRadius;
uniform float mapRadius;
uniform float edgeThickness;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	float brightness = 0.9f;
	float origin_dist = length(worldcoord);
	if (origin_dist > mapRadius) {
		discard;
	}

	if (vertcoord.x < edgeThickness) {
		brightness = 0.01f;
	}
	if (vertcoord.y < edgeThickness) {
		brightness = 0.01f;
	}
	if (origin_dist < spawnRadius) {
		brightness = min(brightness, pow(origin_dist / spawnRadius, 3));
	}
	if (brightness < 0.01f) {
		discard;
	}
	color = texture(sampler0, vec2(texcoord.x, texcoord.y));
	color *= brightness;
}
