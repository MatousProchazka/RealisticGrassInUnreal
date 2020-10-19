************************************************************************************************
************************************************************************************************
******************************************GRASS PLUGIN:*****************************************
************************************************************************************************
************************************************************************************************

This  plugin was created under Bohemia Interactive and uses adjusted model made by Klemens Jahrmann and Michael Wimmer.

Plugin requirements*:
 Windows OS
 Ability to run Unreal Engine
 Dedicated GPU
 

 *Since plugin was not properly tested on many different computers the requirements are just orientational. To tailor plugin more to your computer, look into PLUGIN PERFORMANCE CALIBRARION section


Plugin was created on Unreal Engine 4.22. Full functionality on different versions is not guaranteed!

Plugin is created for generating grass within Unreal engine. 
For decription of main features hover over attribute within the plugin
------------------------------------------------------------------------------------------------------------------------------------------------------------
PLUGIN ACTIVATION:
------------------------------------------------------------------------------------------------------------------------------------------------------------
1.Take the folder GrassPlugin and insert it into "Plugins" folder within your Unreal project (if there is none, create one)
2.Open project in Unreal Engine, then go into Edit->Plugins and find at the bottom of the list on the left Group "Project". Inside should be plugin GrassPlugin. Check the box "Enabled" and restart the engine.
3. Plugin should now appear in Modes on the left(grass picture)
	
	
------------------------------------------------------------------------------------------------------------------------------------------------------------
PLUGIN PERFORMANCE CALIBRATION (IMPORTANT):
------------------------------------------------------------------------------------------------------------------------------------------------------------
 Despite many inbuild optimizations of Unreal, the grass could be demanding on performance especially with big spaces.
 Generating of grass is heavily dependant on used computer and Unreal engine (version, and/or custom adjustments of engine) and can be quite demanding aswell. 
 Since the plugin could be generating millions of grass blades, plugin uses DRAM aswell as VRAM of GPU to spread the computation and speed up generating. 
 To prevent crashes and performance overload(especially with GPU memory) the plugin allows some configuration of performance within GrassPluginConfig.ini located within: "../YourProject/Saved/Config/Windows" 
 (file is generated upon compilation of engine with plugin turned on).
 If your Unreal crashes because of too much RAM usage and you can reduce it by generating grass in smaller pieces.
 Config variables regarding performance:
RAM
 memoryBarrier(True/False) - Turns on(True)/off(False) RAM memory barrier
 minMemoryRemaining(Int) - If RAM memory barrier is turned on, plugin stops generating when amount of available RAM memory is less or euqal than this attribute value
 minGPUMEmoryRemaining(Int) - More of an informative attribute. When plugin generates amount of grass positions higher than this attribute, the warning will appear with options to cancel generating or continue.
GPU
 subSpaceMaxWidth(Int) - The higher the attribute, the more demanding will plugin be on GPU(increasing speed of generating). Based on this attribute plugin separates grass amount into loads being given to GPU
 maxInstanceLimitPG(Int) -  
Runtime
 detailedGrassCullDistance - determines distance from camera at which will the detailed grass get culled (highly influences performance)
 lodCullDistanceFar - determines distance from camera at which will the grass LOD get culled
Configuration file also allow changing of possition of Materials and Meshes

------------------------------------------------------------------------------------------------------------------------------------------------------------
ADVANCED PLUGIN OPTIONS
------------------------------------------------------------------------------------------------------------------------------------------------------------
Aside from attributes within plugin, you can adjust a lot of parameters of BillBoardMat (adjusting the visuals of grass LODs), M_GrassMat (adjusting the visuals of detailed grass)
Plugin also allows to generate grass based on adaptive sampling. Therefore you can add your own texture into "GrassPlugin/Content/Textures". Texture has to be grayscale and in .png format.
Based on texture grass will be generated (black = high density, complete white = no grass)

When snapping grass with shouldSnapToTerrain, the grass gets culled if there is static object above the grass. To generate grass nevertheless of the object above set object collision response to WorldStatic on Overlap/Ignore
In case you want grass to snap onto the object above terrain add tag "grassEnable" (grass collision is set only to landscape collision, therefore grass on objects wont trigger collision with Pawn)
 
 
 
 
 
 