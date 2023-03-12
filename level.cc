#include "level.h"
#include "ai.h"
#include "compat.h"
#include "font.h"
#include "globals.h"
#include "list.h"
#include "player.h"

char curTextures[255];

extern SDL_Surface* surface;
extern Font font;
extern FILE* debug;

SDL_Surface* border[4];
SDL_Surface* levelTextures[NUM_LEVEL_TEXTURES];
SDL_Surface* powerUpTextures[NUM_POWERUP_TEXTURES];

Bomb bombs[TOTAL_BOMBS];
unsigned int grid[GRID_HEIGHT][GRID_WIDTH];

// This cannot be more than 32 !!!!
#define BRICK_BREAKING_NUM_FRAMES 21
int brickBreakingFrameOrder[BRICK_BREAKING_NUM_FRAMES] = {
    4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10};

List bombsInUse;

extern bool keys[256];
extern List playerList;

void reclaimAllBombs() {
  int i = 0;
  remAll(&bombsInUse);
  while (i < TOTAL_BOMBS) {
    bombs[i].state = BOMB_UNUSED;
    i++;
  }
}

Bomb* placeBomb(int playerNum, int i, int j) {
  int index = playerNum << 3;
  if (grid[i][j] & HAS_BOMB_MASK)
    return NULL;
#ifdef DEBUG
  fprintf(debug, "in placeBomb: (x:%d,y:%d)\n", (j << 5) + XOFFSET,
          (i << 5) + YOFFSET);
  fprintf(debug, "              (i:%d,j:%d)\n", i, j);
#endif
  while (index < TOTAL_BOMBS) {
    if (bombs[index].state == BOMB_UNUSED) {
      int temp;
      initBomb(&bombs[index], (XOFFSET) + (j << 5), (YOFFSET) + (i << 5),
               BOMB_NORMAL, 1, NULL);
      unmaskGridSquare(i, j);
      maskGridSquare(i, j, index);

      // insert(&bombsInUse,index);
      return &bombs[index];
    }
    index++;
  }
  return NULL;
}

int loadPowerUpTextures(const char* filename) {
  int i = 0, y = 0;
  SDL_Surface* temp = SDL_LoadBMP(filename);

  while (i < NUM_POWERUP_TEXTURES) {
    powerUpTextures[i] = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
    SDL_Rect src_rect = {0, y, 32, 32};
    SDL_BlitSurface(temp, &src_rect, powerUpTextures[i], nullptr);
    i++;
    y += 32;
  }

  SDL_FreeSurface(temp);
  return 1;
}

int createLevelTextures() {
  int i = 0;

  reclaimAllBombs();
  bombsInUse.head = NULL;

  border[LEFT] = SDL_CreateRGBSurface(0, 48, 480, 32, 0, 0, 0, 0);
  border[RIGHT] = SDL_CreateRGBSurface(0, 48, 480, 32, 0, 0, 0, 0);
  border[UP] = SDL_CreateRGBSurface(0, 544, 32, 32, 0, 0, 0, 0);
  border[DOWN] = SDL_CreateRGBSurface(0, 544, 32, 32, 0, 0, 0, 0);

  i = 0;
  while (i < NUM_LEVEL_TEXTURES) {
    levelTextures[i] = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
    SDL_SetColorKey(levelTextures[i], 1, 0);
    i++;
  }
  return 1;
}

int loadLevelTextures(const char* filename) {
  int i = 0, x = 0;

  sprintf(curTextures, "%s", filename);

  SDL_Surface* temp = SDL_LoadBMP(filename);

  reclaimAllBombs();
  bombsInUse.head = NULL;

  SDL_Rect left_rect = {0, 0, 48, 480};
  SDL_BlitSurface(temp, &left_rect, border[LEFT], 0);
  SDL_Rect right_rect = {592, 0, 48, 480};
  SDL_BlitSurface(temp, &right_rect, border[RIGHT], 0);
  SDL_Rect up_rect = {48, 0, 544, 32};
  SDL_BlitSurface(temp, &up_rect, border[UP], 0);
  SDL_Rect down_rect = {48, 448, 544, 32};
  SDL_BlitSurface(temp, &down_rect, border[DOWN], 0);

  i = 0;
  x = 48;
  while (i < NUM_LEVEL_TEXTURES) {
    SDL_Rect rect = {x, 32, 32, 32};
    SDL_BlitSurface(temp, &rect, levelTextures[i], 0);
    i++;
    x += 32;
  }

  SDL_FreeSurface(temp);
  return 1;
}

int createBasicLevel(int nbombs, int nflames, int nkick, int ndet, int nspeed) {
  int i, j;
  POINT possible[GRID_HEIGHT * GRID_WIDTH];
  int nPossible = 0;
  POINT* used;
  memset(grid, 0, sizeof(int) * GRID_HEIGHT * GRID_WIDTH);
  int floori[12] = {0,
                    0,
                    1,
                    0,
                    0,
                    1,
                    GRID_HEIGHT - 1,
                    GRID_HEIGHT - 1,
                    GRID_HEIGHT - 2,
                    GRID_HEIGHT - 1,
                    GRID_HEIGHT - 1,
                    GRID_HEIGHT - 2};
  int floorj[12] = {0,
                    1,
                    0,
                    GRID_WIDTH - 1,
                    GRID_WIDTH - 2,
                    GRID_WIDTH - 1,
                    GRID_WIDTH - 1,
                    GRID_WIDTH - 2,
                    GRID_WIDTH - 1,
                    0,
                    1,
                    0};
  i = 0;
  while (i < GRID_HEIGHT) {
    j = 0;
    grid[i][j] = 0;
    while (j < GRID_WIDTH) {
      grid[i][j] |= FLOOR;
      if (i % 2 == 1 && j % 2 == 1) {
        grid[i][j] |= INDY_BRICK;
      } else if (rand() % 2 == 0) {
        grid[i][j] |= FLOOR;
      } else {
        possible[nPossible].x = j;
        possible[nPossible++].y = i;
        grid[i][j] |= BRICK;
      }
      j++;
    }
    i++;
  }

  // used = (POINT *) malloc(sizeof(POINT)*(total));

  i = 0;
  while (i < nbombs) {
    int r = rand() % nPossible;
    grid[possible[r].y][possible[r].x] |= BOMB;
    i++;
  }

  i = 0;
  while (i < nflames) {
    int r = rand() % nPossible;
    grid[possible[r].y][possible[r].x] |= FLAME;
    i++;
  }

  i = 0;
  while (i < nspeed) {
    int r = rand() % nPossible;
    grid[possible[r].y][possible[r].x] |= SPEED;
    i++;
  }

  i = 0;
  while (i < nkick) {
    int r = rand() % nPossible;
    grid[possible[r].y][possible[r].x] |= KICK;
    i++;
  }

  i = 0;
  while (i < ndet) {
    int r = rand() % nPossible;
    grid[possible[r].y][possible[r].x] |= DETONATOR;
    i++;
  }

  i = 0;
  while (i < 12) {
    grid[floori[i]][floorj[i]] = FLOOR;
    i++;
  }
  return 1;
}

int loadLevel(const char* filename) {
  int i, j;
  char mapName[255];
  sprintf(mapName, "%s/map.bm", filename);
  FILE* file = fopen(mapName, "r");
  //	srand(GetTickCount()%777);
  //	createBasicLevel(3,3,2,2,3);
  //	if(1) return 1;

  if (!file) {
    return 0;
  }

  i = 0;
  while (i < GRID_HEIGHT) {
    j = 0;

    while (j < GRID_WIDTH) {
      int input;
      fscanf(file, "%d", &input);
      if (!(input & 0x3))
        input |= 1;
      grid[i][j] = input;
      j++;
    }
    i++;
  }
  fclose(file);
  sprintf(mapName, "%s/back.bmp", filename);
  if ((file = fopen(mapName, "r")) != NULL) {
    fclose(file);
    loadLevelTextures(mapName);
  } else {
    loadLevelTextures("./data/level/back.bmp");
  }
  return 1;
}

int clearScores() { return 1; }

int drawScoreBoard() {
  int x = 592;
  int y = 32;
  Node* finger = playerList.head;

  while (finger != NULL) {
    char score[10];
    Player* player = (Player*)finger->value;

    drawScoreBoardPlayer(player, x, y);
    sprintf(score, "%d %d %d", player->nkills, player->ndeaths,
            player->nsuicides);
    drawFont(surface, &font, score, x, y + 32);
    y += 64;
    finger = finger->next;
  }
  return 1;
}

int drawLevelTexture(int tNum, int x, int y) {
  SDL_Rect rect = {x, y, 32, 32};
  SDL_BlitSurface(levelTextures[tNum], &rect, surface, nullptr);
  return 1;
}

int drawLevelSmall(SDL_Surface* surface, int x, int y) {
  int i = 0;
  int j = 0;
  while (i < GRID_HEIGHT) {
    j = 0;

    while (j < GRID_WIDTH) {
      if ((grid[i][j] & FLOOR) == FLOOR) {
        DDraw_DrawSized_Surface(levelTextures[FLOOR_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      } else if ((grid[i][j] & ALT_FLOOR) == ALT_FLOOR) {
        DDraw_DrawSized_Surface(levelTextures[ALT_FLOOR_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      }
      if ((grid[i][j] & INDY_BRICK) == INDY_BRICK) {
        DDraw_DrawSized_Surface(levelTextures[INDY_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      } else if ((grid[i][j] & BRICK) == BRICK) {
        if (grid[i][j] & BRICK_BREAKING_MASK) {
          grid[i][j] += 0x08000000;
          if ((grid[i][j] >> 27) >= BRICK_BREAKING_NUM_FRAMES) {
            grid[i][j] &= 0x03fffff3;
          }
          DDraw_DrawSized_Surface(
              levelTextures[brickBreakingFrameOrder[grid[i][j] >> 27]],
              x + (j << 4), y + (i << 4), 32, 32, 16, 16, surface);
        } else
          DDraw_DrawSized_Surface(levelTextures[BRICK_TEX + (grid[i][j] >> 27)],
                                  x + (j << 4), y + (i << 4), 32, 32, 16, 16,
                                  surface);
      } else if (grid[i][j] & BOMB) {
        DDraw_DrawSized_Surface(powerUpTextures[BOMB_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      } else if (grid[i][j] & FLAME) {
        DDraw_DrawSized_Surface(powerUpTextures[FLAME_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      } else if (grid[i][j] & KICK) {
        DDraw_DrawSized_Surface(powerUpTextures[KICK_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      } else if (grid[i][j] & SPEED) {
        DDraw_DrawSized_Surface(powerUpTextures[SPEED_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      } else if (grid[i][j] & DETONATOR) {
        DDraw_DrawSized_Surface(powerUpTextures[DETONATOR_TEX], x + (j << 4),
                                y + (i << 4), 32, 32, 16, 16, surface);
      }
      j++;
    }
    i++;
  }
  return 1;
}

/*
        The level is drawn
*/
int drawLevel() {
  static int timer = 0;
  int i = 0;
  int j = 0;

  DDraw_Draw_Surface(border[LEFT], 0, 0, 48, 480, surface);
  DDraw_Draw_Surface(border[UP], 48, 0, 544, 32, surface);
  DDraw_Draw_Surface(border[RIGHT], 592, 0, 48, 480, surface);
  DDraw_Draw_Surface(border[DOWN], 48, 448, 544, 32, surface);

  drawScoreBoard();

  while (i < GRID_HEIGHT) {
    j = 0;

    while (j < GRID_WIDTH) {
      if ((grid[i][j] & FLOOR) == FLOOR) {
        DDraw_Draw_Surface(levelTextures[FLOOR_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      } else if ((grid[i][j] & ALT_FLOOR) == ALT_FLOOR) {
        DDraw_Draw_Surface(levelTextures[ALT_FLOOR_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      }
      if ((grid[i][j] & INDY_BRICK) == INDY_BRICK) {
        DDraw_Draw_Surface(levelTextures[INDY_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      } else if ((grid[i][j] & BRICK) == BRICK) {
        if (grid[i][j] & BRICK_BREAKING_MASK) {
          grid[i][j] += 0x08000000;
          if ((grid[i][j] >> 27) >= BRICK_BREAKING_NUM_FRAMES) {
            grid[i][j] &= 0x03fffff3;
          }
          DDraw_Draw_Surface(
              levelTextures[brickBreakingFrameOrder[grid[i][j] >> 27]],
              XOFFSET + (j << 5), YOFFSET + (i << 5), 32, 32, surface);
        } else
          DDraw_Draw_Surface(levelTextures[BRICK_TEX + (grid[i][j] >> 27)],
                             XOFFSET + (j << 5), YOFFSET + (i << 5), 32, 32,
                             surface);
      } else if (grid[i][j] & BOMB) {
        DDraw_Draw_Surface(powerUpTextures[BOMB_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      } else if (grid[i][j] & FLAME) {
        DDraw_Draw_Surface(powerUpTextures[FLAME_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      } else if (grid[i][j] & KICK) {
        DDraw_Draw_Surface(powerUpTextures[KICK_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      } else if (grid[i][j] & SPEED) {
        DDraw_Draw_Surface(powerUpTextures[SPEED_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      } else if (grid[i][j] & DETONATOR) {
        DDraw_Draw_Surface(powerUpTextures[DETONATOR_TEX], XOFFSET + (j << 5),
                           YOFFSET + (i << 5), 32, 32, surface);
      }
      j++;
    }
    i++;
  }
  i = 0;

  while (i < TOTAL_BOMBS) {
    drawBomb(&bombs[i]);
    /*
            May not want to call move Bomb here
            since the game may be paused
    */
    moveBomb(&bombs[i]);
    i++;
  }

#ifdef DEBUG
  i = 0;
  while (i < GRID_HEIGHT) {
    j = 0;
    while (j < GRID_WIDTH) {
      if (grid[i][j] & HAS_BOMB_MASK) {
        char string[25];
        sprintf(string, "%d", grid[i][j] >> 27);
        drawFont(surface, &font, string, (j << 5) + XOFFSET + 10,
                 (i << 5) + YOFFSET + 10);
      }
      j++;
    }
    i++;
  }
#endif
  return 1;
}

int pickupPowerUp(int i, int j) {
  grid[i][j] &= 0x0ffff00f;
  return 1;
}

int maskGridSquare(int i, int j, int bombIndex) {
  int temp;
  grid[i][j] |= HAS_BOMB_MASK;
  temp = bombIndex << 27;
  grid[i][j] |= temp;
  return 1;
}

int unmaskGridSquare(int i, int j) {
  int oldIndex = (grid[i][j] >> 27);
  grid[i][j] &= 0x03ffffff;
  return oldIndex;
}

int checkGrid(int x, int y) {
  int i, j;

  if (x < 48 || y < 32)
    return OCCUPIED;

  i = (y - YOFFSET) >> 5;
  j = (x - XOFFSET) >> 5;

  if (j >= GRID_WIDTH)
    return OCCUPIED;
  if (i >= GRID_HEIGHT)
    return OCCUPIED;

  if (grid[i][j] & 0xc) {
    return OCCUPIED;
  }
  if (grid[i][j] & HAS_BOMB_MASK)
    return OCCUPIED;

  return grid[i][j] & 0xfff;
}

int checkForPlayer(int x, int y) {
  int i, j;

  i = (y - YOFFSET) >> 5;
  j = (x - XOFFSET) >> 5;

  Node* finger = playerList.head;
  while (finger != NULL) {
    Player* player = (Player*)finger->value;
    if ((player->x - XOFFSET) >> 5 == j) {
      if ((player->y - YOFFSET) >> 5 == i)
        return 1;
    }
    finger = finger->next;
  }
  return 0;
}

int levelKillPlayer(Bomb* bomb, int i, int j) {
  Node* finger = playerList.head;
  while (finger != NULL) {
    Player* player = (Player*)finger->value;
    if ((player->x + 16 - XOFFSET) >> 5 == j &&
        (player->y + 16 - YOFFSET) >> 5 == i)
      killPlayer(player, bomb);
    finger = finger->next;
  }
  return BURN_THROUGH;
}
/*
        returns NO_BURN if the flame is not supposed to go through this.
        returns BURN_THROUGH if the flame is supposed to continue burning after
        returns BURN_STOP if the flame is suppsed to burn this square and stop
   after
*/
int levelKill(Bomb* bomb, int i, int j) {
  int ret = BURN_THROUGH;

#ifdef DEBUG
  char labels[][25] = {"", "Burn Through", "Burn Stop", "Burn Stop Bomb",
                       "No Burn"};
#endif

  if (i < 0 || j < 0 || i >= GRID_HEIGHT || j >= GRID_WIDTH) {
    ret = NO_BURN;
    return ret;
  } else if ((grid[i][j] & INDY_BRICK) == INDY_BRICK) {
    ret = NO_BURN;
    return ret;
  } else if ((grid[i][j] & BRICK) == BRICK) {
    /* burn the brick*/
    grid[i][j] |= BRICK_BREAKING_MASK;
    ret = BURN_STOP;
    return ret;
  }

  levelKillPlayer(bomb, i, j);

  if ((grid[i][j] & HAS_BOMB_MASK) == HAS_BOMB_MASK) {
    /*
            explode the other bomb now
    */
    int index = grid[i][j] >> 27;
#ifdef DEBUG
    fprintf(debug, "levelKill is exploding Bomb %d\n", index);
#endif
    if (bombs[index].state != BOMB_EXPLODING) {
      explodeBomb(&bombs[index]);
    }
    ret = BURN_STOP_BOMB;
  }

#ifdef DEBUG
  fprintf(debug, "levelKill(%d,%d) --> %s\n", i, j, labels[ret]);
#endif
  return ret;
}

int levelKick(int i, int j, int dir) {
  if (grid[i][j] & HAS_BOMB_MASK) {
    int bi = grid[i][j] >> 27;
    if (bombs[bi].state != BOMB_EXPLODING) {
      bombs[bi].moving = 1;
      bombs[bi].dir = dir;
    }
    return 1;
  }
  return 0;
}
int releasePowerUpTextures() {
  int i = 0;
  while (i < NUM_POWERUP_TEXTURES) {
    SDL_FreeSurface(powerUpTextures[i]);
    powerUpTextures[i] = NULL;
    i++;
  }
  return 1;
}

int releaseLevelTextures() {
  int i = 0;
  while (i < 4) {
    if (border[i]) {
      SDL_FreeSurface(border[i]);
      border[i] = NULL;
    }
    i++;
  }
  i = 0;
  while (i < NUM_LEVEL_TEXTURES) {
    SDL_FreeSurface(levelTextures[i]);
    levelTextures[i] = NULL;
    i++;
  }
  return 1;
}
