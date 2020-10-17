#version 330
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
flat in ivec4 bone;
in vec4 bone_weigh;
in vec3 pos_mo;

uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec3 cam_pos;
uniform sampler2D diffuse;
uniform float upper_limit = 20;
uniform bool wire = false;
uniform bool floor = false;

out vec4 out_col;

void main() {
	if (wire) {
		if (pos_mo.z < upper_limit) discard;
		out_col = vec4(0, 1, 0, 1);
		return;
	}

	if (floor) {
		if (pos_mo.x > upper_limit) discard;
		vec4 col = vec4(tc.y-tc.x, tc/2, 1)*4;
		out_col = texture(diffuse, pos_wc.xz*0.5) * col;
	} else {
		if (pos_mo.z > upper_limit) discard;
		if (pos_wc.y < -0.2) discard;
		out_col = vec4(0, 1, 0, 1);
		vec4 diff = texture(diffuse, tc);
		vec4 modulator = mix(vec4(0, 1, 0, 1), vec4(0, 1, 1, 0), pos_wc.y/10);
		out_col = diff * modulator;
	}
}
