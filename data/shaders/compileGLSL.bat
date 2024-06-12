rem --- Compile Shaders --- 
@echo on
@echo Start shader compilation.
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default3D.vert -o default3DVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default3D.frag -o default3DFS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default2D.vert -o default2DVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" default2D.frag -o default2DFS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" line.vert -o lineVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" line.frag -o lineFS.spv
@echo.
@echo End shader compilation.
@echo off
