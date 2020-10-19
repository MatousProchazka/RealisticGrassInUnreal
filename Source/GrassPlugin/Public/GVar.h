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
#include "Runtime/CoreUObject/Public/UObject/Object.h"

#include "GVar.generated.h"

UENUM()
enum class EGPGrassShape : uint8 {
	Quad,
	Triangle,
	TriangleQuad
};

//Mixed has to be at the end of the list
UENUM()
enum class EGPFlower : uint8 {
	Daisy,
	Viola,
	Mixed
};



UCLASS(Config = GrassPluginConfig)
class GRASSPLUGIN_API UGVar : public UObject
{
	GENERATED_BODY()

public:
	UGVar() {};
	FString configLoc = FPaths::ProjectDir() + FString("Saved/Config/Windows");
	FString configName = FString("GrassPluginConfig.ini");

	//Material locations
	UPROPERTY(Config, EditDefaultsOnly)
	FString grassMatLocation = FString("Material'/GrassPlugin/Collections/M_GrassMat.M_GrassMat'");
	UPROPERTY(Config, EditDefaultsOnly)
	FString billBoardMatLocation = FString("Material'/GrassPlugin/Collections/BillBoardMat.BillBoardMat'");

	//Mesh locations
	UPROPERTY(Config, EditDefaultsOnly)
	FString quadMeshLocation = FString("/GrassPlugin/Collections/GrassQuado.GrassQuado"); //start with / Game
	UPROPERTY(Config, EditDefaultsOnly)
	FString triangleMeshLocation = FString("/GrassPlugin/Collections/GrassTriangle.GrassTriangle"); //start with / Game
	UPROPERTY(Config, EditDefaultsOnly)
	FString triangleQuadMeshLocation = FString("/GrassPlugin/Collections/GrassQuadTriangle.GrassQuadTriangle"); //start with / Game
	UPROPERTY(Config, EditDefaultsOnly)
	FString billBoardMeshLocation = FString("/GrassPlugin/Collections/BillBoard.BillBoard");

	UPROPERTY(Config, EditDefaultsOnly)
	FString experimentalLODMatLocation = FString("/GrassPlugin/ExperimentalLOD/Plane2.Plane2");

	//Determines distance from camera at which will the detailed grass get culled
	UPROPERTY(Config, EditDefaultsOnly)
		int detailedGrassCullDistance = 1000;

	//Determines distance from camera at which will the grass LOD gets culled
	UPROPERTY(Config, EditDefaultsOnly)
		int lodCullDistanceFar = 40000;
	
	//Turns on(true)/off(false) RAM memory barrier
	UPROPERTY(Config, EditDefaultsOnly)
		bool memoryBarrier = true;

	//Setup the amount of Available Free RAM memory that needs to stay free
	UPROPERTY(Config, EditDefaultsOnly)
		int minMemoryRemaining = 3000;

	//Setup the amount of Avalable Free memory on GPU that needs to stay free
	//Unlike RAM, if GPU reaches full usage, Unreal usually auto crashes, therefore it is recomended to set barrier to reasonable value to prevent this
	UPROPERTY(Config, EditDefaultsOnly)
		int minGPUMemoryRemaining = 1000;

	// GPU limit of space on which the positions are generated
	// The higher the limit, the more demanding the algorithm is on GPU, but the faster it generates positions
	UPROPERTY(Config, EditDefaultsOnly)
		int subSpaceMaxWidth = 2000;


	// limit of amount of instances that will be generated with one sweep
	// The higher the limit, the more demanding the algorithm is on RAM, but faster it generates positions
	// If higher amount of positions is generated than this number, warning is generated giving user choice to continue or not
	UPROPERTY(Config, EditDefaultsOnly)
	int maxInstanceLimitPG = 2000000;
};
