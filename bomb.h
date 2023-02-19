#include "dxgl.h"
#include "t3dlib3.h"
#include "globals.h"

#ifndef BOMB_H
#define BOMB_H


#define LEFT_EXP 0
#define HOR_EXP  1
#define MID_EXP  2
#define RIGHT_EXP 3
#define BOTTOM_EXP 4
#define TOP_EXP 5
#define VER_EXP 6

#define NUM_DEGREES 4
#define NUM_BOMBTEX 4

#define BOMB_NORMAL 1
#define BOMB_DETONATOR 2
#define BOMB_EXPLODING 3
#define BOMB_UNUSED 0


#define BOMB_SPEED 4

typedef struct  BOMB_TYPE
{
	int x,y;
	unsigned int  frame;
	int  timer;
	unsigned char state;
	unsigned char power;
	unsigned char moving;
	unsigned char dir;
	char depths[4];
	void * owner;

}Bomb;

int initBomb(Bomb * bomb,int x, int y,int state,int power, void * owner);

int releaseBombTextures();
int loadBombTextures();
int loadExplosionTextures();

int drawExplosionTile(int type,int frame,int x, int y);
int moveBomb(Bomb * bomb);
int drawBomb(Bomb * bomb);
int explodeBomb(Bomb * bomb);
int midExplodeBomb(Bomb * bomb);
int postExplodeBomb(Bomb * bomb);
#endif