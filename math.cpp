#include "defs.h"

vec2 _vec2(float x, float y) {
	vec2 a = {x, y};
	return a;
}
vec3 _vec3(float x, float y, float z) {
	vec3 a = {x, y, z};
	return a;
}
vec2 vec2Add(vec2 a, vec2 b) {
	return _vec2(a.x+b.x, a.y+b.y);
}
vec2 vec2Mul(vec2 a, vec2 b) {
	return _vec2(a.x*b.x, a.y*b.y);
}
vec2 vec2Sub(vec2 a, vec2 b) {
	return _vec2(a.x-b.x, a.y-b.y);
}

int ipow(int num, int e) {
	while(e>1) num*=num;
	return num;
}
float randf() {
	return (float)rand() / RAND_MAX;
}
float randfr(float min, float max) {
	return min + randf()*(max-min);
}
float min(float a, float b) {
	return a<b ? a : b;
}
float max(float a, float b) {
	return a>b ? a : b;
}
int imin(int a, int b) {
	return a<b ? a : b;
}
int imax(int a, int b) {
	return a>b ? a : b;
}
float diff(float a, float b) {
	return abs(a-b);
}
// vec2 diff2(vec2 a, vec2 b) {
// 	return abs(a-b);
// }
float len(float x, float y) {
	return sqrt(x*x + y*y);
}
float len2(vec2 a) {
	return sqrt(a.x*a.x + a.y*a.y);
}
vec2 normalize2(vec2 v) {
	float l = len(v.x, v.y);
	return _vec2(v.x/l, v.y/l);
}
float clamp(float a, float minimum, float maximum) {
	return min(max(a, minimum), maximum);
}
int clampi(int a, int minimum, int maximum) {
	return min(max(a, minimum), maximum);
}
float smoothstep(float x, float y, float a) {
	return clamp((a-x)/(y-x), 0.0, 1.0);
}
float mix(float x, float y, float a) {
	float t = x + (y-x)*a;//(a-x)/(y-x);
	return t;
}
vec3 mix3(vec3 a, vec3 b, float t) {
	vec3 m = _vec3(
		a.x + (b.x-a.x)*t,
		a.y + (b.y-a.y)*t,
		a.z + (b.z-a.z)*t
	);
	return m;
}
vec2 floor2(vec2 a) {
	return _vec2(floorf(a.x), floorf(a.y));
}
float fract(float a) {
	return a-floorf(a);
}
vec2 fract2(vec2 a) {
	vec2 f = {a.x-floorf(a.x), a.y-floorf(a.y)};
	return f;
}
float dot2(vec2 a, vec2 b) {
	return a.x*b.x + a.y*b.y;
}

vec3 decodeIntColor(int color) {
	vec3 a;
	a.x = (float)((color & 0xFF0000) >> 16) / 255.0;
	a.y = (float)((color & 0x00FF00) >> 8) / 255.0;
	a.z = (float)((color & 0x0000FF)) / 255.0;
	return a;
}
int encodeIntColor(vec3 c) {
	return (int)(c.x*255)<<16 | (int)(c.y*255)<<8 | (int)(c.z*255);
}

float rand2d(vec2 st) {
    return fract(sinf(dot2(st, _vec2(12.9898,78.233)))*43758.5453123);
}
float noise (vec2 st) {
    vec2 i = floor2(st);
    vec2 f = fract2(st);
	
    // Four corners in 2D of a tile
    float a = rand2d(i);
    float b = rand2d(vec2Add(i, _vec2(1.0, 0.0)));
    float c = rand2d(vec2Add(i, _vec2(0.0, 1.0)));
    float d = rand2d(vec2Add(i, _vec2(1.0, 1.0)));
	
    //vec2 u = vec2Mul(f, vec2Mul(f, (vec2Sub(_vec2(3.0,3.0), vec2Mul(_vec2(2.0,2.0), f)))));
	vec2 u = f;
	//return a;
	
    //return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
	return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}
float fbm (vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
#define OCTAVES 6
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st = vec2Mul(st, _vec2(2,2));
        amplitude *= .5;
    }
    return value;
	// return rand2d(_vec2(-1.5, -1.0));
	// return noise(st);
}
