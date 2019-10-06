mkdir shader-bin
%VULKAN_SDK%\Bin32\glslc.exe shaders\simple-draw.vert          -o shader-bin\simple-draw-vert.spv
%VULKAN_SDK%\Bin32\glslc.exe shaders\simple-draw.frag          -o shader-bin\simple-draw-frag.spv
REM %VULKAN_SDK%\Bin32\glslc.exe shaders\simple-draw-textured.vert -o shader-bin\simple-draw-textured-vert.spv
REM %VULKAN_SDK%\Bin32\glslc.exe shaders\simple-draw-textured.frag -o shader-bin\simple-draw-textured-frag.spv