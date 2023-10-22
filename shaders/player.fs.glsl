#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int isHit;
uniform float time;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
	if (isHit == 1.0)
	{
		if (1.0 < mod(time, 2.0)) {
			color.a = 0.0;
		}
	}
}
