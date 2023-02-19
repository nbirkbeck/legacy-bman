#include "stdafx.h"
#include "MyTimer.h"
#include "dxgl.h"
#include "t3dlib3.h"
#include <time.h>
#include "menu.h"
#include "font.h"
#include "bomb.h"
#include "level.h"
#include "player.h"
#include "ai.h"
#include "globals.h"
#include "soundSyz.h"
#include "queue.h"
#include "BFSearch.h"
#include "network.h"

HANDLE networkHandles[4];

int server=0;

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam);
int Game_Init();
int Game_Shutdown(void *parms = NULL, int num_parms = 0);

HINSTANCE             hinstance_app;
HWND                  main_window_handle;
DDSCAPS2              ddscaps; 
DDSURFACEDESC2        ddsd;
LPDIRECTDRAWSURFACE7  lpddsprimary = NULL;   // dd primary surface
LPDIRECTDRAWSURFACE7  lpddsback    = NULL;
LPDIRECTDRAWCLIPPER   lpddclipper  = NULL;
LPDIRECTDRAWSURFACE7  consoleTexture=NULL;
LPDIRECTDRAW7         lpdd = NULL;

FILE *                debug;
MyTimer               myTimer;


bool	              keys[256];
char                  fps[50];
bool                  fullscreen=false;
bool				  console=false;
char                  str[1024];
char                  chars[255];
int                   stri=0;
int dtime=0;
int ltime=0;

Font font;
Font smallFont;
Font menuFont,menuFontGlo;

BITMAP_FILE bitFile;
int (* Game_Main_Func)(void);
int (* keyHandler)(int) =NULL;
int menuMidi;
List playerList;
Player players[4];

Brain brains[4];

int curClientNum=0;

bool g_drawTime=false;
bool g_drawFPS=true;
int shadeType=1;

typedef struct COMMAND_TYPE
{
	char name[24];
	int (* func)(char * );
}Command;

Command commands[12];
int nCommands=11;

int drawConsole()
{
	int i=0;
	static unsigned int blink=0;
	int x,y;
	if(!console)
		return 0;
	
	//fade(0.5);
	while(i<640)
	{
		DDraw_Draw_Surface(consoleTexture,i,0,32,32,lpddsback,1);
		i+=32;
	}
	x=10;y=12;
	drawFont(&font,">",&x,&y);
	drawFont(&font,str,&x,&y);
	blink++;
	if((blink>>4)%2)
		drawFont(&font,"_",&x,&y);
	return 1;
}

int addDirFunc(char * line)
{
	char command[25];
	char dir[255];
	char * cptr;
	if((cptr=strchr(line,' '))!=NULL)
	{
		strcpy(dir,cptr+1);
		loadDirectory(dir);
		char temp[64];
		sprintf(temp,"<%s>",dir);
		MessageBox(NULL,temp,temp,MB_OK);
	}
	return 1;
}

int nameFunc(char * line)
{
	char * c;
	if((c=strchr(line,' '))!=NULL)
	{
		sprintf(players[curClientNum].name,c+1);
	}
	return 1;
}

int nullFunc(char * line)
{
	return 1;
}

int drawFPSFunc(char * line)
{
	char command[25];
	sscanf(line,"%s %d",command,&g_drawFPS);
	return 1;
}
int drawTimeFunc(char * line)
{
	char command[25];
	sscanf(line,"%s %d",command,&g_drawTime);
	return 1;
}
int exitFunc(char * line)
{
	PostQuitMessage(0);
	
	return 1;
}
int shadeTypeFunc(char * line)
{
	char command[25];
	sscanf(line,"%s %d",command,&shadeType);
	return 1;
}


void gotoMenu()
{
	Game_Main_Func = Game_Menu;
}

void  updateScreen();
void fade(float pct);
int movePlayerKeyboard1(Player * p);
int movePlayerKeyboard2(Player * p);


void fadeOut()
{
	float pct=1.0f;
	while(pct>=0)
	{
		fade(pct);
		pct-=0.05f;
		updateScreen();
	}
}

void fade(int sx, int sy, int ex, int ey)
{
	int x, y;
  static int fn=0;
  UCHAR r, g, b;
  USHORT color;
  memset(&ddsd,0,sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  lpddsback->Lock(NULL,
            &ddsd,
            DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
            NULL);
  int pitch = ddsd.lPitch/2;
  USHORT * buffer = (USHORT *) ddsd.lpSurface;
 
  buffer+=(sy*pitch);
  int lineDiff = pitch-ex;
 //y=0,x=0 for entire screen
  
  for (y=sy; y<ey;y++, buffer+=((lineDiff)))
  {
	  buffer+=(sx);
    for (x=sx; x<ex; x++,buffer++)
    {
      // first, get the pixel
      color = *buffer;

      // now extract red, green, and blue
      r = color>>12;
      g = (color & 0x0730) >> 6;
      b = (color & 0x1F) >>1;
 
	  color=(r<<11);
	  color|=(g<<5);
	  color|=b;
	  
      // write the new color back to the buffer
      *buffer = color;
    }
  }
  lpddsback->Unlock(0);
}

void fade(float pct)
{
  int x, y;
  static int fn=0;
  UCHAR r, g, b;
  USHORT color;
  memset(&ddsd,0,sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  lpddsback->Lock(NULL,
            &ddsd,
            DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
            NULL);
  int pitch = ddsd.lPitch/2;
  USHORT * buffer = (USHORT *) ddsd.lpSurface;
 
  int lineDiff = pitch-SCREEN_WIDTH;
 //y=0,x=0 for entire screen
  for (y=0; y<SCREEN_HEIGHT;y++, buffer+=((lineDiff)))
  {
    for (x=0; x<SCREEN_WIDTH; x++,buffer++)
    {
      // first, get the pixel
      color = *buffer;

      // now extract red, green, and blue
      r = color>>11;
      g = (color & 0x0730) >> 5;
      b = (color & 0x1F) ;

	  
      // apply the fade
      r =(UCHAR)((float) r*pct);
      g =(UCHAR)((float) g*pct);
      b =(UCHAR)((float) b*pct);

	  color=(r<<11);
	  color|=(g<<5);
	  color|=b;
	  
      // write the new color back to the buffer
      *buffer = color;
    }
  }
  lpddsback->Unlock(0);
}

int movePlayerKeyboard(Player * player,int * keyBindings)
{

	int ret=0;

	if(!preMovePlayer(player))
		return ret;

	/*
		First  key is keyup
	*/
	if(keys[keyBindings[0]])
	{
		movePlayerUp(player);
		ret = UP|0x4;
	}
	/*
		Second key is keydown
	*/
	else if(keys[keyBindings[1]])
	{
		movePlayerDown(player);
		ret = DOWN|0x4;
	}
	/*
		This index 2 key is left
	*/
	else if(keys[keyBindings[2]])
	{
		movePlayerLeft(player);
		ret = LEFT|4;
	}
	/*
		The third index is right
	*/
	else if(keys[keyBindings[3]])
	{
		movePlayerRight(player);
		ret = RIGHT|4;
	}
	else 
	{
		player->frame=0;
	}
	/*
		The fourth index is dropbomb key
	*/
	if(keys[keyBindings[4]])
	{
		Bomb * b = playerPlaceBomb(player);
		/*if(b)
		{
			player->x=b->x;
			player->y=b->y;
		}*/
		keys[keyBindings[4]]=0;
		ret|=0x8;
	}
	/*
		Finally the fifth key is the alt key
	*/
	if(keys[keyBindings[5]])
	{
		playerAltKey(player);
		keys[keyBindings[5]]=0;
		{
			char * message = (char *) malloc(1000);
				sprintf(message,"alt key hit");
			addMessage(message,&font,player->x,player->y,100);
		}
		ret|=0x10;
	}
	return ret;

}

int movePlayerKeyboard1(Player * p)
{
	int k1[]={'W','S','A','D','Z','X'};
	return movePlayerKeyboard(p,k1);
}
int movePlayerKeyboard2(Player * p)
{
	int k2[]={VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,'K','M'};
	return movePlayerKeyboard(p,k2);
}

int movePlayers()
{
	
		//sendByte(movePlayerKeyboard(&players[0],k1));
	Node * finger = playerList.head;
	while(finger!=NULL)
	{
		Player * player = (Player *)finger->value;
		if(player->movePlayerFunc)
			sendByte(player->movePlayerFunc(player));
		finger=finger->next;
	}
	return 1;
}

void  updateScreen()
{

	if(fullscreen)
		while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
	else
	{
		RECT rect;
		POINT p1;
		POINT p2;

		ZeroMemory(&rect, sizeof( rect ));

		// get the client area
		GetClientRect(main_window_handle, &rect);

		// copy the rect's data into two points
		p1.x = rect.left;
		p1.y = rect.top;
		p2.x = rect.right;
		p2.y = rect.bottom;

		// convert it to screen coordinates (like DirectDraw uses)
		ClientToScreen(main_window_handle, &p1);
		ClientToScreen(main_window_handle, &p2);

		// copy the two points' data back into the rect
		rect.left   = p1.x;
		rect.top    = p1.y;
		rect.right  = p2.x;
		rect.bottom = p2.y;

		// blit the back buffer to our window's position
		lpddsprimary->Blt(&rect, lpddsback, NULL, DDBLT_WAIT, NULL);
	}
}

int drawTime(int y)
{
	char time_str[64];
	time_t t;
	time(&t);
	
	strftime(time_str,64,"Time %I:%M:%S",localtime(&t));
	drawFont(&font,time_str,10,y);
	return y+=font.point;
}


int drawFPS(int y)
{
	static int frames=0;
	static int stime=GetTickCount();
	static int time=0;
	frames++;
		   
	if(frames==20)
	{
			   
		time=GetTickCount()-stime;
		if(time==0)
			sprintf(fps, "Fps:  infinite\n");
		else
			sprintf(fps,"Fps  %2.7f\n",((float)20/(float)time)*1000.0f);
		time=0;
		stime=GetTickCount();
		frames=0;
	}
	drawFont(&font,fps,10,y);
	return y+font.point;
}

inline int drawPlayers()
{
	Node * finger;
	finger = playerList.head;
	while(finger!=NULL)
	{
		Player * player = (Player *) finger->value;
		drawPlayer(player);
		finger = finger->next;
	}
	return 1;
}

int Game_Main()
{	
	static int degree=0;
	static int timer=0;
	
	static int count=0;
	
	int i=0;
	int x,y;
	x=0;
	y=0;

	DDraw_Fill_Surface(lpddsback,0);

	drawLevel();

	drawPlayers();

	movePlayers();

	y=450;
	if(g_drawFPS)
	{
		y=drawFPS(y);
	}
	if(g_drawTime)
	{
		y=drawTime(y);
	}
	drawSoundSyz();
	drawMessages();
	drawConsole();
	
	updateScreen();
	
	
	return 1;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
  // TODO: Place code here.
  // TODO: Place code here.
	int time=0;
	
	WNDCLASSEX winclass; // this will hold the class we create
	HWND	   hwnd;	 // generic window handle
	MSG		   msg;		 // generic message
  //HDC        hdc;      // graphics device context

	// first fill in the window class stucture
	winclass.cbSize         = sizeof(WNDCLASSEX);
	winclass.style			= CS_DBLCLKS | CS_OWNDC | 
							  CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc	= WindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= hInstance;
	winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor		= LoadCursor(NULL, IDC_ARROW); 
	winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName	= NULL;
	winclass.lpszClassName	= "GAMEWINDOW";
	winclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

	// save hinstance in global
	hinstance_app = hInstance;

	// register the window class
	if (!RegisterClassEx(&winclass))
		return(0);

	// create the window
	if(fullscreen)
	{
		if (!(hwnd = CreateWindowEx(NULL,                  // extended style
									"GAMEWINDOW",     // class
									"Bombermang", // title
									WS_POPUP | WS_VISIBLE,
					 				0,0,	  // initial x,y
									SCREEN_WIDTH,SCREEN_HEIGHT,  // initial width, height
									NULL,	  // handle to parent 
									NULL,	  // handle to menu
									hInstance,// instance of this application
									NULL)))	// extra creation parms
		return(0);
	}
	else
	{
		if (!(hwnd = CreateWindowEx(NULL,                  // extended style
									"GAMEWINDOW",     // class
									"Bombermang", // title
									WS_OVERLAPPED | WS_VISIBLE,
					 				0,0,	  // initial x,y
									SCREEN_WIDTH,SCREEN_HEIGHT,  // initial width, height
									NULL,	  // handle to parent 
									NULL,	  // handle to menu
									hInstance,// instance of this application
									NULL)))	// extra creation parms
		return(0);
	}
	main_window_handle = hwnd;

	// initialize game here
	Game_Init();

	Game_Main_Func = Game_Menu;
	// enter main event loop
	
	sprintf(fps,"fps:");

	ltime=GetTickCount();
	while(TRUE)
		{
		dtime=GetTickCount()-ltime;
		ltime=GetTickCount();
		myTimer.start();
		// test if there is a message in queue, if so get it
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		   { 
		   // test if this is a quit
		   if (msg.message == WM_QUIT)
			   break;
		
		   // translate any accelerator keys
		   TranslateMessage(&msg);

		   // send the message to the window proc
		   DispatchMessage(&msg);
		   }
		   Game_Main_Func();
		   
		   myTimer.wait(26);
		   if(keys['P'])
		   {
			//   DSound_Set_Freq(bombSounds[0],8000);
			   
			   myTimer.wait(1000);
		   } 
	}

	Game_Shutdown();

	return(msg.wParam);
	return 0;
}



int Game_Init()
{
	LPDIRECTDRAW lpddTemp;
	LPDIRECTDRAWSURFACE7 temp;
	int i=0;

	//MessageBox(main_window_handle,"Game Init Started","Where",MB_OK);
	
	networkStartup();
	
	//MessageBox(main_window_handle,"Music Init Started","Where",MB_OK);
	//MessageBox(main_window_handle,"Network startup done","Where",MB_OK);
	//MessageBox(main_window_handle,"Threads Started","Where",MB_OK);
	//loadSounds();
	//MessageBox(main_window_handle,"sounds,music,threads done","Where",MB_OK);

	if (FAILED(DirectDrawCreate(NULL, &lpddTemp, NULL)))
			return(0);
	if (FAILED(lpddTemp->QueryInterface(IID_IDirectDraw7,
                               (LPVOID *)&lpdd)))
	{
		return(0);
	}

	if(fullscreen)
	{
		if (FAILED(lpdd->SetCooperativeLevel(main_window_handle, 
											  DDSCL_FULLSCREEN |
											  DDSCL_ALLOWMODEX | 
											  DDSCL_EXCLUSIVE | 
											  DDSCL_ALLOWREBOOT |
											  DDSCL_NORMAL|
											  0)))
		{
			return(0);
		}

		if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT,SCREEN_BPP ,0,0)))
		return 0;	
	
		/*
			Set up the double buffering of the window
		*/
		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
		ddsd.dwBackBufferCount = 1;

		ddsd.ddsCaps.dwCaps =	DDSCAPS_PRIMARYSURFACE|
								DDSCAPS_COMPLEX|DDSCAPS_FLIP;

			// Use the DirectDraw4 device to create a surface in lpddsprimary
		if(FAILED(lpdd->CreateSurface(&ddsd,&lpddsprimary, NULL)))
			return -1;
		ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
		
		if(FAILED(lpddsprimary->GetAttachedSurface(&ddsd.ddsCaps,&lpddsback)))
			return -1;

		RECT rect[1];
		rect[0].left=0;
		rect[0].top=0;
		rect[0].bottom=SCREEN_HEIGHT-1;
		rect[0].right = SCREEN_WIDTH-1;

		lpddclipper = DDraw_Attach_Clipper(lpddsback,1,(LPRECT)&rect);

	}
	else
	{
		if (FAILED(lpdd->SetCooperativeLevel(main_window_handle, 
											  DDSCL_NORMAL)))
		{
			throw "Failed to set cooperative level in windowed mode";
		}
		ZeroMemory(&ddsd, sizeof( ddsd ));

		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		// create the primary buffer
		if(FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
			throw "Failed to create the primary buffer";

		ZeroMemory(&ddsd, sizeof(ddsd));

		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = 640;
		ddsd.dwHeight = 480;

		// create the back buffer
		if(FAILED(lpdd->CreateSurface(&ddsd, &lpddsback, NULL)))
			throw "Failed to create the back buffer";

		// create the clipper
		if(FAILED(lpdd->CreateClipper(0, &lpddclipper, NULL)))
			throw "Failed to create the clipper";

		if(FAILED(lpddclipper->SetHWnd(0, main_window_handle)))
			throw "Failed to set the clipper's HWND";

		if(FAILED(lpddsprimary->SetClipper(lpddclipper)))
			throw "Failed to set the primary surface's clipper";
	}
	//MessageBox(main_window_handle,"Directx setup done","Where",MB_OK);
	while(i<96)
	{
		font.letter[i]=NULL;
		smallFont.letter[i]=NULL;
		menuFontGlo.letter[i]=NULL;
		menuFont.letter[i]=NULL;
		i++;
	}

	font.point=0;
	smallFont.point=0;
	menuFont.point=0;
	menuFontGlo.point=0;

	loadFont(&font,"./fonts/font",12);
	loadFont(&smallFont,"./fonts/font_small",8);
//	loadFont(&menuFont,".fonts/font30_2",30); 
//	loadFont(&menuFontGlo,".fonts/font30_2_glo",30);
	loadBombTextures();
	loadPowerUpTextures("./data/powerup.bmp");
	createLevelTextures();
	loadLevelTextures("./data/level/back.bmp");
	loadLevel("./data/level/basic level");
	i=0;
	

	initMessages();

	initList(&playerList);

	initPlayer(&players[0]);
	loadPlayerTextures(&players[0],"./data/man1.bmp");
	initPlayer(&players[1]);
	loadPlayerTextures(&players[1],"./data/man2.bmp");
	initPlayer(&players[2]);
	loadPlayerTextures(&players[2],"./data/man3.bmp");
	initPlayer(&players[3]);
	loadPlayerTextures(&players[3],"./data/man4.bmp");

	players[0].clientNum=0;
	players[1].clientNum=1;
	players[2].clientNum=2;
	players[3].clientNum=3;

	players[1].x=PLAYER1_X;
	players[1].y=PLAYER1_Y;

	initBrain(&brains[0]);
	initBrain(&brains[1]);
	initBrain(&brains[2]);
	initBrain(&brains[3]);

	players[0].brain=&brains[0];
	players[1].brain=&brains[1];

	players[0].movePlayerFunc=movePlayerAI;
	players[1].movePlayerFunc=movePlayerAI;

#ifdef DEBUG
	debug=fopen("debug.txt","w");
#endif
	//menuMidi=DMusic_Load_MIDI("./data/sounds/menu.mid");
	insert(&playerList,&players[0]);
	insert(&playerList,&players[1]);

	//MessageBox(main_window_handle,"Game Init Finished","Where",MB_OK);
	consoleTexture = DDraw_Create_Surface(32,32,DDSCAPS_VIDEOMEMORY,0);
	Load_Bitmap_File(&bitFile,"./data/console.bmp");
	Scan_Image_Bitmap16(&bitFile,consoleTexture,0,0,32,32);
	Unload_Bitmap_File(&bitFile);

	initSoundSyz();

	//Init the commands
	{
		sprintf(commands[0].name,"addDir");
		commands[0].func=addDirFunc;

		sprintf(commands[1].name,"connect");
		commands[1].func=connectFunc;

		sprintf(commands[2].name,"disconnect");
		commands[2].func=disconnectFunc;

		sprintf(commands[3].name,"drawfps");
		commands[3].func=drawFPSFunc;

		sprintf(commands[4].name,"drawtime");
		commands[4].func=drawTimeFunc;

		sprintf(commands[5].name,"exit");
		commands[5].func=exitFunc;

		sprintf(commands[6].name,"kick");
		commands[6].func=exitFunc;

		sprintf(commands[7].name,"map");
		commands[7].func=nullFunc;	

		sprintf(commands[8].name,"name");
		commands[8].func=nameFunc;
		
		sprintf(commands[9].name,"servername");
		commands[9].func=serverNameFunc;

		sprintf(commands[10].name,"shade");
		commands[10].func=shadeTypeFunc;
	}
	keyHandler=MenuKeyHandler;
	return 0;
}

int removePlayer(int num)
{

	rem(&playerList,(void *)&players[num]);
	return 1;
}

int addSinglePlayer(int num,int type)
{
	if(type==0)
		return 0;
	insert(&playerList,&players[num]);
	switch(type)
	{
	case 1:
		players[num].movePlayerFunc=movePlayerKeyboard1;
		break;
	case 2:
		players[num].movePlayerFunc=movePlayerKeyboard2;
		break;
	case 3:
		players[num].movePlayerFunc=movePlayerKeyboard1;
		break;
	case 4:
		players[num].movePlayerFunc=movePlayerKeyboard2;
		break;
	case 5:
		/*
			It is ok to do this since the brains have already been
			inited, so we will free any memory
		*/
		if(brains[num].path!=NULL)
		{
			remAll(brains[num].path,1);
			free(brains[num].path);
		}
		initBrain(&brains[num]);
		players[num].brain=&brains[num];
		players[num].movePlayerFunc=movePlayerAI;
		break;
	case 6:
		players[num].movePlayerFunc=NULL;
		break;
	}
	return 1;
}

int resetGameState()
{
	initPlayer(&players[0]);
	initPlayer(&players[1]);
	initPlayer(&players[2]);
	initPlayer(&players[3]);
	remAll(&playerList);
	players[0].x=PLAYER0_X;
	players[0].y=PLAYER0_Y;
	players[1].x=PLAYER1_X;
	players[1].y=PLAYER1_Y;
	players[2].x=PLAYER2_X;
	players[2].y=PLAYER2_Y;
	players[3].x=PLAYER3_X;
	players[3].y=PLAYER3_Y;
	players[0].clientNum=0;
	players[1].clientNum=1;
	players[2].clientNum=2;
	players[3].clientNum=3;
	return 1;
}

int Game_Shutdown(void *parms, int num_parms)
{
	int i=0;
	int j=0;

	

	releaseFont(&font);
	releaseFont(&menuFont);
	releaseFont(&menuFontGlo);
	releaseBombTextures();
	releaseLevelTextures();
	releasePowerUpTextures();
	
	//MessageBox(NULL,"Releasing p text1","",MB_OK);

	releasePlayerTextures(&players[0]);
	//MessageBox(NULL,"Releasing p text2","",MB_OK);
	releasePlayerTextures(&players[1]);
	//MessageBox(NULL,"Releasing p text3","",MB_OK);
	releasePlayerTextures(&players[2]);
	//MessageBox(NULL,"Releasing p text4","",MB_OK);
	releasePlayerTextures(&players[3]);

	//MessageBox(NULL,"Rem all","",MB_OK);

	remAll(&playerList);

	releaseBrain(&brains[0]);
	releaseBrain(&brains[1]);
	releaseBrain(&brains[2]);
	releaseBrain(&brains[3]);

	//MessageBox(NULL,"console tex","",MB_OK);

	if(consoleTexture)
	{
		consoleTexture->Release();
		consoleTexture=NULL;
	}
//MessageBox(NULL,"dd stuff","",MB_OK);

	lpddclipper->Release();
	lpddsback->Release();
	lpddsprimary->Release();
	lpddsback=lpddsprimary=NULL;
	
	networkShutdown();

	releaseSoundSyz();
	
#ifdef DEBUG
	fclose(debug);
#endif

	keyHandler = MenuKeyHandler;
	return 1;
}

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
	PAINTSTRUCT		ps;
	HDC				hdc;

	// what is the message
	sendMessageSoundSyz(msg,wparam,lparam);
	switch(msg)
	{	
		
		case WM_MOUSEMOVE:
			
			return 0;
		case WM_LBUTTONDOWN:

			return(0);
		case WM_CREATE: 
			return(0);
			break;
		case WM_CHAR:
		{
			if(console)
			{
				if(wparam=='~' || wparam=='`')
				{

					console=false;
					return 1;
				}
				if(wparam>=32&&wparam<128)
				{
					str[stri++]=wparam;
					str[stri]='\0';
					keys[wparam]=false;
				}
				else if(wparam==VK_TAB)
				{
					int c=0;
					char * start;
					if(str[0]=='/'||str[0]=='\\')
					{
						start=(str+1);
					}
					else start = str;
				
					
					while(c<nCommands)
					{
						char * cptr=NULL;
						
						cptr = strstr(commands[c].name,start);
						if(cptr==commands[c].name)
						{
							sprintf(str,"\\%s ",commands[c].name);
							stri=strlen(str);
							playMenuItemSound();
							break;
						}
						c++;
						//playMenuItemSound();
					}
				}
				else if(wparam==VK_BACK)
				{
					stri--;
					if(stri<0)stri=0;
						str[stri]='\0';
				}
				else if(wparam==VK_RETURN)
				{
					char * string;
					if(str[0]=='/'||str[0]=='\\')
					{
						int c=0;
						char  command[255];
						sscanf(str+1,"%s",command);
						while(c<nCommands)
						{

							if(strcmp(commands[c].name,command)==0)
							{
								commands[c].func(str+1);
								break;
							}
							c++;
						}
						stri=0;
						str[stri]='\0';
						return 1;
					}
					string = (char *)malloc(stri+strlen(players[curClientNum].name)+3);
					sprintf(string,"%s: %s",players[curClientNum].name,str);
					addMessage(string,&font,CHAT_X,CHAT_Y,400);
					sendChatClient(string);
					//free(string);
					
					stri=0;
					str[stri]='\0';
				}
				
				return 1;
			}


			if(wparam==VK_ESCAPE)
			{
				Game_Main_Func=Game_Menu;
				keyHandler=MenuKeyHandler;
			}
			
			
			if(Game_Main_Func==Game_Main && wparam=='~' || wparam=='`')
			{
				console=true;
			}
			/*
			else if()
			{
				str[stri++]=wparam;
				str[stri]='\0';
			}*/
			if(keyHandler)
			{
				keyHandler(wparam);
			}
			return(0);
		}
		case WM_PAINT: 
			hdc = BeginPaint(hwnd,&ps);	 
			EndPaint(hwnd,&ps);
			return(0);
   			break;

		case WM_DESTROY: 
			PostQuitMessage(0);
			return(0);
			break;
		
		case WM_KEYDOWN:
		{
			if(console)
				return 1;
			keys[wparam] = TRUE;	
			

			return 0;
		}

		case WM_KEYUP:
		{
			keys[wparam] = FALSE;
			return 0;
		}
		default:
			break;
    }
	return (DefWindowProc(hwnd, msg, wparam, lparam));

}


