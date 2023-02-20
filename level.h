#include <SDL2/SDL_surface.h>
#include "dxgl.h"
#include "t3dlib3.h"
#include "globals.h"
#include "bomb.h"
#include "list.h"

#ifndef LEVEL_H
#define LEVEL_H

#define GRID_HEIGHT 13
#define GRID_WIDTH  17

#define BOMB_MASK 0xf8000000
#define BOMB_SHAMT 27

#define FLOOR 0x1
#define ALT_FLOOR 0x2
#define BRICK 0x4
#define INDY_BRICK 0x8

#define BOMB_TEX 2
#define FLAME_TEX 0
#define SPEED_TEX 6
#define KICK_TEX 4
#define PUNCH_TEX 8
#define DEATH_TEX 10
#define DETONATOR_TEX 12

#define NUM_POWERUP_TEXTURES 14


#define BOMB 0x10
#define FLAME 0x20
#define SPEED 0x40
#define KICK 0x80
#define DETONATOR 0x100
#define DEATH 0x200
#define PUNCH 0x400
#define PUP8 0x800

#define NUM_LEVEL_TEXTURES 11

#define FLOOR_TEX 0
#define ALT_FLOOR_TEX 1
#define INDY_TEX 2
#define BRICK_TEX 3

#define HAS_BOMB_MASK 0x04000000
#define BRICK_BREAKING_MASK 0x02000000

Bomb * placeBomb(int,int i,int j);
int pickupPowerUp(int i, int j);
int unmaskGridSquare(int i, int j);
int maskGridSquare(int i, int j,int bombIndex);
int loadPowerUpTextures(const char * filename);
int createLevelTextures();
int createBasicLevel(int nbombs,int nflames, int nkick, int ndet, int nspeed);
int loadLevelTextures(const char * filename);
int loadLevel(const char * filename);
int drawLevelTexture(int tNum,int x,int y);
int drawLevelSmall(SDL_Surface*,int x,int y);
int drawLevel();
int levelKill(Bomb *,int,int);
int levelKillPlayer(Bomb *,int,int);
int levelKick(int i, int j, int dir);
int releaseLevelTextures();
int releasePowerUpTextures();
int checkGrid(int x, int y);int checkForPlayer(int x, int y);

#endif
