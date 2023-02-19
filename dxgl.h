#ifndef _DXGLH
#define _DXGLH
#include "compat.h"
#include <iostream> // include important C/C++ stuff
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h> 
#include <math.h>
#include <fcntl.h>

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define SCREEN_WIDTH    640//96//640 // size of screen
#define SCREEN_HEIGHT   480//160//480
#define SCREEN_BPP      16    // bits per pixel

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

typedef struct BITMAP_FILE_TAG
{
        BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info including the palette
        PALETTEENTRY     palette[256];      // we will store the palette here
        UCHAR            *buffer;           // this is a pointer to the data
} BITMAP_FILE, *BITMAP_FILE_PTR;


int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height);

int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename);

int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap);

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,int color);

int Scan_Image_Bitmap(BITMAP_FILE_PTR bitmap, LPDIRECTDRAWSURFACE7 lpdds, int cx,int cy);
int Scan_Image_Bitmap16(BITMAP_FILE_PTR bitmap, LPDIRECTDRAWSURFACE7 lpdds, int cx,int cy,int sizex, int sizey);
LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, int mem_flags, int color_key);

int DDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, int x, int y, 
                      int width, int height, LPDIRECTDRAWSURFACE7 dest, 
                      int transparent);
int DDraw_DrawSized_Surface(LPDIRECTDRAWSURFACE7 source, int x, int y, 
                      int width, int height, int,int,LPDIRECTDRAWSURFACE7 dest, 
                      int transparent);

int NDDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, // source surface to draw
                      int x, int y,                 // position to take data from
                      int width, int height,        // size of source surface
                      LPDIRECTDRAWSURFACE7 dest,    // surface to draw the surface on
                      int transparent ) ;

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list);

int Draw_Clip_Line(int x0,int y0, int x1, int y1,UCHAR color,UCHAR *dest_buffer, int lpitch);
int Clip_Line(int &x1,int &y1,int &x2, int &y2);
int Draw_Line(int xo, int yo, int x1,int y1, UCHAR color,UCHAR *vb_start,int lpitch);
int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds);


#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// this builds a 16 bit color value in 5.5.5 format (1-bit alpha mode)
#define _RGB16BIT555(r,g,b) ((b%32) + ((g%32) << 5) + ((r%32) << 10))

// this builds a 16 bit color value in 5.6.5 format (green dominate mode)
#define _RGB16BIT565(r,g,b) ((b%32) + ((g%64) << 6) + ((r%32) << 11))

// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

#endif
