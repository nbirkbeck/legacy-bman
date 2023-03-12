#include "menu.h"
#include "sound.h"
#include "globals.h"
#include "network.h"
#include "level.h"
#include "soundSyz.h"
#include <SDL2/SDL.h>
#include <unordered_map>
void updateScreen();

extern SDL_Surface* surface;
extern Font font;
extern Font menuFont,menuFontGlo;
extern int  (* Game_Main_Func)(void);
extern std::unordered_map<int, bool> keys;
extern char fps[50];
extern int menuMidi;



int (* drawMenu)(void);
int (* lastMenu)(void);/* used for drawLevelSelect, so it can go back */

int Game_Main();

int mainMenuSel=0;
char options[5][20]={"Single Player","Multi-Player","High Scores","Options","Exit"};
SDL_Surface* background=NULL;

int drawFPS(int);
int drawTime(int);
int drawMainMenu();
int drawMultiPlayerServerMenu();
int drawMultiPlayerClientMenu();
int drawMultiPlayerMenu();
int drawSinglePlayerMenu();

char input[255];
char inputIndex=0;

void gotoGameMain()
{
	Game_Main_Func = Game_Main;
	releaseFont(&menuFontGlo);
	releaseFont(&menuFont);
	SDL_FreeSurface(background);
	background=NULL;
}

int mainMenuActions(int option)
{
	switch(option)
	{
	case 0:
		drawMenu=drawSinglePlayerMenu;
		break;
	case 1:
		drawMenu=drawMultiPlayerMenu;
		break;
	case 2:
		Game_Main_Func = Game_Main;
		releaseFont(&menuFontGlo);
		releaseFont(&menuFont);
		SDL_FreeSurface(background);
		background=NULL;
		break;
	case 3:
	case 4:
		releaseFont(&menuFontGlo);
		releaseFont(&menuFont);
                SDL_FreeSurface(background);
		background=NULL;
		exit(1);
		break;
	}
	return 1;
}

int drawBackground()
{
	static int grid[15][20]=/*
	{{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{2,2,2,2,2,2,2,3,-1,-1,-1,3,-1,2,2,2,2,2,2,2},
	{2,2,2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2,2,2},
	{2,2,2,2,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,-1,2,2,2,2},
	{2,2,2,2,-1,3,-1,-1,-1,-1,1,3,-1,-1,-1,-1,2,2,2,2},
	{2,2,2,-1,-1,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,3,-1,2,2,2},
	{2,2,2,-1,-1,3,-1,-1,-1,-1,-1,-1,-1,3,-1,-1,-1,2,2,2},
	{2,2,2,-1,-1,-1,3,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2},
	{2,2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2,2},
	{2,2,2,2,-1,-1,-1,3,-1,-1,-1,-1,-1,3,-1,-1,2,2,2,2},
	{2,2,2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2,2,2},
	{2,2,2,2,2,2,2,-1,-1,-1,1,-1,-1,2,2,2,2,2,2,2},
	{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
	{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}};*/

	{{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,2,2,2,2},
	{2,2,2,2,2,2,2,2,2,3,2,0,2,2,2,2,2,2,2,2},
	{2,2,2,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,2,2,2},
	{2,2,3,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,2,2,2},
	{2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,2,2},
	{2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2},
	{2,0,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2},
	{2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2},
	{2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2},
	{2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,2,2},
	{2,2,2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,2,0,2},
	{2,0,2,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 ,1,3,2,2},
	{2,2,2,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,2,2,2},
	{2,1,2,2,2,2,2,3,2,2,2,2,2,2,2,2,2,2,2,2},
	{2,2,2,2,2,2,2,2,2,2,2,0,2,2,2,2,2,2,2,2},};

	static int frames=0;
	frames++;
	if(frames==10)
	{
		frames=0;
		int oneToMove=rand()%12;
		int i=2;
		int j=7;
		bool flag=1;
		while(i<13 && flag)
		{
			j=3;
			while(j<17 && flag)
			{
				if(grid[i][j]==3)
				{
					if(oneToMove==0)
					{
						flag=0;
						break;
					}
					else oneToMove--;
				}
				j++;
			}
			if(!flag)
				break;
			i++;
		}
		int dir=rand()%4;
		switch(dir)
		{
		case UP:
			if(grid[i-1][j]==0)
			{
				grid[i-1][j]=3;
				grid[i][j]=0;
			}
			break;
		case DOWN:
			if(grid[i+1][j]==0)
			{
				grid[i+1][j]=3;
				grid[i][j]=0;
			}
			break;
		case LEFT:
			if(grid[i][j-1]==0)
			{
				grid[i][j-1]=3;
				grid[i][j]=0;
			}
			break;
		case RIGHT:
			if(grid[i][j+1]==0)
			{
				grid[i][j+1]=3;
				grid[i][j]=0;
			}
			break;
		}
	}
	for(int i=0;i<15;i++)
	{
		for(int j=0;j<20;j++)
		{
			if(grid[i][j]!=-1)
			drawLevelTexture(grid[i][j],j<<5,i<<5);
		}
	}
	return 1;
}

int drawLevelSelect_leftSelected(char  levelNames[48][48],int * selectedLevel,int * numLevels)
{
	char temp[255];
	(*selectedLevel)--;
	if(*selectedLevel<0)
	{
		*selectedLevel=*numLevels;
		if(*selectedLevel==-1)
			*selectedLevel=0;
	}
	if(*selectedLevel==*numLevels)
	{
		createBasicLevel(rand()%6,rand()%6,rand()%2,rand()%2,rand()%2);
	}
	else
	{
		sprintf(temp,"./data/level/%s",levelNames[*selectedLevel]);
		loadLevel(temp);
	}
	return 1;
}

int drawLevelSelect_rightSelected(char  levelNames[48][48],int * selectedLevel, int * numLevels)
{
	char temp[255];

	(*selectedLevel)++;
	if(*selectedLevel>*numLevels)
		*selectedLevel=0;
	if(*selectedLevel==*numLevels)
	{
		createBasicLevel(rand()%6,rand()%6,rand()%2,rand()%2,rand()%2);
	}
	else
	{
		sprintf(temp,"./data/level/%s",levelNames[*selectedLevel]);
		loadLevel(temp);
	}
	return 1;
}

int drawLevelSelect()
{
	char menuItems[3][25]={"<<",">>","Select"};
	static int selected=0;
	static int selectedLevel=0;
	static char levelNames[48][48];
	static int numLevels=-1;
	char temp[128];
	int x,y;
	drawBackground();

	if(numLevels==-1)
	{
		FILE * file = fopen("data/level/maplist.txt","r");
		numLevels=0;
		if(file!=NULL)
		{
			int i=0;
			fscanf(file,"%d",&numLevels);
			while(i<numLevels)
			{
				fgets(levelNames[i],48,file);
				char * cptr = strchr(levelNames[i],'\n');
				if(cptr!=NULL)
				{
					(*cptr)='\0';
				}
				i++;
			}
		}
		selectedLevel=0;
		
		if(selectedLevel==numLevels)
		{
			createBasicLevel(rand()%6,rand()%6,rand()%2,rand()%2,rand()%2);
		}
		else
		{
			sprintf(temp,"./data/level/%s",levelNames[selectedLevel]);
			loadLevel(temp);
		}
	}
	drawLevelSmall(surface,184,120);
	if(selectedLevel==numLevels)
	{
		sprintf(temp,"Random, Press G to generate");
		
	}
	else
	{	
		sprintf(temp,"./data/level/%s",levelNames[selectedLevel]);
		
	}
	x=320-(getLength(&menuFont,temp)>>1);
	y=120-menuFont.point-5;
	drawFont(surface, &menuFont,temp,x,y);
	
	/*
		draw the menu
	*/
	
	x=320-2*menuFont.point;
	y=360;
	if(selected==0)
          drawFont(surface, &menuFontGlo,menuItems[0],x,y);
	else
          drawFont(surface, &menuFont,menuItems[0],x,y);
	
	x=320+menuFont.point;
	y=360;
	if(selected==1)
		drawFont(surface, &menuFontGlo,menuItems[1],x,y);
	else
		drawFont(surface, &menuFont,menuItems[1],x,y);
	
	x=320-(getLength(&menuFont,menuItems[2])>>1);
	y=380;
	if(selected==2)
		drawFont(surface, &menuFontGlo,menuItems[2],x,y);
	else
		drawFont(surface, &menuFont,menuItems[2],x,y);

	if(keys[SDLK_UP])
	{
		playMenuItemSound();
		if(selected==2)
			selected=0;
		else selected=2;
		keys[SDLK_UP]=0;
	}

	if(keys[SDLK_DOWN])
	{
		playMenuItemSound();
		if(selected<=1)
			selected =2;
		else selected=0;
		keys[SDLK_DOWN]=0;
	}

	if(keys[SDLK_RIGHT])
	{
		playMenuItemSound();
		selected=1;
		drawLevelSelect_rightSelected(levelNames,&selectedLevel,&numLevels);
		keys[SDLK_RIGHT]=0;
	}
	if(keys[SDLK_LEFT])
	{
		playMenuItemSound();
		drawLevelSelect_leftSelected(levelNames,&selectedLevel,&numLevels);
		selected=0;
		keys[SDLK_LEFT]=0;
	}
	if(keys[SDLK_RETURN])
	{
		switch(selected)
		{
		case 0:
			drawLevelSelect_leftSelected(levelNames,&selectedLevel,&numLevels);
			break;
		case 1:
			drawLevelSelect_rightSelected(levelNames,&selectedLevel,&numLevels);
			break;
		case 2:
			drawMenu=lastMenu;
			break;
		}

		
		keys[SDLK_RETURN]=0;
	}
	if(keys['g']||keys['G'])
	{
		playMenuItemSound();
		createBasicLevel(rand()%6,rand()%6,rand()%2,rand()%2,rand()%2);
		keys['g']=keys['G']=0;
	}
	return 1;
}

int drawSinglePlayerMenu()
{
	int x,y;
	int xback=0;
	static int sel=0;
	static int playerStates[4]={0,0,0,0};
	char states[6][25]={"off","keyboard1","keyboard2","keyboard3","keyboard4","AI"};
	int i=0;
	drawBackground();
	y=140;
	xback=320-((getLength(&menuFont,states[1])+getLength(&menuFont,"Player : "))>>1);
	x=xback;
	while(i<4)
	{
		char string[16];
		x=xback;
		sprintf(string,"Player %d: ",i+1);
		drawFont(surface, &menuFont,string,&x,&y);
		sprintf(string,"%s",states[playerStates[i]]);
		if(sel==i)
		{
			if(keys[SDLK_RIGHT])
			{
				playMenuSelectSound();
				playerStates[i]++;
				playerStates[i]%=6;
				keys[SDLK_RIGHT]=0;
			}
			if(keys[SDLK_LEFT])
			{
				playMenuSelectSound();
				playerStates[i]--;
				if(playerStates[i]<0)
					playerStates[i]=5;
				keys[SDLK_LEFT]=0;
			}

			drawFont(surface, &menuFontGlo,string,&x,&y);
		}
		else
		{
			drawFont(surface, &menuFont,string,&x,&y);
		}
		i++;
		y+=(menuFont.point+5);
	}

	x=320-(getLength(&menuFont,"Choose Level")>>1);
	if(sel==4)
		drawFont(surface, &menuFontGlo,"Choose Level",&x,&y);
	else drawFont(surface, &menuFont,"Choose Level",&x,&y);
	y+=(menuFont.point+5);

	x=320-(getLength(&menuFont,"Start")>>1);
	if(sel==5)
		drawFont(surface, &menuFontGlo,"Start",&x,&y);
	else drawFont(surface, &menuFont,"Start",&x,&y);
	y+=(menuFont.point+5);

	x=320-(getLength(&menuFont,"Back")>>1);
	if(sel==6)
		drawFont(surface, &menuFontGlo,"Back",&x,&y);
	else drawFont(surface, &menuFont,"Back",&x,&y);
		
	if(keys[SDLK_UP])
	{
		playMenuItemSound();
		sel--;
		if(sel<0)
			sel=6;
		
		keys[SDLK_UP]=0;
	}
	if(keys[SDLK_DOWN])
	{
		playMenuItemSound();
		sel++;
		if(sel>6)
			sel=0;
		keys[SDLK_DOWN]=0;
	}
	if(keys[SDLK_RETURN])
	{
		playMenuSelectSound();
		switch(sel)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			playerStates[sel]++;
			playerStates[sel]%=6;
			break;
		case 4:
			drawMenu=drawLevelSelect;
			lastMenu=drawSinglePlayerMenu;
			break;
		case 5:
			resetGameState();
			gotoGameMain();

			for(i=0;i<4;i++)
			{
				addSinglePlayer(i,playerStates[i]);
			}

			break;
		case 6:
			drawMenu=drawMainMenu;
			break;

		}
		keys[SDLK_RETURN]=0;
	}
	return 1;
}

int drawMultiPlayerServerMenu()
{
	int x,y;
	int i=1;
	static int nPlayers=2;
	static int multiSel=0;
	static int gametype=0;
	char gameTypes[2][25]={"FFA","Team"};
	char menuItems[6][25]={"Number of Players: ","Game Type: ","Server Name:","Choose Level","Serve","Back"};
	char output[255];
	drawBackground();
	y=140;

	sprintf(output,"%s%d",menuItems[0],nPlayers);
	x=320-(getLength(&menuFont,output)>>1);
	if(multiSel==0)
	{
		drawFont(surface, &menuFontGlo,output,&x,&y);
		if(keys[SDLK_RIGHT])
		{
			nPlayers++;
			if(nPlayers>4)
				nPlayers=4;
			keys[SDLK_RIGHT]=0;
		}
		if(keys[SDLK_LEFT])
		{
			nPlayers--;
			if(nPlayers<=0)
				nPlayers=0;
			keys[SDLK_LEFT]=0;
		}
	}
	else drawFont(surface, &menuFont,output,&x,&y);
	
	y+=menuFont.point+5;
	sprintf(output,"%s%s",menuItems[1],gameTypes[1]);
	x=320-(getLength(&menuFont,output)>>1);
	drawFont(surface, &menuFont,menuItems[1],&x,&y);
	if(multiSel==1)
	{
		drawFont(surface, &menuFontGlo,gameTypes[gametype],&x,&y);
		if(keys[SDLK_RIGHT]||keys[SDLK_LEFT])
		{
			keys[SDLK_RIGHT]=0;
			keys[SDLK_LEFT]=0;
			gametype++;
			gametype%=2;
		}
		
	}
	else drawFont(surface, &menuFont,gameTypes[gametype],&x,&y);

	y+=menuFont.point+5;
	sprintf(output,"%s %s",menuItems[2],input);
	x=320-(getLength(&menuFont,output)>>1);
	if(multiSel==2)
	{
		drawFont(surface, &menuFontGlo,output,&x,&y);
	}
	else drawFont(surface, &menuFont,output,&x,&y);

	y+=menuFont.point+5;
	x=320-(getLength(&menuFont,menuItems[3])>>1);
	if(multiSel==3)
	{
		drawFont(surface, &menuFontGlo,menuItems[3],x,y);
	}
	else drawFont(surface, &menuFont,menuItems[3],x,y);

	y+=menuFont.point+5;
	
	i=4;
	while(i<6)
	{
		x=320-(getLength(&menuFont,menuItems[i])>>1);
		if(i==multiSel)
			drawFont(surface, &menuFontGlo,menuItems[i],x,y);
		else
			drawFont(surface, &menuFont,menuItems[i],x,y);
		y+=(menuFont.point+5);
		i++;
	}	
	
	if(keys[SDLK_UP]||keys[SDLK_LEFT])
	{
		playMenuItemSound();
		multiSel--;
		if(multiSel<0)
			multiSel=5;
		keys[SDLK_UP]=keys[SDLK_LEFT]=0;
	}

	if(keys[SDLK_DOWN]||keys[SDLK_RIGHT])
	{
		playMenuItemSound();
		multiSel++;
		if(multiSel>5)
			multiSel=0;
		keys[SDLK_DOWN]=keys[SDLK_RIGHT]=0;
	}

	
	if(keys[SDLK_RETURN])
	{
		playMenuSelectSound();
		switch(multiSel)
		{
		case 0:
			nPlayers++;
			if(nPlayers>4)
				nPlayers=2;
			break;
		case 1:
			gametype++;
			gametype%=2;
			break;

		case 3:
			lastMenu=drawMultiPlayerServerMenu;
			drawMenu=drawLevelSelect;
			break;
		case 4:
			resetGameState();
			startServer(gametype,nPlayers,input);
			gotoGameMain();
			break;
			
		case 5:
			drawMenu=drawMultiPlayerMenu;
			break;
		}
		keys[SDLK_RETURN]=0;
	}

	return 1;
}


int drawMultiPlayerClientMenu()
{
	int i;
	int x,y;
	static int multiSel=0;
	static int timer=0;
	char menuItems[2][25]={"Connect","Back"};
	drawBackground();

	timer++;


	x=320-(getLength(&menuFont,"Address:")>>1);
	y=200;
	drawFont(surface, &menuFont,"Address:",&x,&y);
	y+=(menuFont.point+5);

	x=320-(getLength(&menuFont,input)>>1);
	if(x<0)
		x=80;
	drawFont(surface, &menuFont,input,&x,&y);
	if((timer>>4)%2==0)
		drawFont(surface, &menuFont,"_",&x,&y);
	else drawFont(surface, &menuFontGlo,"_",&x,&y);
	i=0;
	

	y+=(menuFont.point<<1)+5;

	while(i<2)
	{
		x=320-(getLength(&menuFont,menuItems[i])>>1);
		
		if(i==multiSel)
		{
			drawFont(surface, &menuFontGlo,menuItems[i],x,y);
		}
		else
			drawFont(surface, &menuFont,menuItems[i],x,y);
		
		y+=(menuFont.point+5);
		i++;
	}

	

	if(keys[SDLK_UP]||keys[SDLK_LEFT])
	{
		playMenuItemSound();
		multiSel--;
		if(multiSel<0)
			multiSel=1;
		keys[SDLK_UP]=keys[SDLK_LEFT]=0;
	}

	if(keys[SDLK_DOWN]||keys[SDLK_RIGHT])
	{
		playMenuItemSound();
		multiSel++;
		if(multiSel>1)
			multiSel=0;
		keys[SDLK_DOWN]=keys[SDLK_RIGHT]=0;
	}
	if(keys[SDLK_RETURN])
	{
		playMenuSelectSound();
		switch(multiSel)
		{
		case 0:
			resetGameState();
			connectToServer(input,UDP_PORT);
			gotoGameMain();
			break;

		case 1:
			drawMenu=drawMultiPlayerMenu;
			break;
		}
		keys[SDLK_RETURN]=0;
	}
	return 1;
}
int drawMultiPlayerMenu()
{
	int i;
	int x,y;
	static int multiSel=0;
	char menuItems[3][25]={"Serve","Connect","Main Menu"};

	drawBackground();

	y=200;
	i=0;
	while(i<3)
	{
		x=320-(getLength(&menuFont,menuItems[i])>>1);
		
		if(i==multiSel)
		{
			drawFont(surface, &menuFontGlo,menuItems[i],x,y);
		}
		else
			drawFont(surface, &menuFont,menuItems[i],x,y);
		
		y+=(menuFont.point+5);
		i++;
	}
	if(keys[SDLK_UP])
	{
		playMenuItemSound();
		multiSel--;
		if(multiSel<0)
			multiSel=2;
		keys[SDLK_UP]=0;
	}
	if(keys[SDLK_DOWN])
	{
		playMenuItemSound();
		multiSel++;
		if(multiSel>2)
			multiSel=0;
		keys[SDLK_DOWN]=0;
	}
	if(keys[SDLK_RETURN])
	{
		playMenuSelectSound();
		switch(multiSel)
		{
		case 0:
			drawMenu=drawMultiPlayerServerMenu;
			break;
		case 1:
			drawMenu=drawMultiPlayerClientMenu;
			inputIndex=0;
			input[0]='\0';
			break;
		case 2:
			drawMenu=drawMainMenu;
			break;
		}
		keys[SDLK_RETURN]=0;
	}
	return 1;
}



int drawMainMenu()
{
	int i=0;
	int j=0;
	int x=200;
	int y=200;
	
	static int mainMenuSel=0;

        SDL_Rect src_rect = {0, 0, 640, 480};
	SDL_BlitSurface(background, &src_rect, surface, &src_rect);
	while(i<5)
	{
		j=0;
		x=320-(getLength(&menuFont,options[i])>>1);
		
		if(i==mainMenuSel)
		{
			drawFont(surface, &menuFontGlo,options[i],x,y);
		}
		else
			drawFont(surface, &menuFont,options[i],x,y);
		
		y+=(menuFont.point+5);
		i++;
	}

	if(keys[SDLK_UP])
	{
		playMenuItemSound();
		mainMenuSel--;
		if(mainMenuSel<0)
			mainMenuSel=4;
		keys[SDLK_UP]=0;
	}

	if(keys[SDLK_DOWN])
	{
		playMenuItemSound();
		mainMenuSel++;
		if(mainMenuSel>=5)
			mainMenuSel=0;
		keys[SDLK_DOWN]=0;
	}
	
	if(keys[SDLK_RETURN])
	{
		inputIndex=0;
		input[0]='\0';
		playMenuSelectSound();
		mainMenuActions(mainMenuSel);
		keys[SDLK_RETURN]=0;
	}
	return 1;
}

int Game_Menu()
{
  SDL_FillRect(surface, nullptr, 0);

	if(menuFont.point<=0||menuFontGlo.point<=0)
	{
		loadFont(&menuFont,"./fonts/font30_2",30);
		loadFont(&menuFontGlo,"./fonts/font30_2_glo",30);

                background = SDL_LoadBMP("./data/main.bmp");

		drawMenu=drawMainMenu;
#ifdef ENABLE_SOUND
		DMusic_Play(menuMidi);
#endif
	}

	drawMenu();
#ifdef ENABLE_SOUND
	drawSoundSyz();
#endif
	drawFPS(450);
	drawConsole();
	updateScreen();
	return 1;
}

int MenuKeyHandler(int key)
{
	if(key==SDLK_AC_BACK)
	{
		if(inputIndex>0)
			inputIndex--;
		input[inputIndex]='\0';
	}
	else
	{
		if(key>=32&&key<128)
		{
			input[inputIndex++]=key;
			input[inputIndex]='\0';
		}
	}
	return 1;
}
