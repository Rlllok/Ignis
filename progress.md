# 16 December 2025
Started UI rewrite. It is not working for now. There is a problem about how to build child array and slice it for every parent.

# 12 December 2025
Skeletal animation is currently in a state when it working. There are a lot of room for improvement. The first thing I want to do is support loading multiple animations.

For now I am going to store animations in Skeleton structure. I think this is not a kind of approach that I want to end with. As I see, Skeleton and Animations should be separated. I should play with other game engines to find what I like and don't like.

# 11 December 2025
Removed/moved code.

# 10 December 2025
There is the problem when samples count for animation channels are not equal. For now, I turned of optimization when exporting Blender scene.

# 6 December 2025
Not sure what the problem was but SimpleBoneAnimation.gltf is now working without problem. Maybe, there was an error in path to file.

Skeletal animation is working now. Tested on simple case with one bone (SimpleBoneAnimation.gltf). Now I am more sure that the problem with Dummy.gltf is IK bones - they are not supported for now. This is why skeleton just moves up and down. 

There was another issue. For now I create animation samples based on the count in for first GLTFChannel. But it could be situation, and it was, that not all channels have the same number of samples (frames). For SimpleBoneAnimation.gltf scale doesn't change overtime. This is why there is only 2 samples (It is an optimization from Blender export. 2 is a minimum number in such case).

It could be a good idea to create animation for different values. What I mean by this - separate Transform to it's components. One animation per Translation, Rotation and Scale. I like such system because I can create animation that will accept any value (F32, Vec3F32, Vec4F32 ...). The problem is memory. We save memory in such situation as was described earlier (SImpleBoneAnimation and Scale channel). But if we have situation when every channel has the same number of samples, we store the same value of time per every sample for every channel.

## 5 December 2025
Cannot read SimpleBoneAnimation.gltf. Doesn't understand the reason. And what is insteresting - there is no message, but LOG_ERROR is called (gltf.c:76).

Will work on command_palette.

Started drawing process for CommandPalette. Added simple command to toggle it.

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
