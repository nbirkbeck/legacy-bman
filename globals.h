
#ifndef GLOBALS_H
#define GLOBALS_H

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define MAX_BOMBS_PLAYER 8
#define MAX_POWER_PLAYER 6
#define MAX_PLAYERS 4
#define TOTAL_BOMBS 32

#define XOFFSET 48
#define YOFFSET 32

#define FREE 1
#define OCCUPIED 0

#define BURN_THROUGH 1
#define BURN_STOP 2
#define BURN_STOP_BOMB 3
#define NO_BURN 4

#define BOMB_DEFAULT_TIMER 5000

#define DEBUG

#define CHAT_X 32
#define CHAT_Y 466

int resetGameState();
int addSinglePlayer(int num, int type);
int removePlayer(int num);
int drawConsole();
void gotoMenu();
void fade(int sx, int sy, int ex, int ey);
#endif