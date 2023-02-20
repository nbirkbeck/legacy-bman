#include "player.h"
#include "font.h"
#include "soundSyz.h"
#include <SDL2/SDL_surface.h>

extern SDL_Surface* surface;
extern Font font;
extern BITMAP_FILE bitFile;

#define SCORE_BOARD_TEX 20
#define DEFAULT_SPEED 2
int frameOrder[]={0,0,0,1,1,1,1,0,0,0,2,2,2,2};
int numDeathFrames=33;
int deathFrameOrder[]={12,12,12,13,13,13,14,14,14,15,15,15,16,16,16,17,17,17,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19};



int preMovePlayer(Player * player)
{

	if(player->state!=PLAYER_NORMAL)
		return 0;

	player->frame++;
	player->frame=player->frame%NUM_PLAYER_FRAMES;

	int current=checkGrid(player->x+16,player->y+16);

	if(current>3)
	{
		pickupPowerUp((player->y+16-YOFFSET)>>5,(player->x+16-XOFFSET)>>5);
		if(current&BOMB)
		{
			if(player->nBombs<MAX_BOMBS_PLAYER)
				player->nBombs++;
		}
		else if(current&FLAME)
		{
			if(player->power<MAX_POWER_PLAYER)
			player->power++;
		}
		else if(current&SPEED)
		{
			player->speed=4;
		}
		else if(current&KICK)
		{
			player->powerups=KICK;
		}
		else if(current&DETONATOR)
		{
			player->powerups=DETONATOR;
		}
		else if(current&DEATH)
		{
		}
		else if(current&PUNCH)
		{
		}
	}
	return 1;
}
/*
	All the movePlayer** functions will return 1 if the
	player was actually moved, and 0 if the player could
	not move in that direction
*/
int movePlayerUp(Player * player)
{
	int current=checkGrid(player->x+16,player->y+16);
	int mod =(player->x-XOFFSET)%32;
	if(mod<=8)
		player->x-=mod;
	else if(mod>=24)
		player->x+=(32-mod);

	player->dir=UP;

	/*
		The player is on a bomb
	*/
	if(current==OCCUPIED)
	{
		if(checkGrid(player->x+4,player->y-16)&&
			checkGrid(player->x+28,player->y-16))
		{
			player->y-=player->speed;
			return 1;
		}
	}
	else
	{
		if(checkGrid(player->x+4,player->y-player->speed)&&
			checkGrid(player->x+28,player->y-player->speed))
		{
			player->y-=player->speed;
			return 1;
		}
	}
	return 0;
}
int movePlayerDown(Player * player)
{
	int current=checkGrid(player->x+16,player->y+16);
	int mod =(player->x-XOFFSET)%32;
	if(mod<=8)
		player->x-=mod;
	else if(mod>=24)
		player->x+=(32-mod);
	player->dir=DOWN;
	
	if(current==OCCUPIED)
	{
		if(checkGrid(player->x+4,player->y+48)&&
			checkGrid(player->x+28,player->y+48))
		{
			player->y+=player->speed;
			return 1;
		}
	}
	else
	{
		if(checkGrid(player->x+4,player->y+32+player->speed)&&
			checkGrid(player->x+28,player->y+32+player->speed))
		{
			player->y+=player->speed;
			return 1;
		}
	}
	return 0;
}
int movePlayerLeft(Player * player)
{
	int current=checkGrid(player->x+16,player->y+16);
	int mod =(player->y-YOFFSET)%32;
	if(mod<=8)
		player->y-=mod;
	else if(mod>=24)
		player->y+=(32-mod);
	player->dir=LEFT;
	
	if(current==OCCUPIED)
	{
		if(checkGrid(player->x-16,player->y+4)&&
		checkGrid(player->x-16,player->y+28))
		{
			player->x-=player->speed;
			return 1;
		}
	}
	else if(checkGrid(player->x-player->speed,player->y+4)&&
		checkGrid(player->x-player->speed,player->y+28))
	{
			player->x-=player->speed;
			return 1;
	}

	return 0;
}
int movePlayerRight(Player * player)
{
	int current=checkGrid(player->x+16,player->y+16);
	int mod =(player->y-YOFFSET)%32;
	if(mod<=8)
		player->y-=mod;
	else if(mod>=24)
		player->y+=(32-mod);
	player->dir=RIGHT;
	if(current==OCCUPIED)
	{
		if(checkGrid(player->x+48,player->y+4)&&
			checkGrid(player->x+48,player->y+28))
		{
			player->x+=player->speed;
			return 1;
		}
	}
	else if(checkGrid(player->x+32+player->speed,player->y+4)&&
		checkGrid(player->x+32+player->speed,player->y+28))
	{
		player->x+=player->speed;
		return 1;
	}
	return 0;
}

int initPlayer(Player * player)
{
	player->state=PLAYER_NORMAL;
	player->x=XOFFSET;
	player->y=YOFFSET;
	player->nBombs=3;
	player->nBombsInUse=0;
	player->speed=DEFAULT_SPEED;
	player->bombsInUse.head=NULL;
	player->power=3;
	player->dir=DOWN;
	player->frame=0;
	player->brain=NULL;
	player->nkills=0;
	player->ndeaths=0;
	player->nsuicides=0;
	player->movePlayerFunc=NULL;
	sprintf(player->name,"No Name");
	return 1;
}

/*
*/
Bomb * playerPlaceBomb(Player * player)
{
	if(player->nBombsInUse<player->nBombs)
	{
		Bomb * b = placeBomb(player->clientNum,(16+player->y-YOFFSET)>>5,(16+player->x-XOFFSET)>>5);
		if(b==NULL)
			return NULL;
		if(player->powerups&DETONATOR)
			b->state=BOMB_DETONATOR;
		insert(&player->bombsInUse,b);
		b->power=player->power;
		b->owner= (void *)player;
		player->nBombsInUse++;
		return b;
	}
	return NULL;
}

int playerAltKey(Player * player)
{
	if(player->powerups&DETONATOR)
	{
		Node * finger = player->bombsInUse.head;
		while(finger!=NULL)
		{
			Bomb * b = (Bomb *) finger->value;
			if(b->state!=BOMB_EXPLODING)
			{
				explodeBomb(b);
				break;
			}
			finger = finger->next;

		}
		/*
		if(player->bombsInUse.head!=NULL)
		{
			explodeBomb((Bomb *)player->bombsInUse.head->value);
		}*/
	}
	else if(player->powerups&KICK)
	{
		int i=(player->y+16-YOFFSET)>>5;
		int j=(player->x+16-XOFFSET)>>5;
		if(levelKick(i,j,player->dir))
		{
			playKickSound();
			return 1;
		}
		switch(player->dir)
		{
		case UP:
			i--;
			break;
		case DOWN:
			i++;
			break;
		case RIGHT:
			j++;
			break;
		case LEFT:
			j--;
			break;
		}
		if(levelKick(i,j,player->dir))
		{
			playKickSound();
			return 1;
		}
	}
	return 0;
}

int loadPlayerTextures(Player * player, const char * filename)
{
  int i = 0, y = 0;
	SDL_Surface* temp = SDL_LoadBMP(filename);

	while(i<12)
	{
          player->textures[i]= SDL_CreateRGBSurface(0,32,64,32, 0,0,0,0);
                SDL_Rect rect = {0, y, 32, 64};
		SDL_BlitSurface(temp, &rect, player->textures[i], 0);
                SDL_SetColorKey(player->textures[i], 1, 0);
		y+=64;
		i++;
	}
	y=0;
	while(i<21)
	{
          player->textures[i]= SDL_CreateRGBSurface(0,32,64,32, 0,0,0,0);
                SDL_Rect rect = {32, y, 32, 64};
		SDL_BlitSurface(temp, &rect, player->textures[i], 0);
                SDL_SetColorKey(player->textures[i], 1, 0);
		y+=64;
		i++;

	}
	

	SDL_FreeSurface(temp);
	return 1;
}

int releasePlayerTextures(Player * player)
{
	int i=0;
	remAll(&player->bombsInUse);	
	while(i<21)
	{
		if(player->textures[i])
		{
                  SDL_FreeSurface(player->textures[i]);
			player->textures[i]=NULL;
		}
		i++;
	}
	return 1;
}

int killPlayer(Player * player,Bomb * bomb)
{
	if(player->state==PLAYER_NORMAL)
	{
		player->state=PLAYER_DYEING;
		player->frame=0;
		player->ndeaths++;
		if(bomb->owner!=player)
		{
			((Player *)bomb->owner)->nkills++;
			
		}
		else 
			player->nsuicides++;
		
	}
	return 1;
}

int drawPlayer(Player * player)
{
	if(player->state==PLAYER_NORMAL)
	{
		if(player->dir==UP)
		{
			DDraw_Draw_Surface(player->textures[0+frameOrder[player->frame]],player->x,player->y-32,32,64,surface,1);
		}
		else if(player->dir==DOWN)
		{
			DDraw_Draw_Surface(player->textures[3+frameOrder[player->frame]],player->x,player->y-32,32,64,surface,1);
		}
		else if(player->dir==LEFT)
		{
			DDraw_Draw_Surface(player->textures[6+frameOrder[player->frame]],player->x,player->y-32,32,64,surface,1);
		}
		else if(player->dir==RIGHT)
		{
			DDraw_Draw_Surface(player->textures[9+frameOrder[player->frame]],player->x,player->y-32,32,64,surface,1);
		}
		
		char pos[300];
		sprintf(pos,"%d/%d",player->nBombsInUse,player->nBombs);
		drawFont(surface, &font,pos,player->x-16,player->y);
		
		return 1;
	}
	else
	{
		//DDraw_Draw_Surface(player->textures[deathFrameOrder[player->frame]],player->x,player->y-32,32,64,surface,1);
		DDraw_DrawSized_Surface(player->textures[deathFrameOrder[player->frame]],player->x-player->frame,player->y-32-player->frame,32,64,32+(player->frame<<1),
																			64+(player->frame<<1),surface,1);
		player->frame++;
		if(player->frame>=numDeathFrames)
		{
			player->state=PLAYER_NORMAL;
			player->frame=0;
		}
		return 1;
	}
}

int drawScoreBoardPlayer(Player * p,int x, int y)
{
	DDraw_Draw_Surface(p->textures[SCORE_BOARD_TEX],x,y,32,64,surface,1);
	return 1;
}
