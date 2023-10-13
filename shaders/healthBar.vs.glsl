#version 330

// Input attributes
in vec3 in_position;

uniform float HealthBar_Length_Scale;

out float warning;

void main()
{
	float trans_offset_x = (1.0 - HealthBar_Length_Scale)/2.0;
	gl_Position = vec4(in_position.x*HealthBar_Length_Scale - trans_offset_x, in_position.y , in_position.z, 1.0);

	if(HealthBar_Length_Scale < 0.5){
		warning = 1.0;
	}else{
		warning = -1.0;
	}
}