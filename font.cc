#include "font.h"
#include "list.h"
#include "globals.h"
#include "stdio.h"
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL.h>

extern FILE * debug;
extern int shadeType;

List messages;

SDL_Surface* shadeTextures[4];
/*
	int initMessages()
	pre: none
	post: must be called before any drawMessages function is called
*/
int initMessages()
{
	SDL_Surface* temp = SDL_LoadBMP("fonts/shade.bmp");

        SDL_Rect dest_rect = {0, 0, 16, 16};
        SDL_Rect src_rects[4] = {
                                 {0, 0, 16, 16},
                                 {16, 0, 16, 16},
                                 {0, 16, 16, 16},
                                 {16, 16, 16, 16},
        };
        for (int i = 0; i < 4;++i) {
         shadeTextures[i] = SDL_CreateRGBSurface(0,16,16,32,0,0,0,0);
         SDL_BlitSurface(temp, &src_rects[i], shadeTextures[i], &dest_rect);
        }
	SDL_FreeSurface(temp);
	
	initList(&messages);
	return 1;
}

/*
	int messageShutdown()
	pre:
	post:
*/
int messagesShutdown()
{
	/* Need to free the Message_type allocated */
	Node * finger = messages.head;
	while(finger!=NULL)
	{
		Node * toRelease;
		Message * m = (Message *)finger->value;
		if(m->message)
		{
			free(m->message);
			m->message=NULL;
		}
		if(m)
		{
			free(m);
			m=NULL;
		}
		toRelease=finger;
		finger = finger->next;
		free(toRelease);
	}
	messages.head = nullptr;
	messages.tail = nullptr;
        for (int i = 0; i < 4; ++i) {
          SDL_FreeSurface(shadeTextures[i]);
        }
	return 1;
}
int bumpUp(Message * message);
int bumpUp(Message * message)
{
	Node * finger = (Node *) messages.head;
	while(finger!=NULL)
	{
		Message * inList = (Message *) finger->value;
		if(inList!=message && inList->x == message->x && inList->y==message->y)
		{
			inList->y-=(inList->font->point+2);
			bumpUp(inList);
		}
		finger = finger->next;
	}
	return 1;
}

/*
	int addMessage(char * message,Font * font, int x, int y, int time)
	pre: 
	post:
*/
int addMessage(char * message, Font * font, int x, int y, int time)
{
	Message * m = (Message *)malloc(sizeof(Message));
	m->x=x;
	m->y=y;
	m->time = time;
	m->font = font;
	m->message=message;
	bumpUp(m);
	insert(&messages,m);
#ifdef DEBUG
	fprintf(debug,"inserting %p",m);
#endif
	return 1;
}

/*
	int removeMessage(char * message)
	pre:
	post:
*/
int removeMessage(char * message)
{
	int time=0;
	Node * finger = messages.head;
	while(finger!=NULL)
	{
		Message * m = (Message *) finger->value;
		if(m->message==message)
		{
			time = m->time;
			free(m->message);
			
			rem(&messages,m);
			free(m);
			return time;
		}

		finger = finger->next;
	}
	return time;
}

/*
	int drawMessages()
	pre:  none
	post: if there are any messages, then draw them to the 
		  screen, if there is no messages, then dont worry about it.
		  If the messages timer has elapsed, then remove the message
		  from the queue, and release the memory allocated
*/
int drawMessages(SDL_Surface* surface)
{
	int i=0;
	static int count=0;
	Node * finger = messages.head;

	while(finger!=NULL)
	{
          Message * m = (Message *)finger->value;
          if(m->x==CHAT_X)
          {
            int xx=32;
            switch(shadeType)
            {
            case 0:
            case 1:
            case 2:
            case 3:
              while(xx<m->x+getLength(m->font,m->message))
              {

                SDL_Rect src_rect = {0, 0, 16, 16};
                SDL_Rect dest_rect = {xx, m->y, 16, 14};
                SDL_BlitSurface(shadeTextures[shadeType], &src_rect, surface, &dest_rect);
                xx+=16;
              }
              break;
            case 4:
              fade(m->x,m->y,m->x+getLength(m->font,m->message),m->y+14);
              break;
            }
			
			
            drawFontBound(surface, m->font,m->message,m->x,m->y,608,480);
          }
          drawFont(surface, m->font,m->message,m->x,m->y);
          m->time--;
          finger=finger->next;
		
          if(m->time<=0)
          {

#ifdef DEBUG2
            char filen[255];
            FILE * file;
            sprintf(filen,"%s.txt",m->message);
            file= fopen(filen,"w");
            printList(&messages,file);
            fprintf(file,"freeing %s..",m->message);
#endif

            rem(&messages,m);
            if(m->message)
            {
              free(m->message);
              m->message=NULL;
            }

            if(m)
            {
              free(m);
              m=NULL;
            }

#ifdef DEBUG2
            fprintf(file,"freed\n");
            printList(&messages,file);
            fprintf(file,"Messages Head %d,count:%d\n",messages.head,count);
            if(file)
              fclose(file);
#endif

          }
	}
#ifdef DEBUG2
	fprintf(debug,"draw Message\n");
#endif
	count++;
	return 1;
}

int getLength(Font * font, const char * str)
{
	int i=0;
	int len=0;
	while(i<strlen(str))
	{
		len+=font->width[str[i]-32];
		len++;
		i++;
	}
	return len;
}

/*
	int drawFont(Font * font, char * str, int * sx, int * sy)
	pre:
	post:
*/
int drawFont(SDL_Surface* surface, Font * font, const char * str, int * sx, int * sy)
{
	int x=*sx;
	int y=*sy;
	int i=0;
	while(i<strlen(str))
	{
		int width=font->width[str[i]-32];
		if(str[i]=='\n')
		{
			x=*sx;
			y+=font->point;
		}
		else
		{

                  SDL_Rect rect = {x,y,width,font->point};
                  SDL_BlitSurface(font->letter[str[i]-32], nullptr, surface, &rect);
                  x+=(width+1);
		}
		i++;
		if(x>=640)
		{
			x=*sx;
			y+=font->point;
		}
	}
	*sx=x;
	*sy=y;
	return 1;
}

/*
	int drawFont(Font * font, char * str, int sx, int sy)
	pre:
	post:
*/
int drawFont(SDL_Surface* surface, Font * font, const char * str, int sx, int sy)
{
	int x=sx;
	int y=sy;
	int i=0;
	while(i<strlen(str))
	{
		int width=font->width[str[i]-32];
		if(str[i]=='\n')
		{
			x=sx;
			y+=font->point;
		}
		else
		{
                  SDL_Rect rect = {x,y,width,font->point};
                  SDL_BlitSurface(font->letter[str[i]-32], nullptr, surface, &rect);
                  x+=(width+1);
		}
		i++;
		if(x>=640)
		{
			x=sx;
			y+=font->point;
		}
	}
	return x;
}

/*
	int drawFontBound
	Pre: none
	Post:
*/
int drawFontBound(SDL_Surface* surface, Font * font, const char * str, int sx, int sy,int ex, int ey)
{
	int x=sx;
	int y=sy;
	int i=0;
	while(i<strlen(str))
	{
		int width=font->width[str[i]-32];
                SDL_Rect rect = {x,y,width,font->point};
                SDL_BlitSurface(font->letter[str[i]-32], nullptr, surface, &rect);
                x+=(width+1);
		i++;
		if(x>=ex)
		{
			x=sx;
			y+=font->point;
			if(y>=ey)
				break;
		}
	}
	return x;
}

int releaseFont(Font * font)
{
	int i=0;
	while(i<96)
	{
		if(font->letter[i])
		{
                  SDL_FreeSurface(font->letter[i]);
                  font->letter[i]=NULL;
		}
		i++;
	}
	font->point=0;
	return 1;
}

int loadFont(Font * font,const char * str,int point)
{
	char fileName[255];
	FILE * f;
	int i=0;
	int x=0;
	int y=0;

	sprintf(fileName,"%s.bmp",str);

	SDL_Surface* temp = SDL_LoadBMP(fileName);
	
	sprintf(fileName,"%s.met",str);
	f = fopen(fileName,"r");
	
	font->point=point;

	while(i<96)
	{
		if(f==NULL)
		{
			font->width[i]=point;
		}
		else
			fscanf(f,"%d",&font->width[i]);

		font->letter[i] = SDL_CreateRGBSurface(0, font->width[i], point, 32, 0, 0, 0, 0);
                SDL_Rect src_rect = {x, y, font->width[i], point};
		SDL_BlitSurface(temp, &src_rect, font->letter[i], nullptr);
                SDL_SetColorKey(font->letter[i], 1, 0);
		x+=point;
		if(x>=12*point)
		{
			x=0;
			y+=point;
		}
		i++;
	}

	if(f!=NULL)
		fclose(f);
	SDL_FreeSurface(temp);
	return 1;
}
