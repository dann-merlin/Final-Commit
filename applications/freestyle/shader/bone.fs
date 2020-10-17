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
uniform sampler2D normalmap;
uniform sampler2D specular;
uniform float depth;
uniform float alpha;
uniform mat4 bone_transforms[MAX_BONES];

out vec4 out_col;

vec3 align(in vec3 axis, in vec3 v) {
    float s = sign(axis.z + 0.001f);
    vec3 w = vec3(v.x, v.y, v.z * s);
    vec3 h = vec3(axis.x, axis.y, axis.z + s);
    float k = dot(w, h) / (1.0f + abs(axis.z));
    return k * h - w;
}

void main() {
    vec4 color = bone_weights;
    ivec4 b1v = bone_ids;
    int b1 = bone_ids.r;
    int b2= bone_ids.g;
    out_col = color;
    out_col = vec4(0.2);
    out_col += vec4(b1, b2, 0, 1);
    out_col = vec4(0);
    if (b1 == 0) {
        out_col = vec4(color.r, color.g, 0, 1);
    } else {
        out_col = vec4(0, color.r, color.g, 1);
    }
    //    out_col = pos_wc;
    //    if(pos_wc.y > 1){
    //        out_col = vec4(1,0,1,1);
    //    }
    out_col = vec4(normalize(norm_wc), 1);
    //    out_col = pos_wc;
    //    out_col = vec4(0.2);

    //    out_col = vec4(0,1,0,1);
    if (pos_wc.y < 0.01){
//        discard;
        out_col = vec4(0.3);
    }
//        out_col = texture(diffuse, tc);
}