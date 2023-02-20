

typedef struct COMMAND_TYPE
{
	char name[24];
	int (* func)(char * );
}Command;


void initConsole(int nMessages);
void drawConsole();
void addCommand(char * name,int (* func)(char *));
int shutdownConsole();
