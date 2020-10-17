#version 330

in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
flat in ivec4 bone_ids;
in vec4 bone_weights;

uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec3 cam_pos;
uniform sampler2D diffuse;
uniform float depth;
uniform float alpha;

out vec4 out_col;

void main() {
	if (pos_wc.y < -0.2) discard;
	out_col = vec4(0, 1, 0, 1);
	vec4 diff = texture(diffuse, tc);
	vec4 modulator = mix(vec4(0,1,0,1), vec4(0,1,1,0), pos_wc.y/10);
	out_col = diff * modulator;
}