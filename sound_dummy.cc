#include <SDL2/SDL_audio.h>
#include <iostream>

SDL_AudioDeviceID audio_dev;

void LazyInit();

struct Sound {
  Uint8* audio_buf = 0;
  Uint32 audio_len = 0;

  void Play() {
    if (!audio_buf) {
      LazyInit();
    }
    SDL_PauseAudioDevice(audio_dev, 0);

    SDL_ClearQueuedAudio(audio_dev);
    SDL_QueueAudio(audio_dev, audio_buf, audio_len);
  }
};

Sound explosion_sound;
Sound menu_item_sound;
Sound menu_select_sound;
Sound kick_sound;

int playMenuItemSound() {
  menu_item_sound.Play();
  return 0;
}
int playMenuSelectSound() {
  menu_select_sound.Play();
  return 0;
}
int playKickSound() {
  kick_sound.Play();
  return 0;
}

int playBombSound() {
  explosion_sound.Play();
  return 0;
}

int drawSoundSyz() { return 0; }

int loadDirectory(char*) { return 0; }

int initSoundSyz() { return 0; }

void LazyInit() {
  std::cout << "Init sounds\n";
  SDL_AudioSpec desired_audio_spec = {0};
  desired_audio_spec.freq = 44100;
  desired_audio_spec.channels = 2;
  desired_audio_spec.samples = 2048;
  desired_audio_spec.format = AUDIO_U8;
  desired_audio_spec.callback = 0;

  SDL_AudioSpec audio_spec;
  audio_dev =
      SDL_OpenAudioDevice(nullptr, 0, &desired_audio_spec, &audio_spec, 0);

  SDL_LoadWAV("data/sounds/explosion1.wav", &audio_spec,
              &explosion_sound.audio_buf, &explosion_sound.audio_len);
  SDL_LoadWAV("data/sounds/menuitem.wav", &audio_spec,
              &menu_item_sound.audio_buf, &menu_item_sound.audio_len);
  SDL_LoadWAV("data/sounds/menuselect.wav", &audio_spec,
              &menu_select_sound.audio_buf, &menu_select_sound.audio_len);
  SDL_LoadWAV("data/sounds/kick.wav", &audio_spec, &kick_sound.audio_buf,
              &kick_sound.audio_len);

  std::cout << "Done init audio\n";
}
