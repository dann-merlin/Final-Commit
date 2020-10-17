#version 430
// based on gigatrons shader from shadertoy
// https://www.shadertoy.com/view/MlK3WW
// https://creativecommons.org/licenses/by-nc/3.0/
// GIGATRON FRANCE ... some digit code based from Vox shader :Follow the white rabbit
// so thx to him ; and Amiga Rulez !!
in vec2 tc;
uniform sampler2D tex;
uniform float iTime;
uniform float soup_height;
uniform vec3 cam_pos;
out vec4 out_col;

#define fact_n 2.f
#define fog_speed 1.f
#define fog_brightness 1.f

#define MATRIX_W (MAX_DIM/10.0)
#define MATRIX_H (MAX_DIM/25.0)

vec2 tmp_coord;

//--- BEGIN MAGIC
float random(in vec2 _st) { return fract(sin(dot(_st.xy, vec2(12.9898, 78.233)))* 43758.5453123); }
// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 _st) {
    vec2 i = floor(_st);
    vec2 f = fract(_st);
    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}


#define NUM_OCTAVES 7
float fbm(in vec2 _st) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(_st);
        _st = rot * _st * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}
float fog(in vec2 st, in float time) {
    vec2 q = vec2(fbm(st + 0.00*time),
            fbm(st + vec2(1.0)));
    vec2 r = vec2(fbm(st + 1.0*q + vec2(1.7,9.2)+ 0.15*time),
            fbm(st + 1.0*q + vec2(8.3,2.8)+ 0.126*time));
    float f = fbm(st+r);
    return sqrt(f*f*f + .6*f*f + .5*f);
}

float fog_alpha() {

    float l = distance(cam_pos, vec3(tmp_coord.x * 1000, soup_height, tmp_coord.y * 1000));
    vec2 off = vec2(4, 7);
    float n = fog(tmp_coord + iTime/50 * off, iTime * 0.5f);
    n = pow(n, 3);
    float b = 0;
	float d = 0.f;

	b = 1 - exp(-0.075 * (/*17*/42 + n));

	vec2 ntc = tmp_coord;
	if (abs(ntc.x) >= 0.5) {
		b *= (1 - abs(ntc.x)) * 2;
		if (abs(ntc.y) >= 0.5) {
			b *= (1 - abs(ntc.y)) * 2;
		}
	} else if (abs(ntc.y) >= 0.5) {
		b *= (1 - abs(ntc.y)) * 2;
	}

	return n * b;
}



void main() {
	tmp_coord = tc * 2.f - 1.f;
	out_col = texture(tex, tc);
	float alpha = pow(length(out_col.xyz), 0.2);
	out_col.a = alpha;
}
