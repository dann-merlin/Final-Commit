#version 330
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;

uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec3 cam_pos;
uniform sampler2D diffuse;

out vec4 out_col;

void main() {
	vec3 amb = ambient_col + vec3(0.2);
	vec3 N = normalize(norm_wc);
	// ambient and diffuse
	float n_dot_l = max(0, dot(N, -light_dir));
	vec3 diff = texture(diffuse, tc).rgb;
	out_col = vec4(diff * (amb + light_col * n_dot_l), 1);
	// specular
	vec3 v = normalize(cam_pos - pos_wc.xyz);
	float n_dot_h = max(0, dot(N, normalize(-light_dir + v)));
	out_col.xyz += pow(n_dot_h, 200) * vec3(1);
}