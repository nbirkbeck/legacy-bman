#include "font.h"
#ifndef SOUNDSYZ
#define SOUNDSYZ

#define MAX_PLAYLIST_ENTRIES 128

int playBombSound();
int loadSounds();
int playMenuItemSound();
int playMenuSelectSound();
int playPlayerSound(int p);
int playKickSound();
int drawSoundSyz();
int loadDirectory(char*);
int initSoundSyz();
int releaseSoundSyz();
int sendMessageSoundSyz(int, int, int);
#endif
