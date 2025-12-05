## 5 December 2025
Cannot read SimpleBoneAnimation.gltf. Doesn't understand the reason. And what is insteresting - there is no message, but LOG_ERROR is called (gltf.c:76).

Will work on command_palette.

## 4 December 2025
I didn't add helper functions to read data from accessors. This is what I want to do today.

SimpleAnimation.gltf is working.

Loaded SkeletalAnimation data. It is not fully working. Skeleton is moving, but only moving. I think, the reason is InverseKenematic - I used IK bones when animated my DummySkeleton.

So the next goal is to create simple skeletal animation (without "utility" bones) and try to run it.

## 3 December 2025
The last session ended on try to move AnimationCurve. And it was not finished. After a pause I understand that it should be better named as Point(or Sample). That means that animation will contain some number of points that represents transformation for animation target. With such perspective there is no differents between sampled animation or curved animation. If it is a curve it will just have less points then sampled one.

For now I want to do two things - finish simple api for animation that will just work (only linear point type for now) and read animation data from simple gltf file.

I did it. It reads gltf file, gets animation data from it and plays animation.

I want to clean some code for gltf loader. At least, add helper function to read data from accessor.
