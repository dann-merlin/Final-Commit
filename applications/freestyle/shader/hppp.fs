#version 330

const int MAX_BONES = 64;
const int MAX_WEIGHTS = 4;

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
uniform float layer_max = 1;
uniform float layer_min = -2;

uniform mat4 bone_transforms[MAX_BONES];

out vec4 out_col;

void main() {
	vec3 amb = ambient_col + vec3(0.2);
	vec3 N = normalize(norm_wc);
	// ambient and diffuse
	float n_dot_l = max(0, dot(N, -light_dir));
	vec3 diff = texture(diffuse, tc).rgb;
	out_col = vec4(diff * (amb + light_col * n_dot_l), 1);
}
