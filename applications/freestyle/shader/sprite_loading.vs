#version 330

const int MAX_BONES = 64;
const int MAX_WEIGHTS = 4;

in vec3 in_pos;
in vec3 in_norm;
in vec2 in_tc;
in ivec4 in_bone_ids;
in vec4 in_bone_weights;
uniform mat4 model;
uniform mat4 model_normal;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 bone_transforms[MAX_BONES];
uniform mat4 default_trafo = mat4(1);
out vec4 pos_wc;
out vec3 norm_wc;
out vec2 tc;
flat out ivec4 bone_ids;
out vec4 bone_weights;
out vec3 pos_mo;

void main() {
	float angle = radians(90.0);
	float s = sin(angle);
	float c = cos(angle);
	//    mat4 rot = mat4(vec4(1,0,0,0),vec4(0,c,s,0),vec4(0,-s,c,0),vec4(0,0,0,1));
	mat4 total_trafo = mat4(0);
	for (int i = 0; i < MAX_WEIGHTS; i++) {
		mat4 bone_trafo = bone_transforms[in_bone_ids[i]];
		total_trafo += in_bone_weights[i] * bone_trafo;
	}
	pos_wc = model * default_trafo * total_trafo * vec4(in_pos.xyz, 1);
	//    pos_wc = pos_wc.xzyw;
	norm_wc = vec3(model * default_trafo * total_trafo * vec4(in_norm.xyz, 0));
	tc = in_tc;
	bone_ids = in_bone_ids;
	bone_weights = in_bone_weights;
	pos_mo = in_pos;
	gl_Position = proj * view * pos_wc;
}
