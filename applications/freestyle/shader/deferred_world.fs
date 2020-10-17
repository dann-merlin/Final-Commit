#version 130
in vec2 tc;

uniform sampler2D gbuf_depth;
uniform sampler2D gbuf_diff;
uniform sampler2D gbuf_pos;
uniform sampler2D gbuf_norm;
uniform vec2 screenres;
uniform vec3 player_pos;
uniform vec3 cam_pos;

out vec4 out_col;
out vec4 out_pos;
out vec4 out_norm;

void main() {
    float depth = texture(gbuf_depth, tc).r;
    gl_FragDepth = depth;
    out_col = texture(gbuf_diff, tc);
    out_pos = texture(gbuf_pos, tc);
    out_norm = texture(gbuf_norm, tc);
    if(out_pos.y < player_pos.y && distance(out_pos.xz, player_pos.xz) < 1.75) {
        out_col *= 0.5;
    }
}