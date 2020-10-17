#version 330
// based on https://www.shadertoy.com/view/MltBzf by boysx
// by default licensed with https://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US
//

in vec2 tc;
uniform sampler2D tex;
uniform ivec2 iResolution;
uniform float iTime;
out vec4 out_col;

float rand(vec2 p)
{
    float t = floor(iTime * 0.01) / 100.;
    return fract(sin(dot(p, vec2(t * 12.9898, t * 78.233))) * 43758.5453);
}

float noise(vec2 uv, float blockiness)
{
    vec2 lv = fract(uv);
    vec2 id = floor(uv);

    float n1 = rand(id);
    float n2 = rand(id+vec2(1,0));
    float n3 = rand(id+vec2(0,1));
    float n4 = rand(id+vec2(1,1));

    vec2 u = smoothstep(0.0, 1.0 + blockiness, lv);

    return mix(mix(n1, n2, u.x), mix(n3, n4, u.x), u.y);
}

float fbm(vec2 uv, int count, float blockiness, float complexity)
{
    float val = 0.0;
    float amp = 0.5;

    while(count != 0)
    {
    	val += amp * noise(uv, blockiness);
        amp *= 0.5;
        uv *= complexity;
        count--;
    }

    return val;
}

const float glitchAmplitude = 0.2; // increase this
const float glitchNarrowness = 4.0;
const float glitchBlockiness = 2.0;
const float glitchMinimizer = 5.0; // decrease this

void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = tc;
    vec2 a = vec2(uv.x * (iResolution.x / iResolution.y), uv.y);
    vec2 uv2 = vec2(a.x / iResolution.x, exp(a.y));
	vec2 id = floor(uv * 8.0);
    //id.x /= floor(texture(tex, vec2(id / 8.0)).r * 8.0);

    // Generate shift amplitude
    float shift = glitchAmplitude * pow(fbm(uv2, int(rand(id) * 6.), glitchBlockiness, glitchNarrowness), glitchMinimizer);

    // Apply glitch and RGB shift
    float colR = texture(tex, vec2(uv.x + shift, uv.y)).r * (1. - shift) ;
    float colG = texture(tex, vec2(uv.x - shift, uv.y)).g * (1. - shift) + rand(id) * shift;
    float colB = texture(tex, vec2(uv.x - shift, uv.y)).b * (1. - shift);
    vec3 f = vec3(colR, colG, colB);

    // Output to screen
    out_col = vec4(f, round(f.x+f.y+f.z));
}
