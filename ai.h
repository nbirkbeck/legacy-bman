#ifndef AI_H
#define AI_H

#include "level.h"
#include "globals.h"
#include "player.h"
#include "queue.h"

#define MOVE    1
#define USE_DROP_BOMB 2
#define USE_ALT_KEY 4

#define GOAL_BOMB 1
#define GOAL_HIDE 2
#define GOAL_FIGHT 3

typedef struct GPOINT_TYPE
{
	int i,j;
}GPoint;

typedef struct BRAIN_TYPE
{
	int hasGoal;
	int goal;
	Queue * path;
	int direction;
	int actionToTake;
	int lastSearched;
	int (* think)(BRAIN_TYPE *,Player *); 
}Brain;

int canHide(int i,int j, int prefferedDir);
int calculateUtility();
void initBrain(Brain * brain);
int brainThink1(Brain * brain,Player * player);
int movePlayerAI(Player * player);
int  releaseBrain(Brain * brain);

#endif
