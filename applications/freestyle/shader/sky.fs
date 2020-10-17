#version 430

in vec3 out_pos;

uniform samplerCube cube_texture;

out vec4 out_color;

void main() {
    out_color = 0.2*texture(cube_texture, out_pos);
//    out_color = vec4(0);
}
