#pragma once

#include "animation.h"

func AnimationCurve
CreateAnimationCurve(Arena* arena, AnimationCurveType type, U32 keys_number)
{
  AnimationCurve result = {0};
  result.transforms = TransformArrayAllocate(arena, keys_number);
  result.timestamps = TimestampArrayAllocate(arena, keys_number);
  return result;
}

func B32
AnimationCurveAddPoint(AnimationCurve* curve, Transform tranfsorm, U64 timestamp);
{
  TransformArrayAdd(&curve->transforms, tranform);
  TimestampArrayAdd(&curve->timestamp, timestamp);
}

func Transform
AnimateTransform(Animation animation, U32 timestamp)
{
  Transform result = IdentityTransform();

  U32 animation_time = (timestamp - animation.start_timestamp);
  if (animation.looped)
  {
    animation_time = animation_time%animation.duration;
  }

  AnimationSample sample = animation.samples[0];
  for (I32 i = 1; i < 2; i += 1)
  {
    if (animation.samples[i].timestamp > animation_time) break;

    sample = animation.samples[i];
  }

  AnimationCurve curve = sample.curve;

  F32 blend_value = (F32)(animation_time - curve.start_timestamp)/(F32)(curve.end_timestamp - curve.start_timestamp);
  blend_value = Clamp(blend_value, 0.0f, 1.0f);

  switch (curve.type)
  {
    default:
    {
      AssertMessage(0, "Wrong AnimationCurve type\n");
    }
    
    case AnimationCurveType_Linear:
    {
      result.translation = LerpVec3F32(curve.start_transform.translation, curve.end_transform.translation, blend_value);
      result.rotation = SlerpQuaternion(curve.start_transform.rotation, curve.end_transform.rotation, blend_value);
      result.scale = LerpVec3F32(curve.start_transform.scale, curve.end_transform.scale, blend_value);

      LOG_DEBUG("Transform 0\t: %.2fx %.2fy %.2fz\n", curve.start_transform.translation.x, curve.start_transform.translation.y, curve.start_transform.translation.z);
      LOG_DEBUG("Animation Time\t%f\n", animation_time/1000.0f);
      LOG_DEBUG("Blend value\t%f\n", blend_value);
      LOG_DEBUG("CurrentTransform 0\t: %.2fx %.2fy %.2fz\n", result.translation.x, result.translation.y, result.translation.z);
      LOG_DEBUG("Transform 1\t: %.2fx %.2fy %.2fz\n", curve.end_transform.translation.x, curve.end_transform.translation.y, curve.end_transform.translation.z);
    } break;
  }
  
  return result;
}
