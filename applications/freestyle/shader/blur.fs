#version 330

in vec2 tc;
uniform sampler2D blur_buff;
uniform float velocity;
out vec4 fragColor;

#define SAMPLE_AMOUNT 10

void main() { 

	vec2 uv = tc;

	float strength = (distance(uv, vec2(0.5)) * distance(uv, vec2(0.5))) * velocity * 0.2f;

	vec2 dir = normalize(uv - vec2(0.5, 0.5)) * strength * 0.5;

	vec3 finalCol = vec3(0, 0, 0);

	for(int i = 0; i < SAMPLE_AMOUNT; ++i) {
		vec2 test_coord =  uv + dir * (float(i) / float(SAMPLE_AMOUNT));
		finalCol += test_coord.x < 0 || test_coord.y < 0 || test_coord.x > 1 || test_coord.y > 1 ? vec3(0.00) : texture( blur_buff, test_coord).rgb / float(SAMPLE_AMOUNT);
	}
	fragColor = vec4(finalCol, 1.0);
}
