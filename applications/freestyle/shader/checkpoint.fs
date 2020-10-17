#version 130
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out vec4 out_col;
uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec3 cam_pos;
uniform vec3 cp_pos;
uniform sampler2D diffuse;
uniform sampler2D normalmap;
uniform sampler2D specular;


void main() {
	vec4 diff = texture(diffuse, tc);
	out_col = diff;
	float fade = 1 - (pos_wc.y - cp_pos.y) / 6.0;
	out_col = vec4(0,1,1,fade * fade);
}
