#include "../os_audio.h"

#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/AudioHardware.h>

struct OS_AudioDevice {
  OS_AudioDeviceHeader header;

  U32 id;
  AudioDeviceIOProcID proc_id;

  // -- AlNov: @TODO Remove. For testing purpose
  F32 phase;
};

func void
OS_PrintAvailableAudioDevices(Arena* arena) {
  AudioObjectPropertyAddress property = {
    .mSelector = kAudioHardwarePropertyDevices,
    .mScope = kAudioObjectPropertyScopeGlobal,
    .mElement = kAudioObjectPropertyElementMain,
  };

  U32 data_size = 0;
  AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &property, 0, 0, &data_size);
  U32 devices_count = data_size / sizeof(AudioDeviceID);
  LogDebug("Number of Available Devices: %u\n", devices_count);
  U32* devices_ids = (U32*)PushArena(arena, data_size);
  AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, 0, &data_size, devices_ids);

  for (I32 i = 0; i < devices_count; i += 1) {
    U32 device_id = devices_ids[i];
    AudioObjectPropertyAddress c_name_property = {
      .mSelector = kAudioDevicePropertyDeviceName,
      .mScope = kAudioObjectPropertyScopeGlobal,
      .mElement = kAudioObjectPropertyElementMain,
    };
    U32 name_size = 0;
    AudioObjectGetPropertyDataSize(device_id, &c_name_property, 0, 0, &name_size);
    char c_name[512];
    AudioObjectGetPropertyData(device_id, &c_name_property, 0, 0, &name_size, c_name);
    LogDebug("DeviceId: %u\tName: %s\n", device_id, c_name);
  }
}

func OSStatus
OS_MacOS_AudioRenderCallback(AudioDeviceID device_id, const AudioTimeStamp* now, const AudioBufferList* input_data, const AudioTimeStamp* input_time, AudioBufferList* output_data, const AudioTimeStamp* output_time, void* client_data) {
  OS_AudioDevice* audio_device = (OS_AudioDevice*)client_data;

  AudioBuffer* first_buffer = &output_data->mBuffers[0];
  U32 frame_count = first_buffer->mDataByteSize / (sizeof(F32)*first_buffer->mNumberChannels);

  for (U32 frame = 0; frame < frame_count; frame += 1) {
    for (I32 buffer_index = 0; buffer_index < output_data->mNumberBuffers; buffer_index += 1) {
      AudioBuffer* buffer = output_data->mBuffers + buffer_index;
      F32* samples = (F32*)buffer->mData;
      if (samples == 0) {
        continue;
      }

      if (audio_device->header.read_cursor < audio_device->header.write_cursor) {
        U32 index = audio_device->header.read_cursor % audio_device->header.samples_count;
        for (I32 channel = 0; channel < buffer->mNumberChannels; channel += 1) {
          samples[frame*buffer->mNumberChannels + channel] = audio_device->header.buffers[channel][index];
        }
        audio_device->header.read_cursor += 1;
      }
      else {
        for (I32 channel = 0; channel < buffer->mNumberChannels; channel += 1) {
          samples[frame*buffer->mNumberChannels + channel] = 0.0f;
        }
      }
    }
  }

  return noErr;
}


func OS_AudioDevice
OS_CreateAudioDevice(Arena* arena) {
  OS_AudioDevice device = {};
  AudioObjectPropertyAddress device_id_property = {
    .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
    .mScope = kAudioObjectPropertyScopeGlobal,
    .mElement = kAudioObjectPropertyElementMain,
  };
  U32 device_id_size = sizeof(U32);
  U32 device_id = 0;
  AudioObjectGetPropertyData(kAudioObjectSystemObject, &device_id_property, 0, 0, &device_id_size, &device_id);
  LogDebug("Default Output Device's ID: %u\n", device_id);
  device.id = device_id;

  AudioObjectPropertyAddress sample_rate_property = {
    .mSelector = kAudioDevicePropertyActualSampleRate,
    .mScope = kAudioObjectPropertyScopeGlobal,
    .mElement = kAudioObjectPropertyElementMain,
  };
  U32 sample_rate_size = sizeof(double);
  double sample_rate = 0.0;
  AudioObjectGetPropertyData(device_id, &sample_rate_property, 0, 0, &sample_rate_size, &sample_rate);
  LogDebug("SampleRate: %lf\n", sample_rate);
  device.header.sample_rate = (F32)sample_rate;

  AudioObjectPropertyAddress buffer_frame_size_property = {
    .mSelector = kAudioDevicePropertyBufferFrameSize,
    .mScope = kAudioObjectPropertyScopeGlobal,
    .mElement = kAudioObjectPropertyElementMain,
  };
  U32 buffer_frame_size_property_size = sizeof(U32);
  U32 buffer_frame_size = 0;
  AudioObjectGetPropertyData(device_id, &buffer_frame_size_property, 0, 0, &buffer_frame_size_property_size, &buffer_frame_size);
  LogDebug("Buffer frame size: %u\n", buffer_frame_size);

  AudioObjectPropertyAddress stream_property = {
    .mSelector = kAudioDevicePropertyStreamFormat,
    .mScope = kAudioObjectPropertyScopeGlobal,
    .mElement = kAudioObjectPropertyElementMain,
  };
  U32 stream_property_size = 0;
  AudioObjectGetPropertyDataSize(device_id, &stream_property, 0, 0, &stream_property_size);
  AudioStreamBasicDescription stream_format = {};
  AudioObjectGetPropertyData(device_id, &stream_property, 0, 0, &stream_property_size, &stream_format);

  device.header.channels_count = stream_format.mChannelsPerFrame;
  U32 buffer_size = buffer_frame_size*sizeof(F32);
  LogDebug("Buffer size: %u\n", buffer_size);
  device.header.samples_count = 4096;
  device.header.buffers[0] = (F32*)PushArena(arena, device.header.samples_count*sizeof(F32));
  device.header.buffers[1] = (F32*)PushArena(arena, device.header.samples_count*sizeof(F32));

  LogDebug("Channels: %u\n", stream_format.mChannelsPerFrame);
  LogDebug("Bits per channel: %u\n", stream_format.mBitsPerChannel);
  LogDebug("Bytes per frame: %u\n", stream_format.mBytesPerFrame);
  LogDebug("Format ID: %u\n", stream_format.mFormatID);
  LogDebug("Interleaved: %u\n", !((stream_format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved));
  return device;
}

func void
OS_AudioDevicePlay(OS_AudioDevice* device) {
  Assert(device != 0);
  
  AudioDeviceCreateIOProcID(device->id, OS_MacOS_AudioRenderCallback, device, &device->proc_id);
  Assert(device->proc_id != 0);

  OSStatus status = AudioDeviceStart(device->id, device->proc_id);
}
