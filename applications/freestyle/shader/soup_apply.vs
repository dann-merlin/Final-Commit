#version 330
in vec3 in_pos;
uniform mat4 model;
uniform mat4 model_normal;
uniform mat4 view;
uniform mat4 proj;
out vec2 tc;

void main() {
    tc = (in_pos.xz + 1) * 0.5f;
    gl_Position = proj * view * model * vec4(in_pos,1);
}
