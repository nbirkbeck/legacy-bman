#include <SDL2/SDL_surface.h>

#ifndef FONT_H
#define FONT_H

typedef struct FONT_TYPE {
  SDL_Surface* letter[96];
  int width[96];
  int point;
} Font;

typedef struct MESSAGE_TYPE {
  char* message;
  int x, y, time;
  Font* font;
} Message;

int initMessages();
int messagesShutdown();
int drawMessages(SDL_Surface* surface);
int removeMessage(char* message);
int addMessage(char* message, Font* font, int x, int y, int time);

int getLength(Font* font, const char* str);
int releaseFont(Font* font);
int loadFont(Font* font, const char* str, int point);
int drawFont(SDL_Surface*, Font* font, const char* str, int, int);
int drawFont(SDL_Surface*, Font* font, const char* str, int*, int*);
int drawFontBound(SDL_Surface*, Font* font, const char* str, int, int, int,
                  int);

#endif
