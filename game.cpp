// #include <windows.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>

#include "defs.h"

typedef struct {
	vec2 pos;
	vec2 target;
	float dir;
	float life;
	float hunger;
	float horny;

	float waterAffinity;
	float speed;
} creature;
typedef struct {
	vec2 pos;
	float growing;
} food;

unsigned int staticTerrainBackground[FRAMEBUFFER_WIDTH*FRAMEBUFFER_HEIGHT];

const int TERRAIN_WIDTH = WORLD_WIDTH/1;
const int TERRAIN_HEIGHT = WORLD_HEIGHT/1;
unsigned char terrain[TERRAIN_HEIGHT*TERRAIN_WIDTH];
//#define CREATURES_MAX 20
creature creatures[1000];
int numCreatures = 0;
food foods[500];
int numFoods = 0;

double foodSpawnTimer=0;

vec2 convertCoordsFbToWorld(int x, int y) {
	float xx = ((float)x-(FRAMEBUFFER_WIDTH/2))*((float)WORLD_RIGHT/(FRAMEBUFFER_WIDTH/2));
	float yy = ((float)y-(FRAMEBUFFER_HEIGHT/2))*((float)WORLD_TOP/(FRAMEBUFFER_HEIGHT/2));
	return _vec2(xx, yy);
}

vec2 fbToTerrain(int x, int y) {
	float xx = (float)x * ((float)TERRAIN_WIDTH/FRAMEBUFFER_WIDTH);
	float yy = (float)y * ((float)TERRAIN_HEIGHT/FRAMEBUFFER_HEIGHT);
	return _vec2(xx, yy);
}

vec2 worldToTerrain(float x, float y) {
	float xx = (x+WORLD_RIGHT) * ((float)TERRAIN_WIDTH/WORLD_WIDTH);
	float yy = (y+WORLD_TOP) * ((float)TERRAIN_HEIGHT/WORLD_HEIGHT);
	return _vec2(xx, yy);
}
vec2 terrainToWorld(int x, int y) {
	float xx = (float)(x - TERRAIN_WIDTH/2) * ((float)WORLD_WIDTH/TERRAIN_WIDTH);
	float yy = (float)(y - TERRAIN_HEIGHT/2) * ((float)WORLD_HEIGHT/TERRAIN_HEIGHT);
	return _vec2(xx, yy);
}
vec2 worldToFb(vec2 p) {
	return _vec2((p.x+WORLD_RIGHT)*((float)FRAMEBUFFER_WIDTH/WORLD_WIDTH), (p.y+WORLD_TOP)*((float)FRAMEBUFFER_HEIGHT/WORLD_HEIGHT));
}
unsigned char getTerrainTileFromWorld(vec2 coords) {
	vec2 c = worldToTerrain(coords.x, coords.y);
	return terrain[(int)c.y*TERRAIN_WIDTH+(int)c.x];
}

void renderCircle(unsigned int* framebuffer, vec2 pos, float radius, vec3 color) {
	vec2 p = {(pos.x+WORLD_RIGHT)*((float)FRAMEBUFFER_WIDTH/WORLD_WIDTH), (pos.y+WORLD_TOP)*((float)FRAMEBUFFER_HEIGHT/WORLD_HEIGHT)};
	// printf("%f %f %f \n", p.x, p.y, (float)FRAMEBUFFER_WIDTH / 200);
	float radiusAdjusted = radius*((float)FRAMEBUFFER_HEIGHT/(float)WORLD_HEIGHT);
	for (int y = max(p.y-radiusAdjusted, 0); y < min(p.y+radiusAdjusted, FRAMEBUFFER_HEIGHT); ++y) {
		for (int x = max(p.x-radiusAdjusted, 0); x < min(p.x+radiusAdjusted, FRAMEBUFFER_WIDTH); ++x) {
			float xdiff =(float)x-p.x;
			float ydiff =(float)y-p.y;
			if(sqrt(xdiff*xdiff + ydiff*ydiff) < radiusAdjusted) {
				float c = smoothstep(radiusAdjusted, radiusAdjusted-1, len(xdiff, ydiff));
				vec3 bb = decodeIntColor(framebuffer[y * FRAMEBUFFER_WIDTH + x]);
				framebuffer[y * FRAMEBUFFER_WIDTH + x] = encodeIntColor(mix3(bb, color, c));
				//framebuffer[y * FRAMEBUFFER_WIDTH + x] = (int)(255.0*(1.0-len(xdiff, ydiff)/radiusAdjusted)) << 16;
			}
		}
	}
	//framebuffer[(int)p.y*FRAMEBUFFER_WIDTH + (int)p.x] = 0xFF0000;
	// framebuffer[1] = 0xFF0000;
}

int encodeColor(float r, float g, float b) {
	return ((int)(r*255) << 16) | ((int)(g*255) << 8) | ((int)(b*255) << 0);
}
int encodeColor(vec3 c) {
	return encodeColor(c.x, c.y, c.z);
}

void newCreatureTarget(creature* c) {
	do {
		c->target.x = c->pos.x + (randf()-0.5)*30;
		c->target.y = c->pos.y + (randf()-0.5)*30;
	} while(c->target.x<WORLD_LEFT || c->target.x>WORLD_RIGHT || c->target.y<WORLD_BOTTOM || c->target.y>WORLD_TOP);
}

void addCreature(creature* p1, creature* p2) {
	if(numCreatures < array_size(creatures)) {
		creature c = {0};
		if(p1 && p2) {
			c.pos = p1->pos;
			c.life = 100.0;
			c.horny = 0;
			c.waterAffinity = mix(p1->speed, p2->speed, randf())+(randf()-0.5)*0.05;
			c.speed = mix(p1->speed, p2->speed, randf())+(randf()-0.5)*0.05;
		} else {
			c.pos.x = randf()*WORLD_WIDTH + WORLD_LEFT;
			c.pos.y = randf()*WORLD_HEIGHT + WORLD_BOTTOM;
			c.life = 100.0;
			c.horny = 0;
			c.waterAffinity = randf();
			c.speed = randf();
		}
		c.dir = randfr(0.0, PI*2);
		creatures[numCreatures++] = c;
	}
}

void removeCreature(int i) {
	memcpy(creatures+i, creatures+i+1, sizeof(creature)*(numCreatures-i-1));
	--numCreatures;
}

void addFood() {
	if(numFoods < array_size(foods)) {
		foods[numFoods].pos.x = randf()*WORLD_WIDTH + WORLD_LEFT;
		foods[numFoods].pos.y = randf()*WORLD_HEIGHT + WORLD_BOTTOM;
		foods[numFoods].growing = 0;
		++numFoods;
	}
}

void removeFood(int i) {
// 	memcpy(foods+i, foods+i+1, sizeof(food)*(numFoods-i-1));
// 	--numFoods;
	foods[i].growing = 100;
}

#pragma pack(push, 1)
typedef struct {
	char header[2];
	uint32_t size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;

	// Windows BITMAPINFOHEADER
	uint32_t headerSize;
	int32_t bitmapWidth;
	int32_t bitmapHeight;
	uint16_t colorPlanes;
	uint16_t colorDepth;
	uint32_t compression;
	uint32_t imageSize;
	int32_t hres;
	int32_t vres;
	uint32_t paletteSize;
	uint32_t importantColors;
} bmp_header;
#pragma pack(pop)

typedef struct {
	uint32_t* data;
	bmp_header* header;
} bmp;

bmp fontBmp;

bmp loadBmp(char* filename) {
	FILE* fontFile = fopen(filename, "rb");
	fseek(fontFile, 0, SEEK_END);
	long fileSize = ftell(fontFile);
	rewind(fontFile);
	void* fontData = malloc(fileSize);
	fread(fontData, 1, fileSize, fontFile);
	fclose(fontFile);
	
	bmp_header* header = (bmp_header*)fontData;
	uint32_t* palette = (uint32_t*)((char*)fontData+14+header->headerSize);
	char* data = (char*)fontData+header->offset;
	int rowSize = ((header->colorDepth*header->bitmapWidth+31) / 32) * 4;
	
	uint32_t* image = (uint32_t*)malloc(sizeof(uint32_t)*header->bitmapWidth*header->bitmapHeight);
	//{for(int w=0; w<header.bitmapHeight}
	for(int row=0; row<header->bitmapHeight; ++row) {
		int bitIndex=0;
		for(int pixel=0; pixel<header->bitmapWidth; ++pixel) {//while((bitIndex/8) < rowSize) {
			uint32_t* chunk = (uint32_t*)((char*)fontData+header->offset+(row*rowSize)+(bitIndex/8));
			uint32_t pi = *chunk;
			if(header->colorDepth<8) {
				pi >>= (header->colorDepth-(bitIndex%8));
			}
			pi &= (/*header->colorDepth*header->colorDepth*//*ipow(header->colorDepth)*/((int64_t)1<<header->colorDepth)-1);
			if(header->colorDepth>8) {
				image[row*header->bitmapWidth+pixel] = pi;
			} else {
				image[row*header->bitmapWidth+pixel] = palette[pi];
			}
			if(pixel==120) {
				int asd = 0;
			}
			bitIndex += header->colorDepth;
		}
	}

	bmp result;
	result.data = image;
	result.header = header;
	return result;
}

bmp test4 = loadBmp("test4.bmp");
bmp test8 = loadBmp("test8.bmp");
bmp test24 = loadBmp("test24.bmp");
bmp test32 = loadBmp("test32.bmp");

//#define renderFont(fb, x, y, text, ...) {char buffer[256]; snprintf(buffer, 256, text, __VA_ARGS__); _renderFont(fb, x, y, buffer);}
void renderFont(unsigned int* framebuffer, int x, int y, char* text, ...) {
	va_list args;
	va_start(args, text);
	//printf(text, args);
	//vprintf("%i %i\n", args);
	char buffer[256];
	vsprintf(buffer, text, args);
	va_end(args);
	text = buffer;

	int scale = 2;
	int ci = 0;
	int ri = 0;
	while(*text) {
		if(*text=='\n') {
			++text;
			ci = 0;
			++ri;
			continue;
		}
		for(int fby=0; fby<16*scale; ++fby)
		for(int fbx=0; fbx<8*scale; ++fbx) {
			int ytile = (7 - *text/16)*16;
			int xtile = *text%16*8;
			int pixel = fontBmp.data[(ytile + fby/scale)*fontBmp.header->bitmapWidth + (xtile + fbx/scale)];
			if(pixel==0xFF000000) {
				framebuffer[(y+fby+(-ri*16*scale))*FRAMEBUFFER_WIDTH+(x+fbx+(ci*(8-2)*scale))] = 0xFFFFFFFF;
			}
		}
		++text;
		++ci;
	}
}

void gameStart(unsigned int* framebuffer) {
	vec2 test = convertCoordsFbToWorld(50, 50);
	printf("%f %f\n", test.x, test.y);

// 	for(int y=0; y<FRAMEBUFFER_HEIGHT; ++y)
// 	for(int x=0; x<FRAMEBUFFER_WIDTH; ++x) {
// 		float t = fbm(vec2Mul(convertCoordsFbToWorld(x, y), _vec2(0.05,0.05)));
// 		if(t>0.45) {
// 			framebuffer[y*FRAMEBUFFER_WIDTH+x] = (unsigned int)(t*255) << 8;
// 		} else {
// 			framebuffer[y*FRAMEBUFFER_WIDTH+x] = 255;
// 		}
// 	}

	for(int y=0; y<TERRAIN_HEIGHT; ++y)
	for(int x=0; x<TERRAIN_WIDTH; ++x) {
		terrain[y*TERRAIN_WIDTH+x] = fbm(vec2Mul(terrainToWorld(x, y), /*_vec2(0.05,0.05)*/_vec2(0.015,0.015))) * 255;
		// framebuffer[(y+(FRAMEBUFFER_HEIGHT-TERRAIN_HEIGHT))*FRAMEBUFFER_WIDTH+x] = terrain[y*TERRAIN_WIDTH+x] << 8;
	}

// 	for(int y=0; y<FRAMEBUFFER_HEIGHT; ++y)
// 	for(int x=0; x<FRAMEBUFFER_WIDTH; ++x) {
// 		
// 	}

	{for(int i=0; i<20; ++i) {
// 		creatures[i].pos.x = randFloat()*WORLD_WIDTH + WORLD_LEFT;
// 		creatures[i].pos.y = randFloat()*WORLD_HEIGHT + WORLD_BOTTOM;
// 		newCreatureTarget(creatures+i);
// 		creatures[i].life = 100;
// 		++numCreatures;
		addCreature(0, 0);
	}}
// 	creatures[0].pos.x =0;
//  	creatures[0].pos.y =-50;

	{for(int i=0; i<array_size(foods); ++i) {
		addFood();
	}}
	
	{for(int y=0; y<FRAMEBUFFER_HEIGHT; ++y)
	for(int x=0; x<FRAMEBUFFER_WIDTH; ++x) {
		vec2 pos = fbToTerrain(x, y);
		vec2 f = fract2(pos);
		int t00 = terrain[(int)pos.y*TERRAIN_WIDTH+(int)pos.x];
		// 		int t10 = terrain[(int)pos.y*TERRAIN_WIDTH+((int)pos.x+1)];
		// 		int t01 = terrain[((int)pos.y+1)*TERRAIN_WIDTH+(int)pos.x];
		// 		int t11= terrain[((int)pos.y+1)*TERRAIN_WIDTH+((int)pos.x+1)];
		int lerp = t00;//mix(mix(t00,t10, f.x), mix(t01,t11, f.x), f.y);
		if(lerp>110) {
			staticTerrainBackground[y*FRAMEBUFFER_WIDTH+x] = lerp << 8;
		} else {
			float t = (float)lerp/110;
			vec3 waterColor = mix3(_vec3(0.2, 0.2, 1.0), _vec3(0.2, 0.5, 0.9), t*t);
			staticTerrainBackground[y*FRAMEBUFFER_WIDTH+x] = encodeColor(waterColor);
		}
	}}

// 	FILE* fontFile = fopen("font_v2.bmp", "rb");
// 	fseek(fontFile, 0, SEEK_END);
// 	long fileSize = ftell(fontFile);
// 	rewind(fontFile);
// 	void* fontData = malloc(fileSize);
// 	fread(fontData, 1, fileSize, fontFile);
// 	fclose(fontFile);
// 
// 	bmp_header* header = (bmp_header*)fontData;
// 	fontBmpHeader = header;
// 	uint32_t* palette = (uint32_t*)((char*)fontData+14+header->headerSize);
// 	char* data = (char*)fontData+header->offset;
// 	int rowSize = ((header->colorDepth*header->bitmapWidth+31) / 32) * 4;
// 
// 	fontBmp = (uint32_t*)malloc(sizeof(uint32_t)*header->bitmapWidth*header->bitmapHeight);
// 	for(int row=0; row<header->bitmapHeight; ++row) {
// 		int bitIndex=0;
// 		for(int pixel=0; pixel<header->bitmapWidth; ++pixel) {//while((bitIndex/8) < rowSize) {
// 			uint32_t chunk = *((char*)fontData+header->offset+(row*rowSize)+(bitIndex/8));
// 			uint32_t pi = chunk >> (header->colorDepth-(bitIndex%8)) & (header->colorDepth*header->colorDepth-1);
// 			fontBmp[row*header->bitmapWidth+pixel] = palette[pi];
// 			if(pixel==120) {
// 				int asd = 0;
// 			}
// 			bitIndex += header->colorDepth;
// 		}
// 	}

	fontBmp = loadBmp("font_v2.bmp");

// 	{for(int y=0; y<header->bitmapHeight*4; ++y)
// 	for(int x=0; x<header->bitmapWidth*4; ++x) {
// 		framebuffer[y*FRAMEBUFFER_WIDTH+x] = fontBmp[(y/4)*header->bitmapWidth+(x/4)];
// 	}}
}

void gameUpdate() {
// 	creatures[0].pos.x =0;
// 	creatures[0].pos.y =-50;
// 	creatures[1].pos.x =-50;
// 	creatures[1].pos.y =0;
// 	creatures[2].pos.x =50;
// 	creatures[2].pos.y =50;
	// printf("%f", mix(10, 20, 22));

	{for(int i=0; i<numCreatures; ++i) {
		creature* c = creatures+i;
		bool foundMate = false;
		bool target = false;
		if(c->horny>0.5 && c->hunger<0.5) {
			{for(int i2=0; i2<numCreatures; ++i2) {
				creature* c2 = creatures+i2;
				if(i!=i2 && c2->horny>0.5) {
					float l = len2(vec2Sub(c->pos, c2->pos));
					if(l<1) {
						addCreature(c, c2);
						c->horny = 0;
						c2->horny= 0;
					} else
					if(l<20) {
						c->target = c2->pos;
						foundMate = true;
						target = true;
					}
				}
			}}
		}
		if(!foundMate && c->hunger > 0.1) {
			{for(int f=0; f<numFoods; ++f) {
				if(foods[f].growing<1) {
					float l = len2(vec2Sub(creatures[i].pos, foods[f].pos));
					if(l < 1) {
						removeFood(f);
						// creatures[i].life = min(creatures[i].life+10, 100);
						c->hunger -= 0.5;
					} else
						if(l < 20) {
							vec2 foodTerrain = worldToTerrain(foods[f].pos.x, foods[f].pos.y);
							unsigned char tile = terrain[(int)foodTerrain.y*TERRAIN_WIDTH+(int)foodTerrain.x];
							if(c->waterAffinity>0.5 ? tile<=110 : tile>110) {
								creatures[i].target = foods[f].pos;
								target = true;
								break;
							}
						}
				}
			}}
		}

		float len = len2(vec2Sub(creatures[i].target, creatures[i].pos));
		if(len < 1.0) {
			unsigned char tile;
			for(int ti=0; ti<10; ++ti) {
				newCreatureTarget(c);
				vec2 terrainPos = worldToTerrain(c->target.x, c->target.y);
				tile = terrain[(int)terrainPos.y*TERRAIN_WIDTH+(int)terrainPos.x];
				if(c->waterAffinity>0.5 ? tile<=110 : tile>110) {
					break;
				}
			};
		} else {
			vec2 n = normalize2(vec2Sub(creatures[i].target, creatures[i].pos));

			vec2 terrainPos = worldToTerrain(c->pos.x, c->pos.y);
			float tile = terrain[(int)terrainPos.y*TERRAIN_WIDTH+(int)terrainPos.x];
			float terrainModifier = tile>110 ? 1.0-c->waterAffinity+0.1 : c->waterAffinity+0.1;
			float speed = 0.05+c->speed*0.25 * terrainModifier;
			creatures[i].pos = vec2Add(creatures[i].pos, vec2Mul(n, _vec2(speed, speed)));
		}

#if 0
		if(target) {
			vec2 targetDir = normalize2(vec2Sub(creatures[i].target, creatures[i].pos));
			float rad = atan2f(targetDir.x, targetDir.y);
			if(rad<c->dir && rad>c->dir-PI || rad>c->dir && rad>c->dir+PI) {
				c->dir -= 0.1;
			} else {
				c->dir += 0.1;
			}
			if(c->dir<-PI) c->dir+=(PI*2);
			if(c->dir>PI) c->dir-=(PI*2);

			//vec2 n = normalize2(vec2Sub(creatures[i].target, creatures[i].pos));
		} else {
			vec2 dir = normalize2(vec2Sub(c->target, c->pos));
			vec2 point = (vec2Add(c->pos, vec2Mul(dir, _vec2(2, 2))));
			float rad = atan2f(dir.x, dir.y);
			float righta = rad+(PI/4);
			vec2 right = (vec2Add(c->pos, vec2Mul(_vec2(sin(righta), cos(righta)), _vec2(2, 2))));
			float lefta = rad-(PI/4);
			vec2 left = (vec2Add(c->pos, vec2Mul(_vec2(sin(lefta), cos(lefta)), _vec2(2, 2))));

			unsigned char aheadTile = getTerrainTileFromWorld(point);
			unsigned char leftTile = getTerrainTileFromWorld(left);
			unsigned char rightTile = getTerrainTileFromWorld(right);
			float aheadWeight = aheadTile<110 ? c->waterAffinity : (1.0-c->waterAffinity) * (1-((float)aheadTile-110)/(255.0-110.0));
			float leftWeight = leftTile<110 ? c->waterAffinity : (1.0-c->waterAffinity) * (1-((float)leftTile-110)/(255.0-110.0));
			float rightWeight = rightTile<110 ? c->waterAffinity : (1.0-c->waterAffinity) * (1-((float)rightTile-110)/(255.0-110.0));
			aheadWeight *= aheadWeight;
			leftWeight *= leftWeight;
			rightWeight *= rightWeight;
			float selection = randfr(0.0, aheadWeight+leftWeight+rightWeight);
			if(selection<aheadWeight) {

			} else if(selection<aheadWeight+leftWeight) {
				c->dir -= 0.1;
			} else {
				c->dir += 0.2;
			}
		}

		vec2 n = _vec2(sin(c->dir), cos(c->dir));
		vec2 terrainPos = worldToTerrain(c->pos.x, c->pos.y);
		float tile = terrain[(int)terrainPos.y*TERRAIN_WIDTH+(int)terrainPos.x];
		float terrainModifier = tile>110 ? 1.0-c->waterAffinity+0.1 : c->waterAffinity+0.1;
		float speed = 0.05+c->speed*0.25 * terrainModifier;
		creatures[i].pos = vec2Add(creatures[i].pos, vec2Mul(n, _vec2(speed, speed)));
#endif

		if(c->hunger>0.8) {
			creatures[i].life -= 0.1;
		} else {
			c->life = min(c->life+0.02, 100.0);
		}
		if(creatures[i].life <= 0) {
			removeCreature(i);
		}

		c->hunger += 0.001;
		c->horny += 0.001;
	}}

// 	if(getTime()-foodSpawnTimer > 500) {
// 		if(numFoods < array_size(foods)) {
// 			addFood();
// 		}
// 		foodSpawnTimer = getTime();
// 	}
	{for(int i=0; i<numFoods; ++i) {
		if(foods[i].growing>0) {
			foods[i].growing -= 0.1;
		}
	}}
}

void gameRender(unsigned int* framebuffer) {
	//printf("test\n");
	// framebuffer[0] = 0x0000FF;

	//memset(framebuffer, 0, sizeof(unsigned int)*FRAMEBUFFER_WIDTH*FRAMEBUFFER_HEIGHT);

	memcpy(framebuffer, staticTerrainBackground, sizeof(staticTerrainBackground));

#if 0
	for(int y=0; y<FRAMEBUFFER_HEIGHT; ++y)
	for(int x=0; x<FRAMEBUFFER_WIDTH; ++x) {
		vec2 pos = fbToTerrain(x, y);
		vec2 f = fract2(pos);
 		int t00 = terrain[(int)pos.y*TERRAIN_WIDTH+(int)pos.x];
// 		int t10 = terrain[(int)pos.y*TERRAIN_WIDTH+((int)pos.x+1)];
// 		int t01 = terrain[((int)pos.y+1)*TERRAIN_WIDTH+(int)pos.x];
// 		int t11= terrain[((int)pos.y+1)*TERRAIN_WIDTH+((int)pos.x+1)];
		int lerp = t00;//mix(mix(t00,t10, f.x), mix(t01,t11, f.x), f.y);
		if(lerp>110) {
			framebuffer[y*FRAMEBUFFER_WIDTH+x] = lerp << 8;
		} else {
			float t = (float)lerp/110;
			vec3 waterColor = mix3(_vec3(0.2, 0.2, 1.0), _vec3(0.2, 0.5, 0.9), t*t);
			framebuffer[y*FRAMEBUFFER_WIDTH+x] = encodeColor(waterColor);
		}
	}
#endif

	{for(int i=0; i<numCreatures; ++i) {
		creature* c = creatures+i;
		vec3 color;
		if(c->waterAffinity>0.5) {
			color = _vec3(1, 1-clamp(c->waterAffinity*2-1, 0, 1), 1);
		} else {
			color = _vec3(1, 1, 1-(1.0-clamp(c->waterAffinity*2, 0, 1)));
		}
		
		renderCircle(framebuffer, creatures[i].pos, 1, i>0 ? color : _vec3(1, 0, 0));
		if(!i) {
// 			vec2 dir = _vec2(sin(c->dir), cos(c->dir));
// 			vec2 point = worldToFb(vec2Add(c->pos, vec2Mul(dir, _vec2(5, 5))));
// 			framebuffer[(int)point.y*FRAMEBUFFER_WIDTH+(int)point.x] = 0xFF0000;
// 			float rad = atan2f(dir.x, dir.y);
// 
// 			float righta = rad+(PI/4);
// 			vec2 right = worldToFb(vec2Add(c->pos, vec2Mul(_vec2(sin(righta), cos(righta)), _vec2(5, 5))));
// 			framebuffer[(int)right.y*FRAMEBUFFER_WIDTH+(int)right.x] = 0xFF0000;
// 
// 			float lefta = rad-(PI/4);
// 			vec2 left = worldToFb(vec2Add(c->pos, vec2Mul(_vec2(sin(lefta), cos(lefta)), _vec2(5, 5))));
// 			framebuffer[(int)left.y*FRAMEBUFFER_WIDTH+(int)left.x] = 0xFF0000;
		}
		vec2 fbp = worldToFb(creatures[i].pos);
		fbp.y += 10;
		if(fbp.y<FRAMEBUFFER_HEIGHT && fbp.y>0) {
			{for(int l=-8; l<12; ++l) {
				int index = (int)fbp.y*FRAMEBUFFER_WIDTH+clampi((int)fbp.x+l, 0, FRAMEBUFFER_WIDTH-1);
				if(index>0 && index<FRAMEBUFFER_WIDTH*FRAMEBUFFER_HEIGHT) {
					framebuffer[index] = 255 << ((float)(l+8)/20.0>creatures[i].life/100.0 ? 16 : 8);
				}
			}}
			fbp.y -= 2;
			{for(int l=-8; l<12; ++l) {
				int index = (int)fbp.y*FRAMEBUFFER_WIDTH+clampi((int)fbp.x+l, 0, FRAMEBUFFER_WIDTH-1);
				if(index>0 && index<FRAMEBUFFER_WIDTH*FRAMEBUFFER_HEIGHT) {
					framebuffer[index] = ((float)(l+8)/20.0>creatures[i].hunger/1.0 ? 255<<16 : 255<<16|255<<8);
				}
			}}
			fbp.y -= 2;
			{for(int l=-8; l<12; ++l) {
				int index = (int)fbp.y*FRAMEBUFFER_WIDTH+clampi((int)fbp.x+l, 0, FRAMEBUFFER_WIDTH-1);
				if(index>0 && index<FRAMEBUFFER_WIDTH*FRAMEBUFFER_HEIGHT) {
					framebuffer[index] = ((float)(l+8)/20.0>creatures[i].horny/1.0 ? 255<<16 : 255<<0);
				}
			}}
		}
	}}

	{for(int i=0; i<numFoods; ++i) {
		if(foods[i].growing<1) {
			renderCircle(framebuffer, foods[i].pos, 0.7, _vec3(0, 1, 0));
		}
	}}

// 	renderFont(framebuffer, 20, 720-32-20, "The quick brown fox jumps over the lazy dog!\n"
// 											"Almost before we knew it, we had left the ground.\n"
// 											"The spectacle before us was indeed sublime.\n"
// 											"%i %i", numCreatures, numFoods);

	float avgSpeed = 0;
	float avgWater = 0;
	for(int c=0; c<numCreatures; ++c) {
		avgSpeed += creatures[c].speed;
		avgWater += creatures[c].waterAffinity;
	}
	avgSpeed /= numCreatures;
	avgWater /= numCreatures;
	
	renderFont(framebuffer, 20, 720-32-20, "creatures  %i\n"
										   "food       %i\n"
										   "c1 speed   %f\n"
										   "c1 water   %f\n"
										   "avg speed  %f\n"
										   "avg water  %f\n",
										   numCreatures,
										   numFoods,
										   creatures[0].speed,
										   creatures[0].waterAffinity,
										   avgSpeed,
										   avgWater);

// 	{for(int y=0; y<test4.header->bitmapHeight*4; ++y)
// 	for(int x=0; x<test4.header->bitmapWidth*4; ++x) {
// 		framebuffer[(y+50)*FRAMEBUFFER_WIDTH+x+50] = test4.data[(y/4)*test4.header->bitmapWidth+(x/4)];
// 	}}
// 	{for(int y=0; y<test8.header->bitmapHeight*4; ++y)
// 		for(int x=0; x<test8.header->bitmapWidth*4; ++x) {
// 			framebuffer[(y+50)*FRAMEBUFFER_WIDTH+x+150] = test8.data[(y/4)*test8.header->bitmapWidth+(x/4)];
// 	}}
// 	{for(int y=0; y<test24.header->bitmapHeight*4; ++y)
// 		for(int x=0; x<test24.header->bitmapWidth*4; ++x) {
// 			framebuffer[(y+50)*FRAMEBUFFER_WIDTH+x+250] = test24.data[(y/4)*test24.header->bitmapWidth+(x/4)];
// 	}}
// 	{for(int y=0; y<test32.header->bitmapHeight*4; ++y)
// 		for(int x=0; x<test32.header->bitmapWidth*4; ++x) {
// 			framebuffer[(y+50)*FRAMEBUFFER_WIDTH+x+350] = test32.data[(y/4)*test32.header->bitmapWidth+(x/4)];
// 	}}
}
