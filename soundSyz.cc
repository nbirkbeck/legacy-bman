#include "soundSyz.h"
#include "dxgl.h"
#include "font.h"

extern LPDIRECTDRAWSURFACE7 lpddsback;
extern BITMAP_FILE bitFile;
extern Font font;

#define BACK 0
#define PLAY 1
#define PAUSE 2
#define STOP 3
#define FORWARD 4
#define ADD 5
#define REMOVE 6
#define NEW 7
#define CLOSE 8
#define OPEN 9
#define SCROLL_TOP 10
#define SCROLL_MIDDLE 11
#define SCROLL_BOTTOM 12
#define SCROLL_SLIDER 13

#define VOLUME_CONTROL 14
#define VOL_BACK 0
#define VOL_SLIDER 1

#define PLAYLIST_SELECTION 15

#define S_HIDDEN 0
#define S_OPENING 1
#define S_OPEN 2
#define S_CLOSING 3
#define S_ADDING 4

#define HIDDENX -75
#define OPENX 0

int sound_x, sound_y;
int musicVolume;
int state;

// char str[255];
char addBuffer[255];
int addBufferLen = 0;
char* playlistEntryPaths[MAX_PLAYLIST_ENTRIES];
char* playlistEntryNames[MAX_PLAYLIST_ENTRIES];
bool playlistSelected[MAX_PLAYLIST_ENTRIES];
int numPlaylistEntries = 0;
int currentFile = 0;
int topPlaylist = 0;
LPDIRECTDRAWSURFACE7 buttons[14][2];
LPDIRECTDRAWSURFACE7 volume[2];
LPDIRECTDRAWSURFACE7 playlistBack;

Font* soundFont;
Font* soundFontSel;
int selectedButton = -1;
bool controlDown = 0;
bool shiftDown = 0;

int drawSoundSyzSOPEN();
int drawSoundSyzSHIDDEN();
int drawSoundSyzSADDING();
void playSoundSyz();
void clearPlaylist();
void clearSelectedPlaylist();
void nextSoundSyz();

#define MUSIC_CHANNEL 0
#define SOUND_CHANNEL 1

FSOUND_STREAM* mainStream;

FSOUND_SAMPLE* bombSound;
FSOUND_SAMPLE* kickSound;
FSOUND_SAMPLE* menuItemSound;
FSOUND_SAMPLE* menuSelectedSound;

int playKickSound() {
  FSOUND_PlaySound(SOUND_CHANNEL, kickSound);
  return 1;
}
int playPlayerSound(int p) { return 1; }
int playMenuItemSound() {
  FSOUND_PlaySound(SOUND_CHANNEL, menuItemSound);
  return 1;
}
int playMenuSelectSound() {
  FSOUND_PlaySound(SOUND_CHANNEL, menuSelectedSound);
  return 1;
}
int playBombSound() {
  FSOUND_PlaySound(SOUND_CHANNEL, bombSound);
  return 1;
}

void openSoundSyz() { state = S_OPENING; }
void closeSoundSyz() { state = S_CLOSING; }

void playPlaylistEntry(int index) {
  if (index < 0)
    return;
  if (index >= numPlaylistEntries)
    return;
  currentFile = index;
  playSoundSyz();
}
signed char endCallBack(FSOUND_STREAM* stream, void* buffer, int len,
                        int param) {
  if (mainStream != NULL) {
    if (FSOUND_Stream_GetPosition(mainStream) >=
        FSOUND_Stream_GetLength(mainStream))
      nextSoundSyz();
  }
  return (signed char)1;
}
void playSoundSyz() {
  if (mainStream != NULL) {
    FSOUND_Stream_Close(mainStream);
  }
  char name[512];
  sprintf(name, "%s/%s", playlistEntryPaths[currentFile],
          playlistEntryNames[currentFile]);
  mainStream = FSOUND_Stream_OpenFile(name, 0, 0);
  if (mainStream != NULL) {

    FSOUND_Stream_Play(MUSIC_CHANNEL, mainStream);
    //	FSOUND_Stream_SetEndCallback(mainStream,endCallBack,0);
  }
}
void nextSoundSyz() {
  currentFile++;
  if (currentFile >= numPlaylistEntries)
    currentFile = 0;
  playSoundSyz();
}
void backSoundSyz() {
  currentFile--;
  if (currentFile < 0)
    currentFile = numPlaylistEntries - 1;
  playSoundSyz();
}
void pauseSoundSyz() {
  FSOUND_SetPaused(MUSIC_CHANNEL, !FSOUND_GetPaused(MUSIC_CHANNEL));
}
void stopSoundSyz() { FSOUND_Stream_Stop(mainStream); }
void setMusicVolume() { FSOUND_SetVolume(MUSIC_CHANNEL, musicVolume); }

void addSoundSyz() { state = S_ADDING; }

void removeSoundSyz() {
  int i = 0;
  int j = 0;
  int decrement = 0;
  while (i < numPlaylistEntries) {
    if (playlistSelected[i] && i != currentFile) {
      if (playlistEntryPaths[i]) {
        free(playlistEntryPaths[i]);
        playlistEntryPaths[i] = NULL;
      }
      if (playlistEntryNames[i]) {
        free(playlistEntryNames[i]);
        playlistEntryNames[i] = NULL;
      }
      if (i < currentFile) {
        decrement++;
      }
    }

    i++;
  }
  currentFile -= decrement;
  if (currentFile < 0)
    currentFile = 0;
  int nume = 0;
  i = 0;

  while (i < numPlaylistEntries) {
    if (playlistEntryNames[i] == NULL) {
      j = i;
      while (j < numPlaylistEntries && playlistEntryNames[j] == NULL) {
        j++;
      }
      if (j < numPlaylistEntries) {
        playlistEntryNames[i] = playlistEntryNames[j];
        playlistEntryPaths[i] = playlistEntryPaths[j];
        playlistEntryNames[j] = NULL;
        playlistEntryPaths[j] = NULL;
        nume++;
      } else
        break;
    } else
      nume++;
    i++;
  }
  numPlaylistEntries = nume;
  clearSelectedPlaylist();
}

void (*actions[10])(void) = {
    backSoundSyz, playSoundSyz,   pauseSoundSyz, stopSoundSyz,  nextSoundSyz,
    addSoundSyz,  removeSoundSyz, clearPlaylist, closeSoundSyz, openSoundSyz};
void setMusicVolume(int volume) {}

void clearSelectedPlaylist() {
  memset(playlistSelected, 0, MAX_PLAYLIST_ENTRIES * sizeof(bool));
}

void clearPlaylist() {
  int i = 0;

  while (i < MAX_PLAYLIST_ENTRIES) {
    if (playlistEntryNames[i]) {
      free(playlistEntryNames[i]);
      playlistEntryNames[i] = NULL;
    }
    if (playlistEntryPaths[i]) {
      free(playlistEntryPaths[i]);
      playlistEntryPaths[i] = NULL;
    }
    i++;
  }
  topPlaylist = 0;
  clearSelectedPlaylist();
  numPlaylistEntries = 0;
}

int loadDirectory(char* dir) {
  HANDLE handle;
  WIN32_FIND_DATA ffd;
  int i = 0;
  int dirlen = strlen(dir) + 1;
  char where[255];
  sprintf(where, "%s/*.mp3", dir);

  handle = FindFirstFile(where, &ffd);

  clearPlaylist();

  while (handle != INVALID_HANDLE_VALUE && i < MAX_PLAYLIST_ENTRIES) {
    char* name = (char*)malloc(strlen(ffd.cFileName) + 1);
    char* path = (char*)malloc(dirlen);

    sprintf(name, "%s", ffd.cFileName);
    sprintf(path, "%s", dir);
    playlistEntryNames[i] = name;
    playlistEntryPaths[i] = path;

    i++;
    if (!FindNextFile(handle, &ffd))
      break;
  }
  numPlaylistEntries = i;
  FindClose(handle);
  return 1;
}

int loadSounds() {
  bombSound = FSOUND_Sample_Load(FSOUND_FREE, "./data/sounds/Exp1.wav",
                                 FSOUND_NORMAL, 0);
  kickSound = FSOUND_Sample_Load(FSOUND_FREE, "./data/sounds/kick.wav",
                                 FSOUND_NORMAL, 0);
  menuItemSound = FSOUND_Sample_Load(FSOUND_FREE, "./data/sounds/menuitem.wav",
                                     FSOUND_NORMAL, 0);
  menuSelectedSound = FSOUND_Sample_Load(
      FSOUND_FREE, "./data/sounds/menuselect.wav", FSOUND_NORMAL, 0);

  return 1;
}

int freeSounds() {
  FSOUND_Sample_Free(bombSound);
  FSOUND_Sample_Free(kickSound);
  FSOUND_Sample_Free(menuItemSound);
  FSOUND_Sample_Free(menuSelectedSound);
  return 1;
}

int initSoundSyz() {
  int i, x, y;
  LPDIRECTDRAWSURFACE7 temp;
  sound_x = HIDDENX;
  sound_y = 10;
  musicVolume = 255;

  state = S_HIDDEN;

  Load_Bitmap_File(&bitFile, "./data/controls.bmp");
  temp = DDraw_Create_Surface(80, 160, DDSCAPS_VIDEOMEMORY, 0);
  Scan_Image_Bitmap16(&bitFile, temp, 0, 0, 80, 160);
  Unload_Bitmap_File(&bitFile);
  i = 0;
  x = 0;
  while (i < 5) {
    buttons[i][0] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
    NDDraw_Draw_Surface(temp, x, 0, 16, 16, buttons[i][0], 0);
    i++;
    x += 16;
  }
  i = 0;
  x = 0;
  while (i < 5) {
    buttons[i][1] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
    NDDraw_Draw_Surface(temp, x, 16, 16, 16, buttons[i][1], 0);
    i++;
    x += 16;
  }

  i = 5;
  y = 32;
  x = 0;
  while (i < 8) {
    buttons[i][0] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
    NDDraw_Draw_Surface(temp, x, y, 16, 16, buttons[i][0], 0);
    x += 16;
    i++;
  }

  y = 48;
  x = 0;
  i = 5;
  while (i < 8) {
    buttons[i][1] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
    NDDraw_Draw_Surface(temp, x, y, 16, 16, buttons[i][1], 0);
    x += 16;
    i++;
  }

  buttons[CLOSE][0] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[CLOSE][1] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);

  NDDraw_Draw_Surface(temp, 0, 96, 16, 16, buttons[CLOSE][0], 0);
  NDDraw_Draw_Surface(temp, 16, 96, 16, 16, buttons[CLOSE][1], 0);

  buttons[OPEN][0] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[OPEN][1] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);

  NDDraw_Draw_Surface(temp, 32, 96, 16, 16, buttons[OPEN][0], 0);
  NDDraw_Draw_Surface(temp, 48, 96, 16, 16, buttons[OPEN][1], 0);

  volume[VOL_BACK] = DDraw_Create_Surface(80, 16, DDSCAPS_VIDEOMEMORY, 0);
  volume[VOL_SLIDER] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);

  NDDraw_Draw_Surface(temp, 0, 64, 80, 16, volume[VOL_BACK], 0);
  NDDraw_Draw_Surface(temp, 0, 80, 16, 16, volume[VOL_SLIDER], 0);

  buttons[SCROLL_TOP][0] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[SCROLL_TOP][1] = DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[SCROLL_BOTTOM][0] =
      DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[SCROLL_BOTTOM][1] =
      DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[SCROLL_MIDDLE][0] =
      DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[SCROLL_MIDDLE][1] = NULL;
  buttons[SCROLL_SLIDER][0] =
      DDraw_Create_Surface(16, 16, DDSCAPS_VIDEOMEMORY, 0);
  buttons[SCROLL_SLIDER][1] = NULL;

  NDDraw_Draw_Surface(temp, 0, 112, 16, 16, buttons[SCROLL_TOP][0], 0);
  NDDraw_Draw_Surface(temp, 16, 112, 16, 16, buttons[SCROLL_TOP][1], 0);
  NDDraw_Draw_Surface(temp, 32, 112, 16, 16, buttons[SCROLL_BOTTOM][0], 0);
  NDDraw_Draw_Surface(temp, 48, 112, 16, 16, buttons[SCROLL_BOTTOM][1], 0);
  NDDraw_Draw_Surface(temp, 64, 112, 16, 16, buttons[SCROLL_MIDDLE][0], 0);
  NDDraw_Draw_Surface(temp, 0, 128, 16, 16, buttons[SCROLL_SLIDER][0], 0);

  playlistBack = DDraw_Create_Surface(80, 8, DDSCAPS_VIDEOMEMORY, 0);
  int color = 0xf;
  DDraw_Fill_Surface(playlistBack, color);
  temp->Release();

  soundFont = (Font*)malloc(sizeof(Font));
  soundFontSel = (Font*)malloc(sizeof(Font));
  loadFont(soundFont, "./fonts/font_small", 8);
  loadFont(soundFontSel, "./fonts/font_small_selected", 8);

  FSOUND_Init(44100, 32, 0);
  mainStream = NULL;

  currentFile = 0;
  numPlaylistEntries = 0;
  memset(playlistEntryNames, 0, sizeof(char*) * MAX_PLAYLIST_ENTRIES);
  memset(playlistEntryPaths, 0, sizeof(char*) * MAX_PLAYLIST_ENTRIES);
  clearSelectedPlaylist();
  loadDirectory("d:/my music");

  loadSounds();

  return 1;
}

int drawSoundSyz() {
  switch (state) {
  case S_HIDDEN:
    drawSoundSyzSHIDDEN();
    break;
  case S_CLOSING:
    sound_x -= 4;
    if (sound_x <= HIDDENX) {
      sound_x = HIDDENX;
      state = S_HIDDEN;
      break;
    }
    drawSoundSyzSOPEN();
    break;
  case S_OPENING:
    sound_x += 4;
    if (sound_x >= OPENX) {
      sound_x = OPENX;
      state = S_OPEN;
      break;
    }
    drawSoundSyzSOPEN();
    break;
  case S_OPEN:
    drawSoundSyzSOPEN();
    break;
  case S_ADDING:
    drawSoundSyzSOPEN();
    drawSoundSyzSADDING();
    break;
  }
  return 1;
}

inline int drawSoundSyzSHIDDEN() {

  DDraw_Draw_Surface(buttons[OPEN][selectedButton == OPEN], OPENX, sound_y + 16,
                     16, 16, lpddsback, 1);
  DDraw_Draw_Surface(buttons[CLOSE][selectedButton == CLOSE], OPENX,
                     sound_y + 32, 16, 16, lpddsback, 1);

  return 1;
}
int drawPlaylist(int y) {
  int i = topPlaylist;
  int count = 0;
  int x = 80;
  int yback = y;

  if (mainStream != NULL) {
    if (FSOUND_Stream_GetPosition(mainStream) >=
        FSOUND_Stream_GetLength(mainStream))
      nextSoundSyz();
  }

  while (y <= SCREEN_HEIGHT && i < numPlaylistEntries && count < 12) {
    char string[260];
    sprintf(string, "%d.%s", i, playlistEntryNames[i]);

    if (playlistSelected[i])
      DDraw_Draw_Surface(playlistBack, 0, y, 80, 8, lpddsback, 1);
    if (i == currentFile)
      drawFontBound(soundFontSel, string, 0, y, x, y);
    else
      drawFontBound(soundFont, string, 0, y, x, y);
    y += (soundFont->point);
    i++;
    count++;
  }
  y = yback;
  DDraw_Draw_Surface(buttons[SCROLL_TOP][selectedButton == SCROLL_TOP], x, y,
                     16, 16, lpddsback, 1);
  DDraw_Draw_Surface(buttons[SCROLL_MIDDLE][0], x, y + 16, 16, 16, lpddsback,
                     1);
  DDraw_Draw_Surface(buttons[SCROLL_MIDDLE][0], x, y + 32, 16, 16, lpddsback,
                     1);
  DDraw_Draw_Surface(buttons[SCROLL_MIDDLE][0], x, y + 48, 16, 16, lpddsback,
                     1);
  DDraw_Draw_Surface(buttons[SCROLL_MIDDLE][0], x, y + 64, 16, 16, lpddsback,
                     1);
  DDraw_Draw_Surface(buttons[SCROLL_BOTTOM][selectedButton == SCROLL_BOTTOM], x,
                     y + 80, 16, 16, lpddsback, 1);
  int slidery = y + 8;
  if (numPlaylistEntries != 0)
    slidery += (topPlaylist * 72 / numPlaylistEntries);
  DDraw_Draw_Surface(buttons[SCROLL_SLIDER][0], x, slidery, 16, 16, lpddsback,
                     1);

  return 1;
}
int drawSoundSyzSOPEN() {
  int x, y;
  int i = 0;
  x = sound_x;
  y = sound_y;

  char title[255];
  if (playlistEntryNames[currentFile] != NULL) {
    int time = FSOUND_Stream_GetTime(mainStream);
    int min, sec;
    static float offset = 0.0f;
    min = time / 60000;
    sec = (time / 1000) - (60 * min);
    if (offset > strlen(playlistEntryNames[currentFile]))
      offset = 0;
    sprintf(title, "%s (%2d:%02d) %s",
            playlistEntryNames[currentFile] + ((int)offset), min, sec,
            playlistEntryNames[currentFile]);
    drawFontBound(soundFont, title, x, sound_y - 10, 96, sound_y - 10);
    offset += 0.1f;
  }
  while (i < 5) {
    DDraw_Draw_Surface(buttons[i][selectedButton == i], x, y, 16, 16, lpddsback,
                       1);
    i++;
    x += 16;
  }

  i = 0;
  y += 16;
  x = sound_x;
  DDraw_Draw_Surface(volume[VOL_BACK], sound_x, y, 80, 16, lpddsback, 1);
  DDraw_Draw_Surface(volume[VOL_SLIDER], (musicVolume << 6) / 255 + sound_x, y,
                     16, 16, lpddsback, 1);
  i = 5;
  x = sound_x;
  y += 16;
  while (i < 8) {
    DDraw_Draw_Surface(buttons[i][selectedButton == i], x, y, 16, 16, lpddsback,
                       1);
    x += 16;
    i++;
  }
  DDraw_Draw_Surface(buttons[OPEN][selectedButton == OPEN], sound_x + 80,
                     sound_y + 16, 16, 16, lpddsback, 1);
  DDraw_Draw_Surface(buttons[CLOSE][selectedButton == CLOSE], sound_x + 80,
                     sound_y + 32, 16, 16, lpddsback, 1);

  if (state == S_OPEN)
    drawPlaylist(y + 16);
  return 1;
}

int drawSoundSyzSADDING() {
  DDraw_Draw_Surface(playlistBack, sound_x, sound_y + 48, 80, 8, lpddsback, 1);
  drawFont(soundFont, addBuffer, sound_x, sound_y + 48);
  return 1;
}

inline int sendMessageSoundSyzSOPEN(int message, int wparam, int lparam);
inline int sendMessageSoundSyzSHIDDEN(int message, int wparam, int lparam);
inline int sendMessageSoundSyzSADDING(int message, int wparam, int lparam);

int sendMessageSoundSyz(int message, int wparam, int lparam) {
  switch (state) {
  case S_ADDING:
    sendMessageSoundSyzSADDING(message, wparam, lparam);
    break;
  case S_OPEN:
    sendMessageSoundSyzSOPEN(message, wparam, lparam);
    break;
  case S_HIDDEN:
    sendMessageSoundSyzSHIDDEN(message, wparam, lparam);
    break;
  case S_CLOSING:
    break;
  case S_OPENING:
    break;
  }
  return 1;
}

inline int sendMessageSoundSyzSADDING(int message, int wparam, int lparam) {
  switch (message) {
  case WM_KEYDOWN:
    return 1;
  case WM_KEYUP:
    return 1;
  case WM_CHAR:
    if (wparam == VK_BACK) {
      addBufferLen--;
      if (addBufferLen < 0)
        addBufferLen = 0;
    } else if (wparam == VK_RETURN) {
      // FILE * file = fopen(addBuffer,"r");
      // if(file!=NULL)
      loadDirectory(addBuffer);
      addBufferLen = 0;
      state = S_OPEN;

    } else
      addBuffer[addBufferLen++] = wparam;
    addBuffer[addBufferLen] = '\0';

    return 1;
  }
  return 0;
}

inline int sendMessageSoundSyzSHIDDEN(int message, int wparam, int lparam) {
  int mx = (int)LOWORD(lparam);
  int my = (int)HIWORD(lparam);

  if (mx >= (OPENX + 16))
    return 0;
  if (my >= (sound_y + 32) || my < (sound_y + 16))
    return 0;
  else {
    if (message == WM_LBUTTONDOWN) {
      selectedButton = OPEN;
    } else if (message == WM_LBUTTONUP) {
      if (selectedButton == OPEN) {
        actions[OPEN]();
      }
      selectedButton = -1;
    }
    // else selectedButton=-1;
  }
  return 1;
}

inline int sendMessageSoundSyzSOPEN(int message, int wparam, int lparam) {
  int mx = (int)LOWORD(lparam);
  int my = (int)HIWORD(lparam);
  int i, j;

  if (message == WM_MOUSEMOVE || message == WM_LBUTTONDOWN ||
      message == WM_LBUTTONUP || message == WM_LBUTTONDBLCLK) {

    if (mx < sound_x || my < sound_y) {
      // if(selectedButton!=VOLUME_CONTROL)
      selectedButton = -1;
      return 0;
    }
    if (mx >= (sound_x + 112) || my >= (sound_y + 160)) {
      // if(selectedButton!=VOLUME_CONTROL)
      selectedButton = -1;
      return 0;
    }
    j = (mx - sound_x) >> 4;
    i = (my - sound_y) >> 4;
  }
  switch (message) {

  case WM_LBUTTONDOWN:

    selectedButton = -1;
    if (i == 0) {
      if (j < 5)
        selectedButton = j;
      return 1;
    }
    // The volume control
    else if (i == 1) {
      selectedButton = VOLUME_CONTROL;
      mx -= 8;
      musicVolume = (mx * 256) >> 6;
      if (musicVolume <= 0)
        musicVolume = 0;
      if (musicVolume > 255)
        musicVolume = 255;
      setMusicVolume();
    } else if (i == 2) {
      if (j < 3) {
        selectedButton = 5 + j;
      } else if (j == 5)
        selectedButton = CLOSE;

    } else if (j == 5) {
      int ti = (my - sound_y) >> 3;
      if (ti == 6) {
        selectedButton = SCROLL_TOP;
      }
      if (ti == 17) {
        selectedButton = SCROLL_BOTTOM;
      } else if (ti > 6 && ti < 17) {
        int ty = my - sound_y - 56; // 8 for the up arrow
        topPlaylist = ty * numPlaylistEntries / 80;
        selectedButton = SCROLL_SLIDER;
      }
    } else if (i >= 3) {
      int ti = (my - sound_y) >> 3;
      if (ti >= 6 && ti <= 17) {
        int song = ti - 6 + topPlaylist;
        if (song >= 0 && song < numPlaylistEntries) {
          if (!playlistSelected[song] && !controlDown && !shiftDown)
            clearSelectedPlaylist();
          if (shiftDown) {
            int i = 0;
            int first = 0;

            while (i < numPlaylistEntries) {
              if (playlistSelected[i]) {
                first = i;
                break;
              }
              i++;
            }
            if (i != numPlaylistEntries) {
              if (first < song) {
                while (i < song) {
                  playlistSelected[i] = 1;
                  i++;
                }
              } else {
                while (i > song) {
                  playlistSelected[i] = 1;
                  i--;
                }
              }
            }
          }
          if (controlDown)
            playlistSelected[song] = !playlistSelected[song];
          else
            playlistSelected[song] = 1;
          selectedButton = PLAYLIST_SELECTION;
        }
      }
    } else {
      selectedButton = -1;
    }
    break;
  case WM_LBUTTONUP:

    if (i == 0) {
      if (j < 5) {
        if (selectedButton == j)
          actions[selectedButton](); // do the action here
      }
    }
    // The volume control
    else if (i == 1) {

    } else if (i == 2) {
      if (j < 3) {
        if (selectedButton == 5 + j) {
          actions[selectedButton]();
        }
      } else if (j == 5 && selectedButton == CLOSE) {
        actions[selectedButton]();
      }

    } else if (j == 5) {
      if (i == 3) {
        if (selectedButton == SCROLL_TOP) {
          topPlaylist--;
          if (topPlaylist < 0)
            topPlaylist = 0;
        }
      }
      if (i == 8) {
        if (selectedButton == SCROLL_BOTTOM) {
          topPlaylist++;
          if (topPlaylist + 12 >= numPlaylistEntries)
            topPlaylist = numPlaylistEntries - 12;
          if (topPlaylist < 0)
            topPlaylist = 0;
        }
      }
    }
    selectedButton = -1;
    break;

  case WM_MOUSEMOVE:
    if (selectedButton == VOLUME_CONTROL) {
      mx -= 8;
      musicVolume = (mx * 256) >> 6;
      if (musicVolume <= 0)
        musicVolume = 0;
      if (musicVolume > 255)
        musicVolume = 255;
      setMusicVolume();
    } else if (selectedButton == SCROLL_SLIDER) {
      int ty = my - sound_y - 56; // 8 for the up arrow
      topPlaylist = ty * numPlaylistEntries / 80;
      if (topPlaylist >= (numPlaylistEntries - 12)) {
        topPlaylist = numPlaylistEntries - 12;
      }
      if (topPlaylist < 0)
        topPlaylist = 0;
    }

    else if (selectedButton == PLAYLIST_SELECTION) {
      int ti = (my - sound_y) >> 3;
      if (ti < 6) {
        topPlaylist--;
        if (topPlaylist < 0)
          topPlaylist = 0;
      }
      if (ti > 17) {
        topPlaylist++;
        if ((topPlaylist + 12) >= numPlaylistEntries)
          topPlaylist = numPlaylistEntries - 12;
      } else {
        int song = ti - 6 + topPlaylist;
        if (song >= 0 && song < numPlaylistEntries) {
          // clearSelectedPlaylist();

          int i = 0;
          int lastSelected;
          int difference;
          i = numPlaylistEntries - 1;
          while (i >= 0) {
            if (playlistSelected[i]) {
              lastSelected = i;
              break;
            }
            i--;
          }
          i = 0;
          while (i < numPlaylistEntries) {
            if (playlistSelected[i])
              break;
            i++;
          }
          if (song == i)
            return 1;
          difference = lastSelected - i;
          if (song + difference >= numPlaylistEntries) {
            song = numPlaylistEntries - difference - 1;
            if (song <= 0)
              song = 0;
          }
          if (i != numPlaylistEntries) {
            char* back;
            int tsong;
            int offset = song - i;
            if (offset == 0)
              return 1;
            if (offset > 0) {
              int f = lastSelected;
              while (f >= i) {
                if (playlistSelected[f]) {
                  if (currentFile == f)
                    currentFile = (f + offset);
                  else if (currentFile == f + offset)
                    currentFile = f;
                  back = playlistEntryNames[f];
                  playlistEntryNames[f] = playlistEntryNames[f + offset];
                  playlistEntryNames[f + offset] = back;
                  back = playlistEntryPaths[f];
                  playlistEntryPaths[f] = playlistEntryPaths[f + offset];
                  playlistEntryPaths[f + offset] = back;
                  playlistSelected[f + offset] = 1;
                  playlistSelected[f] = 0;
                }
                f--;
              }
            } else {
              while (i <= lastSelected) {
                if (playlistSelected[i]) {
                  tsong = i + offset;
                  if (currentFile == i)
                    currentFile = (tsong);
                  else if (currentFile == tsong)
                    currentFile = i;
                  back = playlistEntryNames[i];
                  playlistEntryNames[i] = playlistEntryNames[tsong];
                  playlistEntryNames[tsong] = back;
                  back = playlistEntryPaths[i];
                  playlistEntryPaths[i] = playlistEntryPaths[tsong];
                  playlistEntryPaths[tsong] = back;
                  playlistSelected[tsong] = 1;
                  playlistSelected[i] = 0;
                }
                i++;
              }
            }
          }
          // selectedButton=PLAYLIST_SELECTION;
        }
      }
    }
    break;
  case WM_LBUTTONDBLCLK: {
    if (j < 5) {
      if (i >= 3) {
        int song = ((my - sound_y) >> 3) - 6 + topPlaylist;
        playPlaylistEntry(song);
      }
    }
  }
  case WM_KEYDOWN:
    if (wparam == VK_CONTROL)
      controlDown = 1;
    else if (wparam == VK_SHIFT)
      shiftDown = 1;
    break;
  case WM_KEYUP:
    if (wparam == VK_CONTROL)
      controlDown = 0;
    else if (wparam == VK_SHIFT)
      shiftDown = 0;
    break;
    break;
  }
  return 1;
}

int releaseSoundSyz() {
  int i;

  i = 0;
  while (i < 14) {
    if (buttons[i][0] != NULL) {
      buttons[i][0]->Release();
      buttons[i][0] = NULL;
    }
    if (buttons[i][1] != NULL) {
      buttons[i][1]->Release();
      buttons[i][1] = NULL;
    }
    i++;
  }
  i = 0;
  while (i < 2) {
    if (volume[i]) {
      volume[i]->Release();
      volume[i] = NULL;
    }
    i++;
  }
  playlistBack->Release();
  playlistBack = NULL;

  releaseFont(soundFont);
  releaseFont(soundFontSel);
  free(soundFont);
  free(soundFontSel);
  soundFont = NULL;
  soundFontSel = NULL;
  clearPlaylist();
  if (mainStream != NULL)
    FSOUND_Stream_Close(mainStream);
  freeSounds();
  FSOUND_Close();
  return 1;
}
