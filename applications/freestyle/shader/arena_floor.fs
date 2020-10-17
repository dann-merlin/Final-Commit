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
	vec4 col = vec4(tc.y-tc.x, tc/2, 1)*4;
	out_col = texture(diffuse, pos_wc.xz*0.5) * col;
}