#include "compat.h"
#include "t3dlib3.h"
#include "sound.h"
#include "font.h"

#define NUM_BOMB_SOUNDS 4
extern Font font;

int bombSounds[NUM_BOMB_SOUNDS];
int walkSounds[4];
int menuItemSound[2];
int kickSound;
int menuSelectSound;
int loadSounds()
{
	/*
	bombSounds[0] = DSound_Load_WAV("./data/sounds/exp1.wav");
	DSound_Set_Freq(bombSounds[0],11025);
	bombSounds[1] = DSound_Replicate_Sound(bombSounds[0]);
	DSound_Set_Freq(bombSounds[1],11025);
	bombSounds[2] = DSound_Replicate_Sound(bombSounds[0]);
	DSound_Set_Freq(bombSounds[2],11025);
	bombSounds[3] = DSound_Replicate_Sound(bombSounds[0]);
	DSound_Set_Freq(bombSounds[3],11025);

	menuItemSound[0] = DSound_Load_WAV("./data/sounds/menuitem.wav");
	menuItemSound[1] = DSound_Replicate_Sound(menuItemSound[0]);

	menuSelectSound = DSound_Load_WAV("./data/sounds/menuselect.wav");

	walkSounds[0] = DSound_Load_WAV("./data/sounds/walk.wav");
	walkSounds[1] = DSound_Replicate_Sound(walkSounds[0]);
	walkSounds[2] = DSound_Replicate_Sound(walkSounds[0]);
	walkSounds[3] = DSound_Replicate_Sound(walkSounds[0]);

	kickSound = DSound_Load_WAV("./data/sounds/kick.wav");

	DSound_Set_Freq(walkSounds[0],22050);
	DSound_Set_Freq(walkSounds[1],22050);
	DSound_Set_Freq(walkSounds[2],22050);
	DSound_Set_Freq(walkSounds[3],22050);

	DSound_Set_Freq(menuItemSound[0],11025);
	DSound_Set_Volume(menuItemSound[0],50);
	DSound_Set_Freq(menuItemSound[1],11025);
	DSound_Set_Volume(menuItemSound[1],50);

	DSound_Set_Volume(bombSounds[0],50);
	DSound_Set_Volume(bombSounds[1],40);
	DSound_Set_Volume(bombSounds[2],55);
	DSound_Set_Volume(bombSounds[3],60);

	*/
	return 1;
	
}

/*
int playKickSound()
{
	DSound_Play(kickSound);
	return 1;
}

int playPlayerSound(int p)
{
	DSound_Play(walkSounds[p]);
	return 1;
}

int playMenuItemSound()
{
	static int count=0;

	DSound_Play(menuItemSound[count]);
	count=(count+1)%2;
	return 1;
}

int playMenuSelectSound()
{
	DSound_Play(menuSelectSound);
	return 1;
}

int playBombSound()
{
	static unsigned int lastIndex=0;
	int i=0;
	DSound_Play(bombSounds[lastIndex]);

	DSound_Set_Volume(bombSounds[lastIndex],40+rand()%50);
	DSound_Set_Freq(bombSounds[lastIndex],8000+rand()%4500);
	lastIndex=(lastIndex+1)%NUM_BOMB_SOUNDS;


	return 1;
}
*/
int playKickSound(){return 1;}
int playPlayerSound(int p) {return 1;}
int playMenuItemSound(){return 1;}
int playMenuSelectSound(){return 1;}
int playBombSound(){return 1;}
