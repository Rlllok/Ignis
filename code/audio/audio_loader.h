#pragma once

typedef struct AU_WaveFile AU_WaveFile;
struct AU_WaveFile {
  U32  channels_count;
  U32  samples_count;
  F32* samples[2]; // --AlNov: Maximum two channels are supported.

  U64 cursor;
};

func AU_WaveFile AU_LoadWaveFile(Arena* arena, Str8 path);
