rem --- Compile Shaders --- 
@echo on
@echo Start shader compilation.
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default3D.vert -o default3DVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default3D.frag -o default3DFS.spv

"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default3Dtest.vert -o default3DVStest.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default3Dtest.frag -o default3DFStest.spv

"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default2D.vert -o default2DVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default2D.frag -o default2DFS.spv

"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" line.vert -o lineVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" line.frag -o lineFS.spv

"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" defaultFullscreen.vert -o defaultFullscreenVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" defaultFullscreen.frag -o defaultFullscreenFS.spv

"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" SDF.vert -o SDFVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" SDF.frag -o SDFFS.spv

"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" noise.vert -o noiseVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" noise.frag -o noiseFS.spv

"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" skybox.vert -o skyboxVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" skybox.frag -o skyboxFS.spv

@echo.
@echo End shader compilation.
@echo off
