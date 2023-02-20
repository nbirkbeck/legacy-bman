#include "compat.h"
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
#include <SDL2/SDL.h>
#include <cassert>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void* networkHandles[4];

SDL_Surface* consoleTexture;
SDL_Surface* surface;
SDL_Window* window;
SDL_Renderer* renderer;

int server=0;

int Game_Init();
int Game_Shutdown(void *parms = NULL, int num_parms = 0);

FILE *                debug;
MyTimer               myTimer;


std::unordered_map<int, bool> keys;

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

int g_drawTime=false;
int g_drawFPS=true;
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

	SDL_Surface* surface = SDL_GetWindowSurface(window);
	//fade(0.5);
	while(i<640)
	{
          SDL_Rect src_rect= {0, 0, 32, 32};
          SDL_Rect dest_rect= {i, 0, 32, 32};
          SDL_BlitSurface(consoleTexture, &src_rect, surface, &dest_rect);
          i+=32;
	}
	x = 10;
        y = 12;
	drawFont(surface, &font, ">", &x, &y);
	drawFont(surface, &font, str, &x, &y);
	blink++;
	if((blink>>4)%2)
          drawFont(surface, &font,"_",&x,&y);
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
		//MessageBox(NULL,temp,temp,MB_OK);
	}
	return 1;
}

int nameFunc(char * line)
{
	char * c;
	if((c=strchr(line,' '))!=NULL)
	{
          sprintf(players[curClientNum].name, "%s", c+1);
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
        exit(1);
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

  SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_LockSurface(surface);

  int pitch = surface->pitch;
  uint32_t * buffer = (uint32_t*) surface->pixels;
 
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
  SDL_UnlockSurface(surface);
}

void fade(float pct)
{
  int x, y;
  static int fn=0;
  UCHAR r, g, b;
  USHORT color;
  SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_LockSurface(surface);

  int pitch = surface->pitch;
  uint32_t * buffer = (uint32_t*) surface->pixels;
 
  int lineDiff = SCREEN_WIDTH;
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
  SDL_UnlockSurface(surface);
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
  int k1[]={'w','s','a','d','z','x'};
  return movePlayerKeyboard(p,k1);
}

int movePlayerKeyboard2(Player * p)
{
  int k2[]={SDLK_UP,SDLK_DOWN,SDLK_LEFT,SDLK_RIGHT,'k','m'};
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
  SDL_UpdateWindowSurface(window);

#ifdef TODO
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
#endif
}

int drawTime(int y)
{
  char time_str[64];
  time_t t;
  time(&t);
	
  strftime(time_str,64,"Time %I:%M:%S",localtime(&t));
  drawFont(surface, &font,time_str,10,y);
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
  drawFont(surface, &font,fps,10,y);
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
  int x = 0, y = 0;

  SDL_FillRect(surface, nullptr, 0);

  drawLevel();

  drawPlayers();

  movePlayers();

  y=450;
  if(g_drawFPS) {
    y=drawFPS(y);
  }
  if(g_drawTime)
  {
    y=drawTime(y);
  }
  drawSoundSyz();
  drawMessages(surface);
  drawConsole();
	
  updateScreen();
	
  return 1;
}

int HandleInput();

int main(int ac, char* av[]) {
  int time=0;
  Game_Init();

  Game_Main_Func = Game_Menu;
  // enter main event loop
	
  sprintf(fps,"fps:");
  
  ltime = GetTickCount();
  while (true) {
    surface = SDL_GetWindowSurface(window);
    //SDL_LockSurface(surface);
    
    dtime=GetTickCount()-ltime;
    ltime=GetTickCount();
    myTimer.start();

    HandleInput();
    
    Game_Main_Func();
    
    myTimer.wait(26);
    if(keys['P'])
    {
      myTimer.wait(1000);
    }

    //SDL_UnlockSurface(surface);
    SDL_UpdateWindowSurface(window);
  }
  
  Game_Shutdown();

  return 0;
}

int Game_Init()
{
  int i=0;
  std::cerr << "Init\n";

#ifdef ENABLE_NETWORK
  networkStartup();
#endif

  if(fullscreen)
  {
    assert(false);
  }
  else
  {
    int rendererFlags = SDL_RENDERER_SOFTWARE, windowFlags = 0;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
      printf("Couldn't initialize SDL: %s\n", SDL_GetError());
      exit(1);
    }

    window =
      SDL_CreateWindow("Shooter 01", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

    if (!window)
    {
      printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
      exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    renderer = SDL_CreateRenderer(window, -1, rendererFlags);

    if (!renderer)
    {
      printf("Failed to create renderer: %s\n", SDL_GetError());
      exit(1);
    }
  }

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

  std::cerr << "Loading fonts\n";
  loadFont(&font,"fonts/font",12);
  loadFont(&smallFont,"fonts/font_small",8);
  //	loadFont(&menuFont,".fonts/font30_2",30); 
  //	loadFont(&menuFontGlo,".fonts/font30_2_glo",30);
  loadBombTextures();
  loadPowerUpTextures("data/powerup.bmp");
  createLevelTextures();
  loadLevelTextures("data/level/back.bmp");
  loadLevel("data/level/basic level");
  i=0;
	

  initMessages();

  initList(&playerList);

  std::cout << "Loading players\n";
  initPlayer(&players[0]);
  loadPlayerTextures(&players[0],"data/man1.bmp");
  initPlayer(&players[1]);
  loadPlayerTextures(&players[1],"data/man2.bmp");
  initPlayer(&players[2]);
  loadPlayerTextures(&players[2],"data/man3.bmp");
  initPlayer(&players[3]);
  loadPlayerTextures(&players[3],"data/man4.bmp");

  players[0].clientNum=0;
  players[1].clientNum=1;
  players[2].clientNum=2;
  players[3].clientNum=3;

  players[1].x=PLAYER1_X;
  players[1].y=PLAYER1_Y;

#ifdef ENABLE_BRAIN
  initBrain(&brains[0]);
  initBrain(&brains[1]);
  initBrain(&brains[2]);
  initBrain(&brains[3]);
  
  players[0].brain=&brains[0];
  players[1].brain=&brains[1]

  players[0].movePlayerFunc=movePlayerAI;
  players[1].movePlayerFunc=movePlayerAI;
#endif

#ifdef DEBUG
  debug=fopen("debug.txt","w");
#endif
  //menuMidi=DMusic_Load_MIDI("./data/sounds/menu.mid");
  insert(&playerList,&players[0]);
  insert(&playerList,&players[1]);
  
  //MessageBox(main_window_handle,"Game Init Finished","Where",MB_OK);
  consoleTexture = SDL_LoadBMP("data/console.bmp");

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

#ifdef ENABLE_NETWORK    
    sprintf(commands[9].name,"servername");
    commands[9].func=serverNameFunc;
#endif
    sprintf(commands[10].name,"shade");
    commands[10].func=shadeTypeFunc;
  }
  keyHandler=MenuKeyHandler;

  std::cout << "Done init\n";
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

#ifdef ENABLE_AI
  releaseBrain(&brains[0]);
  releaseBrain(&brains[1]);
  releaseBrain(&brains[2]);
  releaseBrain(&brains[3]);
#endif
  //MessageBox(NULL,"console tex","",MB_OK);

#ifdef CONSOLE_TEXTURE
  if(consoleTexture)
  {
    consoleTexture->Release();
    consoleTexture=NULL;
  }
#endif
  
  //MessageBox(NULL,"dd stuff","",MB_OK);
#ifdef ENABLE_NETWORK
  networkShutdown();
#endif

#ifdef ENABLE_SOUND
  releaseSoundSyz();
#endif
  
#ifdef DEBUG
  fclose(debug);
#endif

  keyHandler = MenuKeyHandler;
  return 1;
}


int HandleKeyPress(SDL_Keysym keysym) {
  int wparam = keysym.sym;
  std::cout << "text input:" << wparam << "\n";
  const char* name = SDL_GetKeyName(keysym.sym);
  if (keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
    if (name && name[0] == ';')
      wparam = ':';
  }
  std::cout << "'" << name << "'" << std::endl;
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
        else if(wparam==SDLK_TAB)
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
#ifdef ENABLE_SOUND
              playMenuItemSound();
#endif
              break;
            }
            c++;
            //playMenuItemSound();
          }
        }
        else if(wparam==SDLK_BACKSPACE)
        {
          stri--;
          if(stri<0)stri=0;
          str[stri]='\0';
        }
        else if(wparam==SDLK_RETURN)
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


      if(wparam==SDLK_ESCAPE)
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

int HandleInput() {
  int wparam = 0;
  // what is the message
#ifdef ENABLE_SOUND
  sendMessageSoundSyz(msg,wparam,lparam);
#endif
  SDL_Event event;
  while (SDL_PollEvent(&event)) 
    switch(event.type)
  {	
		
  case SDL_MOUSEMOTION:
    return 0;
  case SDL_MOUSEBUTTONDOWN:
    return 0;
  case SDL_QUIT:
    exit(0);
    break;
     

  case SDL_KEYDOWN:
    {
      SDL_Keysym keysym = event.key.keysym; 
      std::cout << "Keyboard:" << (char)keysym.sym << "\n";
      if (!keys[keysym.sym]) {
        HandleKeyPress(keysym);
      }
      keys[keysym.sym] = true;
            
      return 0;
    }
  case SDL_KEYUP:
    {
      SDL_Keysym keysym = event.key.keysym;
      keys[keysym.sym] = false;

      return 0;
    }
  default:
    break;
  }
  return 0;

}


