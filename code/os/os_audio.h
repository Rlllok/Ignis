#pragma once

typedef struct OS_AudioDeviceHeader OS_AudioDeviceHeader;
struct OS_AudioDeviceHeader {
  U32  channels_count;
  F32  sample_rate;
  U32  samples_count;
  U32  read_cursor;
  U32  write_cursor;
  F32* buffers[2]; // --AlNov: Currently maximum number of channels is 2.
};
typedef struct OS_AudioDevice OS_AudioDevice;

func void OS_PrintAvailableAudioDevices(Arena* arena);
func OS_AudioDevice OS_CreateAudioDevice(Arena* arena);
func void OS_AudioDevicePlay(OS_AudioDevice* device);
func void OS_AudioDeviceStop(OS_AudioDevice* device);
