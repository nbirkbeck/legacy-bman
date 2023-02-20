#include "console.h"
#include "font.h"


Command * commands;
LPDIRECTDRAWSURFACE7 screen=NULL;

int maxCommands=0;
int numCommands=0;


/*
	void initConsole(int maxCommands)
	pre: nCommands >=0
	post: maxCommands are allocated and must be freed with a call
	to shutdownConsole when done
*/
void initConsole(int max,LPDIRECTDRAWSURFACE7 scr)
{
	commands = (Command *) malloc(sizeof(Command)*max);
	maxCommands=max;
	numCommands=0;
	screen=scr;
}

/*
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
		DDraw_Draw_Surface(consoleTexture,i,0,32,32,scr,1);
		i+=32;
	}
	x=10;y=12;
	drawFont(&font,">",&x,&y);
	drawFont(&font,str,&x,&y);
	blink++;
	if((blink>>4)%2)
		drawFont(&font,"_",&x,&y);
	return 1;
}*/



/*
	void addCommand
	pre: The name must be less than 24 chars, and the function must not be null
	post: The command is added to the list of commands in the consoles
	database
*/
void addCommand(char * name,int (* func)(char *))
{
	commands[numCommands].func=func;
	sprintf(commands[numCommands].name,name);
}

/*
	int shutdownConsole()
	pre: initConsole has been called
	post: the memory allocated is released
*/
int shutdownConsole()
{
	if(commands)
		free(commands);
	return 1;
}
