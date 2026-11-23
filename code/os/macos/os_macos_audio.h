#pragma once

#pragma push_macro("func")
#undef func
#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardware.h>
#pragma pop_macro("func")

struct OS_AudioDevice {
  OS_AudioDeviceHeader header;

  U32 id;
  AudioDeviceIOProcID proc_id;
};
