#version 130
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out vec4 out_col;
uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec3 cam_pos;
uniform sampler2D diffuse;
uniform sampler2D normalmap;
uniform sampler2D specular;

vec3 align(in vec3 axis, in vec3 v) {
    float s = sign(axis.z + 0.001f);
    vec3 w = vec3(v.x, v.y, v.z * s);
    vec3 h = vec3(axis.x, axis.y, axis.z + s);
    float k = dot(w, h) / (1.0f + abs(axis.z));
    return k * h - w;
}

void main() {
    // vec3 N = align(norm_wc, 2 * texture(normalmap, tc).xyz - 1);
    // ambient and diffuse
    // float n_dot_l = max(0, dot(N, -light_dir));
    vec4 diff = texture(diffuse, tc);
    // out_col = vec4(diff.rgb * (ambient_col + light_col * n_dot_l), diff.a);
    
    // // specular
    // vec3 v = normalize(cam_pos - pos_wc.xyz);
    // float n_dot_h = max(0, dot(N, normalize(-light_dir + v)));
    // out_col.xyz += pow(n_dot_h, 200) * texture(specular, tc).rgb;

    //     out_col = out_col ;
    out_col = diff; 
    out_col = pos_wc - vec4(0,10,0,0);
    out_col = vec4(norm_wc, 1);
}
