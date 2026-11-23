#include "audio.h"

#include "audio_loader.c"

func void
AU_Init() {
  au_context.arena = AllocateArena(Gigabytes(2), Kilobytes(32));
  au_context.device = OS_CreateAudioDevice(au_context.arena);
}

func void
AU_Shutdown() {
  // @TODO
}

func AU_Sound
AU_LoadSoundFromWAV(Arena* arena, Str8 file_path) {
  AU_Sound result = ZeroStruct();

  ScratchArena scratch = BeginScratchArena(au_context.arena); {
    AU_WaveFile wav = AU_LoadWaveFile(scratch.arena, file_path);
    result.samples_count = wav.samples_count;
    result.samples[0] = (F32*)PushArena(arena, sizeof(F32)*result.samples_count);
    memcpy(result.samples[0], wav.samples[0], sizeof(F32)*result.samples_count);
    result.samples[1] = (F32*)PushArena(arena, sizeof(F32)*result.samples_count);
    memcpy(result.samples[1], wav.samples[1], sizeof(F32)*result.samples_count);
  }
  EndScratchArena(scratch);

  return result;
}

func AU_Player
AU_CreatePlayer() {
  AU_Player result = ZeroStruct();
  // @TODO
  return result;
}

func void
AU_PlayerUpdate(AU_Player* player) {
}

func void
AU_PlayerPlaySound(AU_Player* player, AU_Sound* sound) {
}
