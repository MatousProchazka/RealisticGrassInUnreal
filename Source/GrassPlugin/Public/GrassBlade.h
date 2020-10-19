// Copyright 2020 Matous Prochazka, Bohemia Interactive, a.s.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//
//   3. Neither the name of the copyright holder nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Runtime/CoreUObject/Public/UObject/Object.h"
#include "AssetRegistryModule.h"
#include "HelperFunctions.h"
#include "GVar.h"


#if WITH_EDITOR 
#include "Editor.h"
#include "LevelEditorViewport.h"
#endif

#include "EngineUtils.h"

#include "GrassBlade.generated.h"



UCLASS()
class GRASSPLUGIN_API AGrassBlade : public AActor //outdated name, grass patch would probably be more appropriate
{
	GENERATED_BODY()

public:
	AGrassBlade();
	AGrassBlade(const FObjectInitializer& objectInitializer);
	
	//Removes all instances of grass generated in this class
	void ClearInstances();

	//Spawns given amount of sole grass blades in given space
	void SpawnGrassBlades(int amount, int startIndex, FVector4 bounds, FVector patchPosition, FVector textureCorner, int textureWidth, bool shouldSnapToTerrain);

	//Spawns turf of grass around given position based on given attributes
	//@param amount - amount of grass in turf
	//@param radius - distance from position in which can grass blade be randomly positioned
	//@param position - placement of center of turf
	//@param bDebugCubes - should put debug cubes on grass positions?
	//@param cubeSize - size of debug cube
	//@param shouldSnapToTerrain - should the grass be modes onto height of landscape/tagged objects?
	//@param normalQuat - gives quaternion of normal generated based on terrain normal (if input FQuat is FQuat::Identity, the normal quaternion is generated within this function)
	void SpawnGrassBladesAroundPosition(int amount, int radius, FVector position, bool shouldSnapToTerrain, FQuat normalQuat);

	//Spawns Flowers around given position based on given attributes
	void SpawnFlowersAroundPosition(FVector position, int minAmount, int maxAmount, float innerFlowerRadius, EGPFlower flowerKind, bool shouldSnapToTerrain, FQuat normalQuat, float spawnWeight);

	//Spawns just one grass blade plus debug cube to check functionality of single grass
	void SpawnDefaultObject(int index);

	//Helper function for initialization of the materials
	void InitializeMaterials(UMaterialInstanceDynamic* mat, UMaterialInstanceDynamic* billBoardMat);
	
	//Setters of attributes within materials
	void SetMaterialTextureSize(int textureWidth, int textureHeight);
	void SetMaterialMovementPosition(const FTransform &transform);
	void SetActiveGrassBlades(EGPGrassShape shape);

	//Ray Setter
	void SetRayLength(int value) { rayLength = value; };

	//function raytraces position of terrain, then adjusts given position and normalQuat accordingly. Returns sucess of operation
	//@return param position - insert position of the object and returns adjusted position
	//@return param position - returns normal quaternion of the intersection 
	//@return - returns if ray got valid blocking hit
	int SnapingAdjustments(FVector& position, FQuat& normalQuat);

	void SetExperimentalLOD(bool value) { experimentalLOD = value; };
protected:
	
	int width = 5;
	int height = 30;
	int rayLength;
	bool experimentalLOD;
	   
	//Helper function to initialize meshes
	void InitiateMesh();

	UPROPERTY(EditAnywhere)
	UMaterialInstanceDynamic* dynMaterial;
	
	UPROPERTY(EditAnywhere)
	UMaterialInstanceDynamic* bbMat;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHierarchicalInstancedStaticMeshComponent* grassBlades;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHierarchicalInstancedStaticMeshComponent* triangleGrassBlades;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHierarchicalInstancedStaticMeshComponent* triangleQuadGrassBlades;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UHierarchicalInstancedStaticMeshComponent* activeGrassBladesInstances;

	//Billboard turfs 
	UPROPERTY(Editanywhere, BlueprintReadWrite)
	UHierarchicalInstancedStaticMeshComponent* billboardTurfInstances;
	
private:

	//Spawns the billboard instance on position
	void SpawnBillboardGrassTurf(FVector position, int radius, bool shouldSnapToTerrain, FQuat normalQuat);
	
	//Attribute setter for texture
	void SetupDefaultTexture(UTexture2D*& texture, int textureWidth, int textureHeight, EPixelFormat format);
	
	//Removes all instances of given instance manager
	void ClearHierarchicalInstances(UHierarchicalInstancedStaticMeshComponent* instances);
	
	//Helper function for Initialization of instance manager
	void InitiateHierarchicalInstanceMesh(UHierarchicalInstancedStaticMeshComponent* instances, FString meshLocation);

	//Sends ray up and bellow grass position and returns hit information if there are any
	void FindLandScapeRayTrace(FVector start, FVector end, FHitResult& hitResult);

	//if snap is turned on, ray searches for nearby terrain and if found adjusts given position and returns normal in the point found by ray
	int AdjustPosition(FVector& position, FVector& impactNormal);

	//Creates quaternion out of given normal
	FQuat FindQuatOfNormal(const FVector& upVector, const FVector& normal);

	//Generates random position ofseted from given position based on radius
	FVector GenRandomPositionWithinRad(FVector position, int radius, float precision);

	//
	void InitAllInstancesSelectability();
	void InitAllInstancesCollision();
	void InitShadowCast();
	void InitCullDistance();
};

