#version 130
//in vec2 tc;
//in vec3 wpos;
in vec3 pos_wc;
in float seg_num;
uniform vec3 player_pos;
uniform float time;
out vec4 out_col;

vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(){
	float scaling = 0.05 / 800;
	float scaled = mod((scaling * time + pos_wc.y/1000.0), 1.0);
	vec3 hsv = vec3(scaled, 1, 1);
	vec3 col = hsv2rgb(hsv);

	if (abs(mod(seg_num, 1)-0.5) > 0.4) {
		out_col = vec4(col, 1);
	} else {
		out_col = vec4(0, 0, 0, 1);
	}
}
