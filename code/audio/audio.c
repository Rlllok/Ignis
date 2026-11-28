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

func AU_PlayerEntriesChunk*
AU_AllocatePlayerEntriesChunk(Arena* arena, AU_Player* player, U32 capacity) {
  AU_PlayerEntriesChunk* result = (AU_PlayerEntriesChunk*)PushArena(au_context.arena, sizeof(AU_PlayerEntriesChunk));
  result->entries = (AU_PlayerEntry*)PushArena(au_context.arena, capacity*sizeof(AU_PlayerEntry));
  result->length = 0;
  result->capacity = capacity;
  return result;
}

func AU_Player
AU_CreatePlayer() {
  AU_Player result = ZeroStruct();
  result.volume = 0.3f;

  AU_PlayerEntriesChunk* chunk = AU_AllocatePlayerEntriesChunk(au_context.arena, &result, 256);
  DllPushBack(result.first_chunk, result.last_chunk, chunk);

  return result;
}

func void
AU_PlayerUpdate(AU_Player* player) {
  U32 target_samples = 512;
  U32 available_samples = au_context.device.header.write_cursor - au_context.device.header.read_cursor;
  U32 samples_to_generate = target_samples - available_samples;

  if (samples_to_generate > 0) {
    for (I32 sample_index = 0; sample_index < samples_to_generate; sample_index += 1) {
      F32 sample_left = 0.0f;
      F32 sample_right = 0.0f;

      for (AU_PlayerEntriesChunk* chunk = player->first_chunk; chunk != 0; chunk = chunk->next) {
        I32 entry_index = 0;
        while (entry_index < chunk->length) {
          AU_PlayerEntry* entry = chunk->entries + entry_index;

          if (entry->cursor < entry->sound->samples_count) {
            sample_left += entry->sound->samples[0][entry->cursor];
            sample_right += entry->sound->samples[1][entry->cursor];
            entry->cursor += 1;
          }
          else {
            if (entry->looped) {
              entry->cursor = 0;
            }
            else {
              if (chunk->length != 0) {
                *entry = chunk->entries[chunk->length - 1];
                chunk->length -= 1;
                entry_index -= 1;
              }
            }
          }

          entry_index += 1;
        }
      }

      sample_left = Clamp(sample_left*player->volume, -1.0f, 1.0f);
      sample_right = Clamp(sample_right*player->volume, -1.0f, 1.0f);

      U32 device_sample_index = au_context.device.header.write_cursor % au_context.device.header.samples_count;

      au_context.device.header.buffers[0][device_sample_index] = sample_left;
      au_context.device.header.buffers[1][device_sample_index] = sample_right;
      au_context.device.header.write_cursor += 1;
    }
  }
}

func void
AU_PlayerPlaySound(AU_Player* player, AU_Sound* sound) {
  AU_PlayerEntry* entry = 0;

  for (AU_PlayerEntriesChunk* chunk = player->first_chunk; chunk != 0; chunk = chunk->next) {
    if (chunk->length < chunk->capacity) {
      entry = chunk->entries + chunk->length;
      chunk->length += 1;
      break;
    }
  }

  if (entry != 0) {
    *entry = (AU_PlayerEntry)ZeroStruct();
    entry->sound = sound;
  }
  else {
    AU_PlayerEntriesChunk* chunk = AU_AllocatePlayerEntriesChunk(au_context.arena, player, 256);
    entry = chunk->entries + chunk->length;
    chunk->length += 1;

    *entry = (AU_PlayerEntry)ZeroStruct();
    entry->sound = sound;

    DllPushBack(player->first_chunk, player->last_chunk, chunk);
  }
}
