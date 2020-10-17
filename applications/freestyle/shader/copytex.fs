#version 330
in vec2 tc;
uniform sampler2D tex;
out vec4 out_col;
void main() {
	vec3 c = texture(tex, tc).rgb;
    out_col = vec4(c,1);
}
