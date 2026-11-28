#pragma once

#include "audio_loader.h"

struct {
  B32 initialized;

  Arena*         arena;
  OS_AudioDevice device;
} au_context;

func void AU_Init();
func void AU_Shutdown();

typedef struct AU_Sound AU_Sound;
struct AU_Sound {
  U32 samples_count;
  F32* samples[2]; // Left and Right channels
};

func AU_Sound AU_LoadSoundFromWAV(Arena* arena, Str8 file_path);

typedef struct AU_PlayerEntry AU_PlayerEntry;
struct AU_PlayerEntry {
  AU_Sound* sound;
  U64       cursor;
  B32       looped;
};

typedef struct AU_Player AU_Player;
struct AU_Player {
  AU_PlayerEntry entries[1024]; // --AlNov: @NOTE @TODO Add memory managment for PlayerEntries
  I32            entries_count;

  F32 volume;
};

func AU_Player AU_CreatePlayer();

func void AU_PlayerUpdate(AU_Player* player);
func void AU_PlayerPlaySound(AU_Player* player, AU_Sound* sound);
