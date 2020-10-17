#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices=4) out;

in VS_OUT {
    vec3 pos_wc;
} gs_in[];

uniform mat4 proj;
uniform mat4 view;

out vec3 wpos;
out vec2 tc;

void main() {
    float r = 0.1;

    vec3 v1 = normalize((view * gl_in[0].gl_Position).xyz);
    vec3 v2 = normalize((view * gl_in[1].gl_Position).xyz - (view* gl_in[0].gl_Position).xyz);
    //    vec3 v2 = normalize(cross(v1, vec3(1,0,0)));// "up" (to next vertex)
    vec3 v3 = normalize(cross(v1, v2));// right

    vec3 c1 = r * v3 + 0.1 * r * v2;
    vec3 c2 = -r * v3 + 0.1 * r * v2;
    vec3 c3 = r * v3;
    vec3 c4 = -r * v3;

    tc = vec2(0, 0);
	wpos = gs_in[1].pos_wc;
    gl_Position = proj * (view * gl_in[1].gl_Position + vec4(c1, 0));
    EmitVertex();

    tc = vec2(0, 1);
	wpos = gs_in[1].pos_wc;
    gl_Position = proj * (view * gl_in[1].gl_Position + vec4(c2, 0));
    EmitVertex();

    tc = vec2(1, 0);
	wpos = gs_in[0].pos_wc;
    gl_Position = proj * (view * gl_in[0].gl_Position + vec4(c3, 0));
    EmitVertex();

    tc = vec2(1, 1);
	wpos = gs_in[0].pos_wc;
    gl_Position = proj * (view * gl_in[0].gl_Position + vec4(c4, 0));
    EmitVertex();

    EndPrimitive();
}
