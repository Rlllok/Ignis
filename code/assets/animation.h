#pragma once

#include "base/base_include.h"

U64 _timestamp_nil = 0;
DefineArray(U64, TimestampArray, _timestamp_nil)

typedef U16 AnimationCurveType;
enum AnimationCurveTypeEnum
{
  AnimationCurveType_Step,
  AnimationCurveType_Linear,
} AnimationCurveTypeEnum;

typedef struct AnimationCurve AnimationCurve;
struct AnimationCurve
{
  AnimationCurveType type;
  union
  {
    struct {
      TranformArray transforms;
      TimestampArray timestamps;
    } step;

    struct {
      TranformArray transforms;
      TimestampArray timestamps;
    } linear;
  };
};

func AnimationCurve CreateAnimationCurve(Arena* arena, AnimationCurveType type, U32 keys_number);
func B32 AnimationCurveAddPoint(AnimationCurve* curve, Transform tranfsorm, U64 timestamp);

typedef struct Animation Animation;
struct Animation
{
  AnimationCurve curve;
  U32 duration;
  B32 looped;
  U32 start_timestamp;
};

func Transform AnimateTransform(Animation animation, U32 timestamp);
