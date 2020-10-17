#version 330
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;

uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec3 cam_pos;
uniform sampler2D diffuse;
uniform sampler2D specular;
uniform float time = 2;

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
	out_col.xyz += pow(n_dot_h, 200) * texture(specular, tc).rgb;


	amb = vec3(0, 1, 1) + vec3(0.2);
	amb = vec3(0, 0.2, 0);
	// ambient and diffuse
	n_dot_l = max(0, dot(N, vec3(0, -1, 0)));
	diff = texture(diffuse, tc).rgb;
	vec4 floor_col = vec4(diff * (amb + vec3(0, 1, 1) * n_dot_l), 1);
	// specular
	v = normalize(cam_pos - pos_wc.xyz);
	n_dot_h = max(0, dot(N, normalize(vec3(0, -1, 0) + v)));
	floor_col.xyz += pow(n_dot_h, 200) * texture(specular, tc).rgb;

	out_col += 0.5* floor_col;

	float factor =  time * 0.002 - 2.0;
	factor = clamp(factor, 0, 1);
	out_col = mix(out_col, vec4(-N.x, N.y, N.z, 1), factor);
}