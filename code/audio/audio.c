#include "audio.h"

#include "audio_loader.c"

func void
AU_Init() {
  au_context.arena = AllocateArena(Gigabytes(2), Kilobytes(32));
  au_context.device = OS_CreateAudioDevice(au_context.arena);
  OS_AudioDevicePlay(&au_context.device);
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
  result.volume = 0.3f;
  // @TODO
  return result;
}

func void
AU_PlayerUpdate(AU_Player* player) {
  if (player->entries_count > 0) {
    AU_PlayerEntry* player_entry = player->entries;
    if (player_entry->cursor < player_entry->sound->samples_count) {
      U32 target_samples = 512;
      U32 available_samples = au_context.device.header.write_cursor - au_context.device.header.read_cursor;
      if (available_samples < target_samples) {
        U32 samples_to_generate = target_samples - available_samples;
        F32 volume = player->volume;
        for (I32 sample_index = 0; sample_index < samples_to_generate; sample_index += 1) {
          U32 sample_index = player_entry->cursor % player_entry->sound->samples_count;
          player_entry->cursor += 1;

          U32 index = au_context.device.header.write_cursor % au_context.device.header.samples_count;
          au_context.device.header.buffers[0][index] = player_entry->sound->samples[0][sample_index]*volume;
          au_context.device.header.buffers[1][index] = player_entry->sound->samples[1][sample_index]*volume;
          au_context.device.header.write_cursor += 1;
        }
      }
    }
  }
}

func void
AU_PlayerPlaySound(AU_Player* player, AU_Sound* sound) {
  AU_PlayerEntry* entry = player->entries + player->entries_count;
  *entry = (AU_PlayerEntry)ZeroStruct();
  entry->sound = sound;
  player->entries_count += 1;
}
