rem --- Compile Shaders --- 
@echo on
@echo Start shader compilation.
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" triangle.vert -o triangleVS.spv
"C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" triangle.frag -o triangleFS.spv
@echo.
@echo End shader compilation.
@echo off