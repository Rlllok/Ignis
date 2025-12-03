#pragma once

#include "base/base_include.h"

typedef U16 AnimationPointType;
enum AnimationCurveTypeEnum
{
  AnimationPointType_Step,
  AnimationPointType_Linear,
} AnimationPointTypeEnum;

typedef struct AnimationPoint AnimationPoint;
struct AnimationPoint
{
  AnimationPointType type;
  U64 timestamp;

  union
  {
    struct {
      Transform transform;
    } step;

    struct {
      Transform transform;
    } linear;
  };
};
AnimationPoint _animation_point_nil = {.linear.transform = IdentityTransform()};
DefineArray(AnimationPoint, AnimationPointArray, _animation_point_nil)

typedef struct Animation Animation;
struct Animation
{
  AnimationPointArray points;
  U32 duration;
  B32 looped;
  U32 start_timestamp;
};

func Transform AnimateTransform(Animation animation, U32 timestamp);
