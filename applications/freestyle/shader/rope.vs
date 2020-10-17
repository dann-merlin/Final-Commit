#version 330
in vec3 in_pos;
in float in_seg_num;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

//out VS_OUT {
//    vec3 pos_wc;
//} vs_out;

out vec3 pos_wc;
out float seg_num;

void main(){
    pos_wc = vec3(model * vec4(in_pos, 1));
//    vs_out.pos_wc = pos_wc;
    gl_Position = proj * view * vec4(pos_wc, 1);
    seg_num = in_seg_num;
}
