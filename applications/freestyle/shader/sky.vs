#version 430
in vec3 in_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 out_pos;
out vec2 tc;

void main() {
    out_pos = in_pos;
	vec4 v = proj * view * model * vec4(out_pos, 1);
	// tc = v.xy;
    gl_Position = v;
}
