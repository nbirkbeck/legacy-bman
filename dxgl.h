#ifndef _DXGLH
#define _DXGLH
#include "compat.h"
#include <fcntl.h>
#include <iostream> // include important C/C++ stuff
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define SCREEN_WIDTH 640  // 96//640 // size of screen
#define SCREEN_HEIGHT 480 // 160//480
#define SCREEN_BPP 16     // bits per pixel

#define BITMAP_ID 0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE 256

int DDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, int x, int y, int width,
                       int height, LPDIRECTDRAWSURFACE7 dest, int transparent);
int DDraw_DrawSized_Surface(LPDIRECTDRAWSURFACE7 source, int x, int y,
                            int width, int height, int, int,
                            LPDIRECTDRAWSURFACE7 dest, int transparent);

int NDDraw_Draw_Surface(
    LPDIRECTDRAWSURFACE7 source, // source surface to draw
    int x, int y,                // position to take data from
    int width, int height,       // size of source surface
    LPDIRECTDRAWSURFACE7 dest,   // surface to draw the surface on
    int transparent);

// this builds a 16 bit color value in 5.5.5 format (1-bit alpha mode)
#define _RGB16BIT555(r, g, b) ((b % 32) + ((g % 32) << 5) + ((r % 32) << 10))

// this builds a 16 bit color value in 5.6.5 format (green dominate mode)
#define _RGB16BIT565(r, g, b) ((b % 32) + ((g % 64) << 6) + ((r % 32) << 11))

// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a, r, g, b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

#endif
