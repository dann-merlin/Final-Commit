#version 330
// based on gigatrons shader from shadertoy
// https://www.shadertoy.com/view/MlK3WW
// https://creativecommons.org/licenses/by-nc/3.0/
// GIGATRON FRANCE ... some digit code based from Vox shader :Follow the white rabbit
// so thx to him ; and Amiga Rulez !!
in vec2 tc;
uniform float iTime;
uniform ivec2 iResolution;
uniform vec3 cam_pos;
uniform float soup_height;
out vec4 out_col;
bool stop = false;
vec2 tmp_coord;

#define PI 3.14159265359
#define GR 1.21803398875
#define MAX_DIM 1000
//64000
//(max(iResolution.x, iResolution.y))
#define fact_n 2.f
#define fog_speed 1.f
#define fog_brightness 1.f

#define MATRIX_W (MAX_DIM/10.0)
#define MATRIX_H (MAX_DIM/25.0)

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
//--- END MAGIC

float SampleDigit(const in float fDigit, const in vec2 vUV)
{
	if(vUV.x < 0.0) return 0.0;
	if(vUV.y < 0.0) return 0.0;
	if(vUV.x >= 1.0) return 0.0;
	if(vUV.y >= 1.0) return 0.0;

	// In this version, each digit is made up of a 4x5 array of bits

	float fDigitBinary = 0.0;

	if(fDigit < 0.5) // 0
	{
		fDigitBinary = 7.0 + 5.0 * 16.0 + 5.0 * 256.0 + 5.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 1.5) // 1
	{
		fDigitBinary = 2.0 + 2.0 * 16.0 + 2.0 * 256.0 + 2.0 * 4096.0 + 2.0 * 65536.0;
	}
	else if(fDigit < 2.5) // 2
	{
		fDigitBinary = 7.0 + 1.0 * 16.0 + 7.0 * 256.0 + 4.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 3.5) // 3
	{
		fDigitBinary = 7.0 + 4.0 * 16.0 + 7.0 * 256.0 + 4.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 4.5) // 4
	{
		fDigitBinary = 4.0 + 7.0 * 16.0 + 5.0 * 256.0 + 1.0 * 4096.0 + 1.0 * 65536.0;
	}
	else if(fDigit < 5.5) // 5
	{
		fDigitBinary = 7.0 + 4.0 * 16.0 + 7.0 * 256.0 + 1.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 6.5) // 6
	{
		fDigitBinary = 7.0 + 5.0 * 16.0 + 7.0 * 256.0 + 1.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 7.5) // 7
	{
		fDigitBinary = 4.0 + 4.0 * 16.0 + 4.0 * 256.0 + 4.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 8.5) // 8
	{
		fDigitBinary = 7.0 + 5.0 * 16.0 + 7.0 * 256.0 + 5.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 9.5) // 9
	{
		fDigitBinary = 7.0 + 4.0 * 16.0 + 7.0 * 256.0 + 5.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 10.5) // '.'
	{
		fDigitBinary = 2.0 + 0.0 * 16.0 + 0.0 * 256.0 + 0.0 * 4096.0 + 0.0 * 65536.0;
	}
	else if(fDigit < 11.5) // '-'
	{
		fDigitBinary = 0.0 + 0.0 * 16.0 + 7.0 * 256.0 + 0.0 * 4096.0 + 0.0 * 65536.0;
	}

	vec2 vPixel = floor(vUV * vec2(4.0, 5.0));
	float fIndex = vPixel.x + (vPixel.y * 4.);

	return mod(floor(fDigitBinary / pow(2.0, fIndex)), 2.0);
}

float fog_alpha() {
    // but these coordinates also work so i'll just use these
    float l = distance(cam_pos, vec3(tmp_coord.x * 1000, soup_height, tmp_coord.y * 1000));
    vec2 off = vec2(4, 7);
    float n = fog(tmp_coord + iTime/50 * off, iTime * 0.5f);
    n = pow(n, 3);
    float b = 0;
    // float d = gl_FragDepth;
	float d = 0.f;

    //only do stuff if a fragment was rasterized for that position
    // if (d != 1) {
    //     // ml and mn could be increased to make thicker fog but we kind of like the way it is:
    //     // just an ambient element that gives the scene a bit of a spooky flair
    //     b = 1 - exp(-0.075 * (l + l / 17 * n));
    // } else {
        // for the outside: use a fog with depth 8 (17 would be the maximum depth of the playing field
        // but 8 looks a lot better
        b = 1 - exp(-0.075 * (/*17*/42 + n));

        // fade out the fog at the edges
        vec2 ntc = tmp_coord;
        if (abs(ntc.x) >= 0.5) {
            b *= (1 - abs(ntc.x)) * 2;
            if (abs(ntc.y) >= 0.5) {
                b *= (1 - abs(ntc.y)) * 2;
            }
        } else if (abs(ntc.y) >= 0.5) {
            b *= (1 - abs(ntc.y)) * 2;
        }
    // }

	return n * b;
    // out_col= vec4(1, 1, 1, n * b);
    // fade out the fog close to geometry
    // float fade_depth = clamp(l / 1.7, 0, 2);
    // out_col *= vec4(fade_depth);
}

//float fog_alpha(vec2 coord) {
//    // but these coordinates also work so i'll just use these
//    vec2 off = vec2(4, 7);
//    float n = fog(coord + iTime/500 * off, iTime/2);
//    n = pow(n, 3);
//    float b = 0;
//    float d = gl_FragDepth;

//    //only do stuff if a fragment was rasterized for that position
//    // if (d != 1) {
//    //     // ml and mn could be increased to make thicker fog but we kind of like the way it is:
//    //     // just an ambient element that gives the scene a bit of a spooky flair
//    //     b = 1 - exp(-0.075 * (l + l / 17 * n));
//    // } else {
//        // for the outside: use a fog with depth 8 (17 would be the maximum depth of the playing field
//        // but 8 looks a lot better
//        b = 1 - exp(-0.075 * (/*17*/42 + n));

//        // fade out the fog at the edges
//        vec2 ntc = coord;
//        if (abs(ntc.x) >= 0.5) {
//            b *= (1 - abs(ntc.x)) * 2;
//            if (abs(ntc.y) >= 0.5) {
//                b *= (1 - abs(ntc.y)) * 2;
//            }
//        } else if (abs(ntc.y) >= 0.5) {
//            b *= (1 - abs(ntc.y)) * 2;
//        }
//    // }

//	return n * b;
//    // out_col= vec4(1, 1, 1, n * b);
//    // fade out the fog close to geometry
//    // float fade_depth = clamp(l / 1.7, 0, 2);
//    // out_col *= vec4(fade_depth);
//}


vec4 calcCol(vec2 xz){
	// out_col = vec4(0);
	// return;
	vec2 offset = vec2(iTime * 2.f * vec2(0.01f, 0.03f));
	vec2 uv = (tc + offset + vec2(1)) * 2.f;
	// out_col = vec4(uv.xy,0,1);
	// return;
	// float w=texture(iChannel0,uv).r; // get first value !
	vec2 my = fract(uv*vec2(MATRIX_W,MATRIX_H));
	vec2 bh = floor(uv*vec2(MATRIX_W,MATRIX_H));

	// permutation is based on wave sample w index !
	// (w<0.2) ? 1.5 :2.5; // alien code !!
	// (w<0.2) ? 1.5 :2.5;

	float number = (mod( 0.02 * iTime*PI*cos(bh.x-bh.y*MATRIX_W), 1.));
	float digit = SampleDigit(number, GR*my);


	float t=iTime*0.2;
	float sp=-2.;
	float ec=20.;
	float fd=0.1;

	float r=abs(sin(uv.x+uv.y*ec-(t*sp)));
	float g=abs(sin(uv.x-uv.y*ec-(t*sp)));
	float b=abs(sin(uv.x*uv.y*ec-(t*sp)));

	// float n = clamp(2 * pow(fog(10 * vec2(pos.xz + offset), 0.0005f * iTime * fog_speed),1.5),0.2,1.3);
	// float dist = distance(player_pos, pos);
	// float thickness = exp(-0.15 * (dist));
	// float alpha = (1 - thickness) * n;
	float alpha = fog_alpha();
	// out_col = vec4(1,1,1,alpha); return;
	// if(abs(pos.x) > 0.5f) out_col = vec4(0,1,0,1);
	float fog_threshold = 0.2;
	float smoothie = smoothstep(0.1, 0.3, alpha);

	float alpha_one = alpha;
	vec3 col = vec3(1);
	col *= mix(vec3(0.),vec3(r,g,b)-fd,digit);
	// alpha = round(col.x + col.y + col.z + 0.5);
	if(length(col) < 0.01) {
		alpha_one = 0;
	}
	float alpha_two = pow(alpha, 1.4);
	alpha = mix(alpha_one, alpha_two, 1 - smoothie);
	// float tmp = clamp(length(col * 10), 0, 1);
	return vec4(0, 1, 0, alpha);
	// if(length(col) > 0.01) {
	// 	col = vec3(0,0.4,0);
	// }
	// if(length(col) > 0.01) {
	// 	out_col = vec4(0,0.4,0,1);
	// } else {
	// 	out_col = vec4(0,0.4,0,0.6);
	// 	// float n = clamp(fact_n*pow(fog(4 * vec2(pos.xz + offset), 0.0005f * iTime * fog_speed),5),0,1.3);
	// 	float n = clamp(2 * pow(fog(10 * vec2(pos.xz + offset), 0.0005f * iTime * fog_speed),1.5),0.2,1.3);
	// 	float dist =
	// 	float thickness = exp(-0.15 * (dist));
	// 	out_col *= vec4(1,1,1, fog_brightness * (1 - thickness) * n);
	// }
}

void main() {
	tmp_coord = tc * 2.f - 1.f;
	vec4 my_col = calcCol(tmp_coord);
	out_col = my_col;
}
