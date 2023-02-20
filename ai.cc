#include "ai.h"
#include "BFSearch.h"
#include "font.h"

//#define DRAW_UTILITY
//#define DRAW_CANHIDE
//#define DRAW_NEXT
extern unsigned int grid[GRID_HEIGHT][GRID_WIDTH];
extern List playerList;
extern Bomb bombs[TOTAL_BOMBS];

extern SDL_Surface* surface;
extern Font font;

extern FILE * debug;

int searchDepth=0;
int g_goalUtility=1;
int utilities[GRID_HEIGHT][GRID_WIDTH];



/************************************************
*
*************************************************/
void initBrain(Brain * brain)
{
	brain->hasGoal=0;  //May not need
	brain->direction=0;
	brain->actionToTake=0;
	brain->think = brainThink1;
	brain->path=NULL;
	brain->lastSearched=GetTickCount();
}

int checkUtility(int i,int j)
{
	if(i<0||j<0||i>=GRID_HEIGHT||j>=GRID_WIDTH)
		return 0;
	else return utilities[i][j];
}

int canHide(int i,int j, int prefferedDir)
{
	int orders[4];
	int nsteps[4];//The number of steps in each direction to safety
	int c=0;
	int dir=0;
	
	orders[c++]=prefferedDir;
	
	while(dir<4)
	{
		if(dir!=prefferedDir)
		{
			orders[c++]=dir;
		}
		dir++;
	}
	c=0;
	if(checkUtility(i,j)==0 ||grid[i][j]&HAS_BOMB_MASK ||grid[i][j]&0xc)
		return -1;
	while(c<4)
	{
		int steps=0;
		int ti=i;
		int tj=j;
		switch(orders[c])
		{
		case UP:
			while(1)
			{
				ti--;
				steps++;
				/*
					Either on a brick, or off the course
					so we are done
				*/
				if(checkUtility(ti,j)==0 || grid[ti][j]&HAS_BOMB_MASK)
				{
					steps=-1;
					break;
				}
				if(checkUtility(ti,j+1)>0)
					break;
				if(checkUtility(ti,j-1)>0)
					break;
				
			}
			nsteps[UP]=steps;
			break;
		case DOWN:
			while(1)
			{
				ti++;
				steps++;
				/*
					Either on a brick, or off the course
					so we are done
				*/
				if(checkUtility(ti,j)==0 ||grid[ti][j]&HAS_BOMB_MASK)
				{
					steps=-1;
					break;
				}
				if(checkUtility(ti,j+1)>0)
					break;
				if(checkUtility(ti,j-1)>0)
					break;
				
			}
			nsteps[DOWN]=steps;
			break;
		case LEFT:
			while(1)
			{
				tj--;
				steps++;
				/*
					Either on a brick, or off the course
					so we are done
				*/
				if(checkUtility(i,tj)==0||grid[i][tj]&HAS_BOMB_MASK)
				{
					steps=-1;
					break;
				}
				if(checkUtility(i+1,tj)>0)
					break;
				if(checkUtility(i-1,tj)>0)
					break;
				
			}
			nsteps[LEFT]=steps;
			break;
		case RIGHT:
			while(1)
			{
				tj++;
				steps++;
				/*
					Either on a brick, or off the course
					so we are done
				*/
				if(checkUtility(i,tj)==0||grid[i][tj]&HAS_BOMB_MASK)
				{
					steps=-1;
					break;
				}
				if(checkUtility(i+1,tj)>0)
					break;
				if(checkUtility(i-1,tj)>0)
					break;
				
			}
			nsteps[RIGHT]=steps;
			break;
		}
		c++;
	}
	c=0;
	int leasti=-1;
	while(c<4)
	{
		if(nsteps[c]>0)
		{
			if(leasti==-1)
			{
				leasti=c;
			}
			else if(nsteps[leasti]> nsteps[c])
			{
				leasti=c;
			}
		}
		c++;
	}
	if(leasti==-1)
		return -1;
	return nsteps[leasti];
}

int AIPlayerTalk(Player * player, char * message)
{
	char * str;
	str = (char *)malloc(strlen(message)+1);
	strcpy(str,message);
	addMessage(str,&font,player->x,player->y,250);
	return 1;
}

inline int canGoSingle(int i, int j,int curUtility)
{
	if(i<0||j<0||i>=GRID_HEIGHT||j>=GRID_WIDTH)
	{
		return 0;
	}
	if(grid[i][j]&HAS_BOMB_MASK)
		return 0;
	return (!(grid[i][j]&0xc))&&(utilities[i][j]>-1*(BOMB_DEFAULT_TIMER-530) || utilities[i][j]>=curUtility);
}

inline int canGo(int i, int j,int depth)
{
	if(i<0||j<0||i>=GRID_HEIGHT||j>=GRID_WIDTH)
	{
		return 0;
	}
	if(grid[i][j]&HAS_BOMB_MASK)
	{
		return 0;
	}
	return (!(grid[i][j]&0xc))&&(utilities[i][j]>=-1*((BOMB_DEFAULT_TIMER-530-(1+searchDepth-depth)*530)));
}

Queue * search(int previous,int distance,int i,int j);
Queue * searchSafe(int previous,int distance,int i,int j);
Queue * searchWrapper(int distance,int i,int j, int goalUtility);
Queue * searchSafeWrapper(int distance,int i,int j, int goalUtility);
Queue * IDFSearch(int maxDistance,int i, int j, int goalUtility)
{
	int c=1;
	Queue * q=NULL;
	while(c<maxDistance && q==NULL)
	{
		q=searchWrapper(c,i,j,goalUtility);
		c++;
	}
	return q;
}

Queue * IDFSearchSafe(int maxDistance,int i, int j, int goalUtility)
{
	int c=1;
	Queue * q=NULL;
	while(c<maxDistance && q==NULL)
	{
		q=searchSafeWrapper(c,i,j,goalUtility);
		c++;
	}
	return q;
}

Queue * searchWrapper(int distance,int i,int j, int goalUtility)
{
	searchDepth=distance;
	g_goalUtility=goalUtility;
	return search(-1,distance,i,j);
}

Queue * searchSafeWrapper(int distance,int i,int j, int goalUtility)
{
	searchDepth=distance;
	g_goalUtility=goalUtility;
	return searchSafe(-1,distance,i,j);

}
Queue * search(int previous,int distance,int i,int j)
{
	Queue * path;
	int depth = distance;
#ifdef DEBUG_SEARCH
	fprintf(debug,"search[%d,%d], distance=%d :\n",i,j,distance);
#endif
	if(distance<=0)
	{
		if(i<0||i>=GRID_HEIGHT||j<0||j>=GRID_WIDTH)
			return 0;
#ifdef DEBUG_SEARCH
			fprintf(debug,"utilities[i][j]=%d canHide=%d\n",utilities[i][j],canHide(i,j,0));
#endif

		if(utilities[i][j]>= g_goalUtility && canHide(i,j,0)!=-1)
		{

			Queue * q =(Queue *) malloc(sizeof(Queue));
			GPoint * p = (GPoint *)malloc(sizeof(GPoint));
			initQueue(q);
			p->i=i;
			p->j=j;
			insert(q,p);
			return q;
		}
		else return 0;
	}
#ifdef DEBUG_SEARCH
	fprintf(debug,"\n");
#endif
	distance--;
	if(previous!=UP)
	{
		if(canGo(i+1,j,depth))
		{
			path=search(DOWN,distance,i+1,j);
			if(path!=NULL)
			{
				GPoint * p = (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	if(previous!=DOWN)
	{
		if(canGo(i-1,j,depth))
		{
			path=search(UP,distance,i-1,j);
			if(path!=NULL)
			{
				GPoint *p = (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	if(previous!=LEFT)
	{
		if(canGo(i,j+1,depth))
		{
			path=search(RIGHT,distance,i,j+1);
			if(path!=NULL)
			{
				GPoint *p = (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	if(previous!=RIGHT)
	{
		if(canGo(i,j-1,depth))
		{
			path=search(LEFT,distance,i,j-1);
			if(path!=NULL)
			{
				GPoint * p= (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	return 0;
}

Queue * searchSafe(int previous,int distance,int i,int j)
{
	Queue * path;
	int depth = distance;

	if(distance<=0)
	{
		if(i<0||i>=GRID_HEIGHT||j<0||j>=GRID_WIDTH)
			return 0;

		if(utilities[i][j]>= g_goalUtility)
		{

			Queue * q =(Queue *) malloc(sizeof(Queue));
			GPoint * p = (GPoint *)malloc(sizeof(GPoint));
			initQueue(q);
			p->i=i;
			p->j=j;
			insert(q,p);
			return q;
		}
		else return 0;
	}

	distance--;
	if(previous!=UP)
	{
		if(canGo(i+1,j,depth))
		{
			path=searchSafe(DOWN,distance,i+1,j);
			if(path!=NULL)
			{
				GPoint * p = (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	if(previous!=DOWN)
	{
		if(canGo(i-1,j,depth))
		{
			path=searchSafe(UP,distance,i-1,j);
			if(path!=NULL)
			{
				GPoint *p = (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	if(previous!=LEFT)
	{
		if(canGo(i,j+1,depth))
		{
			path=searchSafe(RIGHT,distance,i,j+1);
			if(path!=NULL)
			{
				GPoint *p = (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	if(previous!=RIGHT)
	{
		if(canGo(i,j-1,depth))
		{
			path=searchSafe(LEFT,distance,i,j-1);
			if(path!=NULL)
			{
				GPoint * p= (GPoint *) malloc(sizeof(GPoint));
				p->i=i;
				p->j=j;
				insert(path,p);
				return path;
			}
		}
	}
	return 0;
}

/*********************************************************
*
*
*********************************************************/
int calculateUtility()
{
	int i,j;
	memset(utilities,0,sizeof(int)*GRID_HEIGHT*GRID_WIDTH);

		i=0;
		j=0;
		while(i<GRID_HEIGHT)
		{
			j=0;
			while(j<GRID_WIDTH)
			{
				
				if((grid[i][j]&0xc)==0)
				{
					int nbricks=1;
					if(utilities[i][j]>=0)
					{
						if(i-1>=0)
						{
							if(grid[i-1][j]&BRICK)
								nbricks++;
						}
						if(j-1>=0)
						{
							if(grid[i][j-1]&BRICK)
								nbricks++;
						}
						if(i+1<GRID_HEIGHT)
							if(grid[i+1][j]&BRICK)
								nbricks++;
						if(j+1<GRID_WIDTH)
							if(grid[i][j+1]&BRICK)
								nbricks++;
						if(grid[i][j]&0xff0)
							nbricks+=100;
						utilities[i][j]=nbricks;
					}	
				}	
				j++;
			}
			i++;
		}

	return 1;
}


int calculateBombUtility(Player * player)
{
	int i=0;
	while(i<TOTAL_BOMBS)
	{
		Bomb * bomb = &bombs[i];
		if(bomb->state!=BOMB_UNUSED)
		{
			int ut;
			if(bomb->state==BOMB_NORMAL)
			{
				ut=-1*(BOMB_DEFAULT_TIMER-bomb->timer);
			}
			else if(bomb->state==BOMB_DETONATOR)
			{
				if((Player *)bomb->owner==player)
				{
					ut=-1;
				}
				else ut = -(BOMB_DEFAULT_TIMER-1);
			}
			else if(bomb->state==BOMB_EXPLODING)
			{
				ut=-(BOMB_DEFAULT_TIMER-1);
			}
			
					
			int si,sj;
			int bi, bj;
			int c;

			bi=(bomb->y-YOFFSET)>>5;
			bj=(bomb->x-XOFFSET)>>5;

			if(ut<utilities[bi][bj])
				utilities[bi][bj]=ut;

			c=1;
			si=bi-1;

			//Go up with the bomb utility
			while(c<=bomb->power)
			{
				if(si<0)
					break;
				if(grid[si][bj]&0x8)
					break;
				if(utilities[si][bj]>ut)
					utilities[si][bj]=ut;
				si--;
				c++;
			}

			c=1;
			si=bi+1;
			//Go down with it
			while(c<=bomb->power)
			{
				if(si>=GRID_HEIGHT)
					break;
				if(grid[si][bj]&0x8)
					break;
				if(utilities[si][bj]>ut)
					utilities[si][bj]=ut;
				si++;
				c++;
			}

			sj=bj-1;
			c=1;
			//Go left with it
			while(c<=bomb->power)
			{
				if(sj<0)
					break;
				if(grid[bi][sj]&0x8)
					break;
				if(utilities[bi][sj]>ut)
					utilities[bi][sj]=ut;
				sj--;
				c++;
			}
	
			sj=bj+1;
			c=1;
			//Go right with it
			while(c<=bomb->power)
			{
				if(sj>=GRID_WIDTH)
					break;
				if(grid[bi][sj]&0x8)
					break;
				if(utilities[bi][sj]>ut)
					utilities[bi][sj]=ut;
				sj++;
				c++;
			}
			
		}
		i++;
	}
	return 1;
}

/************************************************
*
*
*************************************************/
int brainThink1(Brain * brain,Player * player)
{
	static int t=0;
	static int times=0;
	
	int pi=(player->y - YOFFSET + 16)>>5;
	int pj=(player->x - XOFFSET + 16)>>5;
	int gx,gy;
	brain->actionToTake=0;
	if(brain->hasGoal)
	{
		if(brain->goal==GOAL_FIGHT)
		{
		
			Node * finger = playerList.head;
			Player * opp;
			while(finger!=NULL)
			{
				opp = (Player *) finger->value;
				if(opp!=player)
					break;
				finger = finger->next;	
			}
			brain->actionToTake=MOVE;
			if(opp!=NULL && opp!=player)
			{
				
				if(opp->x<player->x)
				{
					brain->direction=LEFT;
				}
				else if(opp->x>player->x)
				{
					brain->direction = RIGHT;
				}
				else if(opp->y<player->y)
				{
					brain->direction=UP;
				}
				else if(opp->y>player->y)
				{
					brain->direction = DOWN;
				}
				if(rand()%10)
					brain->hasGoal=0;
				return 1;
			}
		}
		if(brain->path && brain->path->head)
		{
			Node * finger = brain->path->head;
			GPoint * next = NULL;
			
				next = (GPoint *)finger->value;
				gx=((next->j<<5)+XOFFSET);
				gy=((next->i<<5)+YOFFSET);

				if(abs(player->x -gx)<=4 && abs(player->y -gy)<=4)
				{
					finger = finger->next;

					rem(brain->path,next);
					free(next);
					next = NULL;

					
					if(finger!=NULL)
					{
						next = (GPoint *)finger->value;
					}
				}
				if(next!=NULL)
				{
					gx=((next->j<<5)+XOFFSET);
					gy=((next->i<<5)+YOFFSET);
		
					
					if(!canGoSingle(next->i,next->j,utilities[pi][pj]))
					//if(!validatePath(brain->path))//Go(next->i,next->j,10))
					{
						char * str = (char *) malloc(20);
						
						brain->hasGoal=0;
						remAll(brain->path);
						free(brain->path);
						brain->path=NULL;

						
						drawFont(surface,&font,"Path is Invalid",player->x,player->y);
							
						brain->actionToTake=0;
						if(player->powerups&KICK)
						{
							brain->actionToTake|=USE_ALT_KEY;
						}
						return 1;
					}
					
#ifdef DRAW_NEXT
					drawFont(surface, &font,"next",gx,gy);
#endif
					brain->actionToTake=MOVE;
					if(pi>next->i)
					{
						brain->direction=UP;
					}
					else if(pi<next->i)
					{
						brain->direction=DOWN;
					}
					else if(pj<next->j)
					{
						brain->direction=RIGHT;
					}
					else if(pj>next->j)
					{
						brain->direction=LEFT;
					}
			}
			if(utilities[pi][pj]>0)
			{
				if(player->powerups&DETONATOR)
				{
					brain->actionToTake|=USE_ALT_KEY;
				}
			}
		}
		else
		{
			if(brain->path && !brain->path->head)
			{
				free(brain->path);
				brain->path=NULL;
			}
			if(/*utilities[pi][pj]>1&&*/ brain->goal==GOAL_BOMB)
			{
				brain->actionToTake=0;
				
				if(player->nBombsInUse!=player->nBombs && canHide(pi,pj,0)!=-1)
				{
					brain->actionToTake|=USE_DROP_BOMB;
					brain->hasGoal=0;
				}
				
				else
				{
					brain->path=doBFS((player->y-YOFFSET+16)>>5,(player->x-XOFFSET+16)>>5,player->clientNum+200,6);
					//brain->hasGoal=1;
					//brain->actionToTake=0;
				}
			}
			else
			{
				brain->hasGoal=0;
				brain->actionToTake=0;
			}
		}

	}
	else
	{
		Queue * q=NULL;
		
		q=doBFS((player->y-YOFFSET+16)>>5,(player->x-XOFFSET+16)>>5,player->clientNum+200,6);
		
			if(q!=NULL)
			{
				brain->path=q;
				brain->goal=GOAL_BOMB;
				brain->hasGoal=1;
				brain->actionToTake=0;
				AIPlayerTalk(player,"Can get him");
				return 1;
			}

		if(abs(GetTickCount()-brain->lastSearched)>5)
		{
			q=doBFS((player->y-YOFFSET+16)>>5,(player->x-XOFFSET+16)>>5,2,8);
			
			
			brain->lastSearched=GetTickCount();
		}
		if(q!=NULL)
			brain->goal=GOAL_BOMB;
		else if(q==NULL/* && utilities[pi][pj]<1*/)
		{
			if(utilities[pi][pj]>0)
			{
				q=NULL;
				brain->hasGoal=0;
				brain->actionToTake=0;
				return 1;
			}
			drawFont(surface, &font,"Search safe",player->x,player->y);
			q=doBFS((player->y-YOFFSET+16)>>5,(player->x-XOFFSET+16)>>5,1,12);
			brain->goal=GOAL_HIDE;
		}
		else
		{
			AIPlayerTalk(player,"Must find you!");
			brain->goal=GOAL_FIGHT;
			q=NULL;
			brain->hasGoal=1;
			return 1;
		}
		
		brain->actionToTake=0;
		
		int step=0;
		Node * finger;
		if(q!=NULL)
			finger =q->head;
		else finger=NULL;

		brain->path=q;
		if(q)
		brain->hasGoal=1;
	}
#ifdef DRAW_UTILITY
		int i=0;
		int j=0;
		while(i<GRID_HEIGHT)
		{
			j=0;
			while(j<GRID_WIDTH)
			{
				char str[5];
				sprintf(str,"%d",utilities[i][j]);
				drawFont(surface, &font,str,(j<<5)+XOFFSET,(i<<5)+YOFFSET);
			
				j++;
			}
			i++;
		}
#endif
#ifdef DRAW_CANHIDE
		i=j=0;
		while(i<GRID_HEIGHT)
		{
			j=0;
			while(j<GRID_WIDTH)
			{
				char str[35];
				sprintf(str,"%d",canHide(i,j,0));
				drawFont(surface, &font,str,(j<<5)+XOFFSET+16,(i<<5)+YOFFSET+16);
				
				j++;
			}
			i++;
		}
#endif
#ifdef DEBUG
	if(brain->path)
	{
	Node * finger =  brain->path->head;
	char note[1024];
	note[0]='\0';
	while(finger!=NULL)
	{
		char temp[25];
		GPoint * p = (GPoint *)finger->value;
		sprintf(temp,"[%d,%d]\n",p->i,p->j);
		strcat(note,temp);
		finger = finger->next;
	}
	drawFont(surface, &font,note,10,10);
	}
#endif
	return 1;
}

int movePlayerAI(Player * player)
{
	Brain * brain;
	int ret=0;
	if(!preMovePlayer(player))
		return ret;

	if(!player->brain)
		return ret;
	calculateUtility();
	calculateBombUtility(player);
	brain = (Brain *)player->brain;
	brain->think(brain,player);
	
	if(brain->actionToTake&USE_DROP_BOMB)
	{
		ret|=0x8;
		playerPlaceBomb(player);
	}
	if(brain->actionToTake&USE_ALT_KEY)
	{
		ret|=0x10;
		playerAltKey(player);
	}
	
	if(brain->actionToTake&MOVE)
	{
		switch(brain->direction)
		{
		case UP:
		ret|=(UP|4);
		if(movePlayerUp(player))
		{
			return ret;
		}
		break;
		case DOWN:
		ret|=(DOWN|4);
		if(movePlayerDown(player))
			return ret;
		break;
		case LEFT:
		ret|=(LEFT|4);
		if(movePlayerLeft(player))
			return ret;
		break;
		case RIGHT:
		ret|=(RIGHT|4);
		if(movePlayerRight(player))
			return ret;
		break;
		}
	}else player->frame=0;
	

	return ret;
}

int releaseBrain(Brain * brain)
{
	if(brain->path)
	{
		remAll(brain->path,1);
		free(brain->path);
		brain->path=NULL;
	}
	return 1;
}
