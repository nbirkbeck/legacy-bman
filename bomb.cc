#include "bomb.h"
#include "compat.h"
#include "player.h"
#include "level.h"
#include "soundSyz.h"

extern SDL_Surface* surface;
extern int dtime;

SDL_Surface* explosion[4][7];
SDL_Surface* bombTex[4];

unsigned char bombFrameOrder[]={0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1};
unsigned char numBombFrames = 30;
unsigned char explosionFrameOrder[]={0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,3,3,2,2,2,1,1,1,0,0,0};
unsigned char numExplosionFrames = 27;

int loadBombTextures()
{
	int i = 0, x = 0;
	loadExplosionTextures();

        SDL_Surface* temp = SDL_LoadBMP("./data/bomb.bmp");

	while(i<4)
	{
          bombTex[i]= SDL_CreateRGBSurface(0, 32,32, 32, 0, 0, 0, 0);
          SDL_Rect rect = {x, 0, 32, 32};
          SDL_BlitSurface(temp, &rect, bombTex[i], nullptr);
          SDL_SetColorKey(bombTex[i], 1, 0);
          x+=32;
          i++;
	}
	SDL_FreeSurface(temp);
	return 1;	
}



int loadExplosionTextures()
{
	int i = 0, j = 0, x = 0, y = 0;
        SDL_Surface* temp = SDL_LoadBMP("./data/explosion.bmp");

	while(i<NUM_DEGREES)
	{
		
		j=0;
		x=0;
		while(j<7)
		{
			explosion[i][j]=SDL_CreateRGBSurface(0, 32,32, 32, 0, 0, 0, 0);
                        SDL_Rect rect = {x, y, 32, 32};
			SDL_BlitSurface(temp, &rect, explosion[i][j], nullptr);
                        SDL_SetColorKey(explosion[i][j], 1, 0);
			x+=32;
			j++;
		}
		y+=32;
		i++;
	}
        SDL_FreeSurface(temp);
	return 1;
}
int initBomb(Bomb * bomb,int x, int y,int state,int power, void * owner)
{
	bomb->x = x;
	bomb->y = y;
	bomb->state=state;
	bomb->timer=BOMB_DEFAULT_TIMER;
	bomb->power=power;
	bomb->owner=owner;
	bomb->frame=0;
	bomb->dir=0;
	bomb->moving=0;
	return 1;
}

int moveBomb(Bomb * bomb)
{
	bomb->frame++;
	if(bomb->state==BOMB_NORMAL)
	{
		bomb->timer-=dtime;
		if(bomb->timer<=0)
		{
			explodeBomb(bomb);
		}
	}
	if(bomb->state==BOMB_EXPLODING)
	{	
		if(bomb->frame%5==0)
		{
			midExplodeBomb(bomb);
		}
		else if(bomb->frame==numExplosionFrames)
		{
			bomb->state=BOMB_UNUSED;
			postExplodeBomb(bomb);
		}
	}
	
	if(bomb->moving)
	{
		int bi=unmaskGridSquare((bomb->y+16-YOFFSET)>>5,(bomb->x+16-XOFFSET)>>5);
		int changed=1;
		int x,y;
		switch(bomb->dir)
		{
		case UP:
			x=bomb->x+16;
			y=bomb->y-BOMB_SPEED;
			if(checkGrid(x,y)&& !checkForPlayer(x,y))
				bomb->y-=BOMB_SPEED;
			else
			{
				changed=0;
			}
			break;
		case DOWN:
			x=bomb->x+16;
			y=bomb->y+32+BOMB_SPEED;
			if(checkGrid(x,y) && !checkForPlayer(x,y))
				bomb->y+=BOMB_SPEED;
			else
			{
				//bomb->y+=BOMB_SPEED;
				changed=0;
			}
			break;
		case RIGHT:
			x=bomb->x+32+BOMB_SPEED;
			y=bomb->y+16;
			if(checkGrid(x,y) && !checkForPlayer(x,y))
				bomb->x+=BOMB_SPEED;
			else
			{	
				//bomb->x+=BOMB_SPEED;
				changed=0;
			}
			break;
		case LEFT:
			x=bomb->x-BOMB_SPEED;
			y=bomb->y+16;
			if(checkGrid(x,y) && !checkForPlayer(x,y))
				bomb->x-=BOMB_SPEED;
			else
			{
				changed=0;
			}
			break;
		}
		maskGridSquare((bomb->y-YOFFSET+16)>>5,(bomb->x-XOFFSET+16)>>5,bi);
		if(!changed)
		{
			int bi=unmaskGridSquare((bomb->y+16-YOFFSET)>>5,(bomb->x+16-XOFFSET)>>5);
			int modx = (bomb->x-XOFFSET)%32;
			int mody = (bomb->y-YOFFSET)%32;
			bomb->moving=0;
			
			switch(bomb->dir)
			{
			case RIGHT:
				if(modx<=16)
					bomb->x-=modx;
				else bomb->x+=(32-modx);
				break;
			case LEFT:
				if(modx<=16)
					bomb->x-=modx;
				else bomb->x+=(32-modx);
				break;
			case UP:
				if(mody<=16)
					bomb->y-=mody;
				else bomb->y+=(32-mody);
				break;
			case DOWN:
				if(mody<=16)
					bomb->y-=mody;
				else bomb->y+=(32-mody);
				break;
			}
			maskGridSquare((bomb->y-YOFFSET+16)>>5,(bomb->x-XOFFSET+16)>>5,bi);
		}
	}
	return 1;
}

int drawBomb(Bomb * bomb)
{
	if(bomb->state==BOMB_UNUSED)
		return 0;
	if(bomb->state==BOMB_EXPLODING)
	{
		int i=0;
		int temp;
		
		if(bomb->x==0 && bomb->y==0)
		{

			bomb->x=0;
			bomb->y=0;
		}

		DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][MID_EXP],
						bomb->x,bomb->y,32,32,surface,1);
		if(bomb->depths[LEFT]!=0)
		{
			temp = abs(bomb->depths[LEFT]);
			i=temp-1;
			if(bomb->depths[LEFT]>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][LEFT_EXP],
					bomb->x-(temp<<5),bomb->y,32,32,surface,1);
			}
			while(i>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][HOR_EXP],
					bomb->x-(i<<5),bomb->y,32,32,surface,1);
				i--;
			}
		}
		if(bomb->depths[RIGHT]!=0)
		{
			temp = abs(bomb->depths[RIGHT]);
			i=temp-1;
			if(bomb->depths[RIGHT]>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][RIGHT_EXP],
					bomb->x+(temp<<5),bomb->y,32,32,surface,1);
			}
			while(i>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][HOR_EXP],
					bomb->x+(i<<5),bomb->y,32,32,surface,1);
				i--;
			}
		}
		if(bomb->depths[UP]!=0)
		{
			temp=abs(bomb->depths[UP]);
			i=temp-1;
			if(bomb->depths[UP]>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][TOP_EXP],
					bomb->x,bomb->y-(temp<<5),32,32,surface,1);
			}
			while(i>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][VER_EXP],
					bomb->x,bomb->y-(i<<5),32,32,surface,1);
				i--;
			}
		}
		if(bomb->depths[DOWN]!=0)
		{
			temp=abs(bomb->depths[DOWN]);
			i=temp-1;
			if(bomb->depths[DOWN]>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][BOTTOM_EXP],
					bomb->x,bomb->y+(temp<<5),32,32,surface,1);
			}
			while(i>0)
			{
				DDraw_Draw_Surface(explosion[explosionFrameOrder[bomb->frame]][VER_EXP],
					bomb->x,bomb->y+(i<<5),32,32,surface,1);
				i--;
			}

		}
		

	}
	else
	{
		DDraw_Draw_Surface(bombTex[bombFrameOrder[bomb->frame%numBombFrames]],
						bomb->x,bomb->y,32,32,surface,1);
	}
	return 1;
}

int drawExplosionTile(int type,int frame,int x, int y)
{

	frame%=numExplosionFrames;
	DDraw_Draw_Surface(explosion[explosionFrameOrder[frame]][type],
						x,y,32,32,surface,1);
	return 1;
}

int postExplodeBomb(Bomb * bomb)
{
	unmaskGridSquare((bomb->y+16-YOFFSET)>>5,(bomb->x+16-XOFFSET)>>5);
	if(bomb->owner!=NULL)
	{
		((Player *)bomb->owner)->nBombsInUse--;
		rem(&((Player *)bomb->owner)->bombsInUse,bomb);
		
	}
	return 1;
}

int midExplodeBomb(Bomb * bomb)
{
	int bi,bj;
	int i=0;
	int j=0;
	bi = (bomb->y-YOFFSET)>>5;
	bj = (bomb->x-XOFFSET)>>5;

	levelKill(bomb,bi,bj);
	i=1;
	while(i<=abs(bomb->depths[UP]) )
	{
		if((bi-i)<0)
			break;
		else levelKillPlayer(bomb,bi-i,bj);
		i++;
	}

	i=1;
	while(i<=abs(bomb->depths[DOWN]))
	{
		if((bi+i)>=GRID_HEIGHT)
			break;
		else levelKillPlayer(bomb,bi+i,bj);
		i++;
	}

	j=1;
	while(j<=abs(bomb->depths[LEFT]))
	{
		if(j<=0)
			break;
		else levelKillPlayer(bomb,bi,bj-j);
		j++;
	}
	
	j=1;
	while(j<=abs(bomb->depths[RIGHT]))
	{
		if(j>=GRID_WIDTH)
			break;
		else levelKillPlayer(bomb,bi,bj+j);
		j++;
	}	

	return 1;
}

int explodeBomb(Bomb * bomb)
{
	int i=0;
	int bi;
	int bj;
	int j=0;

	if(bomb->moving)
	{
		int bi=unmaskGridSquare((bomb->y+16-YOFFSET)>>5,(bomb->x+16-XOFFSET)>>5);
		int modx = (bomb->x-XOFFSET)%32;
		int mody = (bomb->y-YOFFSET)%32;
		bomb->moving=0;
		
		switch(bomb->dir)
		{
		case RIGHT:
			if(modx<=16)
				bomb->x-=modx;
			else bomb->x+=(32-modx);
			break;
		case LEFT:
			if(modx<=16)
				bomb->x-=modx;
			else bomb->x+=(32-modx);
			break;
		case UP:
			if(mody<=16)
				bomb->y-=mody;
			else bomb->y+=(32-mody);
			break;
		case DOWN:
			if(mody<=16)
				bomb->y-=mody;
			else bomb->y+=(32-mody);
			break;
		}
		maskGridSquare((bomb->y-YOFFSET+16)>>5,(bomb->x-XOFFSET+16)>>5,bi);
	}
	playBombSound();

	bi=(bomb->y-YOFFSET)>>5;
	bj=(bomb->x-XOFFSET)>>5;

	bomb->state=BOMB_EXPLODING;
	bomb->timer=0;
	bomb->frame=0;
	
	levelKill(bomb,bi,bj);

	i=1;
	while(1)
	{
		int ret=levelKill(bomb,bi-i,bj);
		if(ret==NO_BURN)
		{
			i--;
			break;
		}
		else if(ret==BURN_STOP)
		{
			break;
		}
		
		else if(ret==BURN_STOP_BOMB)
		{
			i*=-1;
			break;
		}
		if(i==bomb->power)
		{
			break;
		}
		i++;
	}
	bomb->depths[UP]=i;
	i=1;
	while(1)
	{
		int ret=levelKill(bomb,bi+i,bj);
		if(ret==NO_BURN)
		{
			i--;
			break;
		}
		else if(ret==BURN_STOP)
		{
			break;
		}
	
		else if(ret==BURN_STOP_BOMB)
		{
			i*=-1;
			break;
		}
		
		if(i==bomb->power)
		{
			break;
		}
		i++;
	}
	bomb->depths[DOWN]=i;
	j=1;
	while(1)
	{
		int ret=levelKill(bomb,bi,bj-j);
		if(ret==NO_BURN)
		{
			j--;
			break;
		}

		else if(ret==BURN_STOP)
		{
			break;
		}
		else if(ret==BURN_STOP_BOMB)
		{
			j*=-1;
			break;
		}
		if(j==bomb->power)
		{
			break;
		}
		j++;
	}
	bomb->depths[LEFT]=j;
	j=1;
	while(1)
	{
		int ret=levelKill(bomb,bi,bj+j);
		if(ret==NO_BURN)
		{
			j--;
			break;
		}
		else if(ret==BURN_STOP)
		{
			break;
		}
		else if(ret==BURN_STOP_BOMB)
		{
			j*=-1;
			break;
		}
		if(j==bomb->power)
		{
			break;
		}
		j++;
	}
	bomb->depths[RIGHT]=j;
	
	/*
	unmaskGridSquare((bomb->y-YOFFSET)>>5,(bomb->x-XOFFSET)>>5);

	if(bomb->owner!=NULL)
	{
		((Player *)bomb->owner)->nBombsInUse--;
		rem(&((Player *)bomb->owner)->bombsInUse,bomb);
		
	}*/
	return 1;
}

int releaseBombTextures()
{
	int i=0;
	int j;
	while(i<NUM_DEGREES)
	{
		j=0;
		while(j<7)
		{
			if(explosion[i][j])
			{
                          SDL_FreeSurface(explosion[i][j]);
				explosion[i][j]=NULL;
			}
			j++;
		}
		i++;
	}
	i=0;
	while(i<NUM_BOMBTEX)
	{
		if(bombTex[i])
		{
                  SDL_FreeSurface(bombTex[i]);
			bombTex[i]=NULL;
		}
		i++;
	}

	return 1;
}
