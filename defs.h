#include <stdlib.h>
#include <math.h>

#undef min
#undef max

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define FRAMEBUFFER_WIDTH (WINDOW_WIDTH/1)
#define FRAMEBUFFER_HEIGHT (WINDOW_HEIGHT/1)

#define WORLD_TOP 100
#define WORLD_RIGHT (100.0*((float)FRAMEBUFFER_WIDTH/(float)FRAMEBUFFER_HEIGHT))
#define WORLD_BOTTOM -100
#define WORLD_LEFT -(100.0*((float)FRAMEBUFFER_WIDTH/(float)FRAMEBUFFER_HEIGHT))
#define WORLD_HEIGHT (WORLD_TOP*2)
#define WORLD_WIDTH (WORLD_RIGHT*2)

#define array_size(arr) (sizeof(arr)/sizeof(arr[0]))

#define PI (3.14159265359f)
#define PIHALF (PI/2.0f)

typedef __int64  int64_t;
typedef unsigned __int64  uint64_t;
typedef __int32  int32_t;
typedef unsigned __int32  uint32_t;
typedef __int16  int16_t;
typedef unsigned __int16  uint16_t;

typedef struct vec2 {
	float x;
	float y;
} vec2;
typedef struct vec3 {
	float x;
	float y;
	float z;
// 	vec3(float a, float b, float c) {
// 		x = a;
// 		y = b;
// 		z = c;
// 	}
} vec3;

double getTime();

vec2 _vec2(float x, float y);
vec3 _vec3(float x, float y, float z);
vec2 vec2Add(vec2 a, vec2 b);
vec2 vec2Mul(vec2 a, vec2 b);
vec2 vec2Sub(vec2 a, vec2 b);

int ipow(int num, int e);
float randf();
float randfr(float min, float max);
float min(float a, float b);
float max(float a, float b);
int imin(int a, int b);
int imax(int a, int b);
float diff(float a, float b);
float len(float x, float y);
float len2(vec2 a);
vec2 normalize2(vec2 v);
float clamp(float a, float minimum, float maximum);
int clampi(int a, int minimum, int maximum);
float smoothstep(float x, float y, float a);
float mix(float x, float y, float a);
vec3 mix3(vec3 a, vec3 b, float t);
vec2 floor2(vec2 a);
float fract(float a);
vec2 fract2(vec2 a);
float dot2(vec2 a, vec2 b);

vec3 decodeIntColor(int color);
int encodeIntColor(vec3 c);

float rand2d(vec2 st);
float noise (vec2 st);
float fbm (vec2 st);

float perlin2d(float x, float y, float freq, int depth);

void gameStart(unsigned int* framebuffer);
void gameUpdate();
void gameRender(unsigned int* framebuffer);
