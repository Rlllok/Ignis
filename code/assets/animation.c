#pragma once

#include "animation.h"

func Transform
AnimateTransform(Animation animation, U32 timestamp)
{
  Transform result = IdentityTransform();

  U32 animation_time = (timestamp - animation.start_timestamp);
  if (animation.looped)
  {
    animation_time = animation_time%animation.duration;
  }

  AnimationPoint current_point = AnimationPointArrayGet(&animation.points, 0);
  AnimationPoint next_point = AnimationPointArrayGet(&animation.points, 0);
  for (I32 i = 1; i < animation.points.length; i += 1)
  {
    next_point = AnimationPointArrayGet(&animation.points, i);
    if (next_point.timestamp > animation_time) break;
    current_point = next_point;
  }

  F32 blend_value = (F32)(animation_time - current_point.timestamp)/(F32)(next_point.timestamp - current_point.timestamp);
  blend_value = Clamp(blend_value, 0.0f, 1.0f);

  switch (current_point.type)
  {
    default:
    {
      AssertMessage(0, "Wrong AnimationCurve type\n");
    }
    
    case AnimationPointType_Linear:
    {
      result.translation = LerpVec3F32(current_point.linear.transform.translation, next_point.linear.transform.translation, blend_value);
      result.rotation = SlerpQuaternion(current_point.linear.transform.rotation, next_point.linear.transform.rotation, blend_value);
      result.scale = LerpVec3F32(current_point.linear.transform.scale, next_point.linear.transform.scale, blend_value);

      // LOG_DEBUG("Transform 0\t: %.2fx %.2fy %.2fz\n", current_point.linear.transform.translation.x, current_point.linear.transform.translation.y, current_point.linear.transform.translation.z);
      LOG_DEBUG("Animation Time\t%f\n", animation_time/1000.0f);
      LOG_DEBUG("Blend value\t%f\n", blend_value);
      LOG_DEBUG("CurrentTransform 0\t: %.2fx %.2fy %.2fz\n", result.translation.x, result.translation.y, result.translation.z);
    } break;
  }
  
  return result;
}
