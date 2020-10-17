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
uniform sampler2D specular;
uniform float upper_limit = 20;
uniform bool wire = false;

out vec4 out_col;

void main() {
	if (wire) {
		if (pos_mo.z < upper_limit) discard;
		out_col = vec4(0, 1, 0, 1);
		return;
	}
	if (pos_mo.z > upper_limit) discard;

	vec3 amb = ambient_col + vec3(0.2);
	vec3 N = normalize(norm_wc);
	// ambient and diffuse
	float n_dot_l = max(0, dot(N, -light_dir));
	vec3 diff = texture(diffuse, tc).rgb;
	out_col = vec4(diff * (amb + light_col * n_dot_l), 1);
	// specular
	vec3 v = normalize(cam_pos - pos_wc.xyz);
	float n_dot_h = max(0, dot(N, normalize(-light_dir + v)));
	out_col.xyz += pow(n_dot_h, 200) * texture(specular, tc).rgb; //vec3(1);

	amb = vec3(0,0.2,0);
	// ambient and diffuse
	n_dot_l = max(0, dot(N, vec3(0, -1, 0)));
	diff = texture(diffuse, tc).rgb;
	vec4 floor_col = vec4(diff * (amb + vec3(0, 1, 1) * n_dot_l), 1);
	// specular
	v = normalize(cam_pos - pos_wc.xyz);
	n_dot_h = max(0, dot(N, normalize(vec3(0, -1, 0) + v)));
	floor_col.xyz += pow(n_dot_h, 200) * texture(specular, tc).rgb; //vec3(0, 1, 1);

	out_col += clamp(upper_limit/2, 0, 0.5)* floor_col;
}
