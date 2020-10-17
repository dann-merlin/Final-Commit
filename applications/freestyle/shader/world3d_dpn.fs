#version 330
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;

uniform sampler2D diffuse;
uniform float time;
out vec4 out_col;
out vec4 out_pos;
out vec4 out_norm;

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	// float scaling = pos_wc.y / 800;
	float scaling = 0.05 / 800;
	float scaled = mod((scaling * time + pos_wc.y/1000.0), 1.0);
	vec3 hsv = vec3(scaled, 1, 1);
	vec3 col =  hsv2rgb(hsv);
	// vec4 col = vec4(tc.y-tc.x, tc/2, 1)*4;
	// col = vec4(0,pos_wc.y/800.0 * mod(time, 30000.0f),0,1);
	out_col = texture(diffuse, tc) * vec4(col, 1);
	// out_col = vec4(texture(diffuse, tc).rgb,1);
	out_pos = pos_wc;
	out_norm = vec4(norm_wc, 1);
}
