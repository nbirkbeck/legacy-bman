#include "globals.h"
#include "bomb.h"
#include "level.h"
#include "ai.h"

#ifndef PLAYER_H
#define PLAYER_H


#define PLAYER_NORMAL 1
#define PLAYER_DYEING 2

#define PLAYER0_X XOFFSET
#define PLAYER0_Y YOFFSET
#define PLAYER1_X (XOFFSET+(16<<5))
#define PLAYER1_Y (YOFFSET+(12<<5))
#define PLAYER2_X (XOFFSET+(16<<5))
#define PLAYER2_Y (YOFFSET)
#define PLAYER3_X (XOFFSET)
#define PLAYER3_Y (YOFFSET+(12<<5))


#define NUM_PLAYER_FRAMES 14

#define PLAYER_NET_SIZE 48

typedef struct PLAYER_TYPE
{
	int x,y;
	char dir;
	unsigned char clientNum;
	int powerups;//  use a mask for these 
	char power;   //  strength for the bombs
	char nBombs;
	int frame;
	char speed;
	int state;// Normal or Dying
	/*
		Need the player textures here as well
	*/
	unsigned char nkills;
	unsigned char ndeaths;
	unsigned char nsuicides;
	char name[24];
	LPDIRECTDRAWSURFACE7 textures[21];
	char nBombsInUse;
	List bombsInUse;
	void * brain;
	int (* movePlayerFunc)(PLAYER_TYPE *);
}Player;

int preMovePlayer(Player * player);
int movePlayerUp(Player * player);
int movePlayerDown(Player * player);
int movePlayerRight(Player * player);
int movePlayerLeft(Player * player);

int killPlayer(Player * player,Bomb *);
int initPlayer(Player * player);
Bomb * playerPlaceBomb(Player * player);
int playerAltKey(Player * player);
int loadPlayerTextures(Player * player, char * filename);
int releasePlayerTextures(Player * player);
int drawPlayer(Player * player);
int drawScoreBoardPlayer(Player * player,int x, int y);
#endif
