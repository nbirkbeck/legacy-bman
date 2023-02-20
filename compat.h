#ifndef COMPAT_H
#define COMPAT_H
#include <SDL2/SDL_surface.h>
#include <stdint.h>

class DIRECTDRAWSURFACE7 {
public:
  void Release();
};

typedef DIRECTDRAWSURFACE7* LPDIRECTDRAWSURFACE7;

class DIRECTSOUND {
public:
};

typedef DIRECTSOUND* LPDIRECTSOUND;

class DIRECTSOUNDBUFFER {
public:
};

typedef DIRECTSOUNDBUFFER* LPDIRECTSOUNDBUFFER;

class DSBUFFERDESC {};

class IDirectMusicSegment {};
class IDirectMusicSegmentState {};
class IDirectMusicLoader {};
class IDirectMusicPerformance {};

class BITMAPFILEHEADER {};
class BITMAPINFOHEADER {};
class WAVEFORMATEX {};

struct PALETTEENTRY {};

struct LPRECT {};

typedef unsigned char UCHAR;

typedef int LPDIRECTDRAWCLIPPER;

typedef void* LPVOID;
typedef void* HRESULT;
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* DSCAPS;
typedef void* DSBCAPS;
typedef void* COLORREF;

typedef void* WIN32_FIND_DATA;

enum Fields {
  DDSCAPS_VIDEOMEMORY = 0,
  DSBCAPS_CTRLPAN = 3,
  DSBCAPS_CTRLFREQUENCY = 4,
  DSBCAPS_CTRLVOLUME = 5,
};

int32_t GetTickCount();
void Sleep(int32_t);

class POINT {
public:
  int32_t x;
  int32_t y;
};

typedef int32_t DWORD;
typedef uint16_t USHORT;

void DDraw_Draw_Surface(SDL_Surface* src, int x, int y, int w, int h,
                        SDL_Surface* dest, int transparent = 1);

void DDraw_DrawSized_Surface(SDL_Surface* src, int x, int y, int w, int h,
                             int sw, int sh, SDL_Surface* dest,
                             int transparent = 1);

#endif
