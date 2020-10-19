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
#include "GrassBlade.h"
#include "Array.h"
#include "Math.h"
#include "Runtime/Core/Public/Math/Quat.h"
#include "EngineUtils.h"
#include "vector.h"
#include "cuda_poisson_lib.h"
#include "GVar.h"


#include "windows.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "GrassRendering.generated.h"

 //Now becauase this is just a simulation of 2D array within Unreal environment, you need to be carefull with use
	 // Ti. While accessing 2D array by grid[x][y] is possible. Replacing values this way is not
USTRUCT()
struct F2DVectorArray {
	GENERATED_BODY()
public:
	TArray<FVector> array;

	FVector operator[] (int32 i) {
		return array[i];
	}

	void Add(FVector vec) {
		array.Add(vec);
	}

	void Replace(int32 i, FVector value) {
		if (array.IsValidIndex(i)) {
			array.RemoveAt(i, 1, false);
			array.Insert(value, i);
		}
	}
};
UCLASS()
class GRASSPLUGIN_API UGrassRendering : public UObject
{
	GENERATED_BODY()
public:
	UGrassRendering();
	
	// Used to be able to invoke it
#if WITH_EDITOR
	FEditorModeID editorMode;
#endif
	FSimpleDelegate OnNewMapLoad;
			
	//TopLeft and botRight determine the plane within Unreal in which the grass gets spawned, in default x goes from negative to positive
	//Created plane needs to be square
	//TopLeft corner of the space where the grass should get generated (x needs to be negative)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D topLeftCorner = FVector2D(-1000, 1000);

	//TopLeft and botRight determine the plane within Unreal in which the grass gets spawned, in default x goes from negative to positive
	//Created plane needs to be square
	//Bottom Right corner of the space where the grass should get generated (x needs to be positive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D botRightCorner = FVector2D(1000, -1000);
	
	//Turns on adaptive sampling based on given texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool adaptiveSampling = false;

	//Sets the density of grass spawned on positions coresponding to black pixels within the image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "adaptiveSampling"))
		int lowerThreshold = 10;
	//Sets the density of grass spawned on positions coresponding to white pixels within the image (aside from completelly white which will leave space empty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "adaptiveSampling"))
		int upperThreshold = 80;

	//Name of greyscale texture in .png format that should serve as a density texture for adaptive sampling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "adaptiveSampling"))
		FString pictureName = "snow_tigerG";

	//Allows to divide space on which we generate grass into multiple parts and generate only some
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "adaptiveSampling"))
		bool divideIntoSmaller;

	//Sets amount of parts the space should be divided into
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "adaptiveSampling"))
		int amountOfParts = 9;
	//Index of part of space that should be generated(0 to amountOfParts-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "adaptiveSampling", UIMin = 0, ClampMin = 0))
		int renderPart = 0;
	
	//Determines whether the ray should be send for every grass to be moved onto landscape/collision objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool shouldSnapToTerrain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "No LOD"))
	bool experimentalLODSystem = false;
	
	//Determines the default model used for the grass 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGPGrassShape grassShape = EGPGrassShape::Triangle;

	//Determines the length of ray being shot from grass blade in both directions (understand total distance, not just in one direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int rayLength = 1200;

	//In case of true, clears first the previous grass and then generates new one
	//In case of false, generates new grass while keeping the previously generated grass
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool overridePrevious = true;

	//*** POISSON DISK ATTRIBUTES ***//
	//Determines density of poisson disk sapling (higher number, less dense)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float turfRadius = 10;

	//Determines number of iterations needed to find other places within sapling (smaller is faster, but might leave spots empty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int poissonDiskTries = 30;

	//Radius of where can be grass generated around turf origin (generated by poisson sapling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float turfGrassRadius = 2;

	//Number of blades generated within turf
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int numOfBladesWithinTurf = 20;

	//Helper function for initializing of materials
	void InitMaterial(UMaterial*& mat, FString matLoc);

	//Main spawn function that computes poissonDisk distribution (regular/adapted) and spawns grass
	void SpawnGrassBladesInTurfs();
	
	//Changes the grass model on the fly based on the chosen variable
	void RefreshGrassMode();

	//Removes all the instances from grass instance managers
	void ClearGrass();

	//Checks whether GrassBlade instance manager is spawned within scene, and if not, it spawns one and sets the attributes properly
	void SpawnPatchIfNotSpawned();

protected:
	bool spawned = false;

	UMaterial* grassMaterial;
	UMaterial* billboardMaterial;
	
	AGrassBlade* grassPatch;
	// Divides the space into subspaces that can be handled by GPU
	//@return param positions - array of positions 
	//@param radius - max distance between positions
	//@param maxTries - max amount of attempts in dart throwing in poisson disk sampling
	//@param bounds - borders for sampling
	void PoissonDiskForWholeBoundaries(std::vector<float>& positions, const int radius, const int maxTries,
		const float bounds[]);
	//Poisson Disk adaptive sampling variation
	//@return param positions - array of positions 
	//@return param radiusValues - array of max distances between positions created based on an image
	//@return param imgH - height of image
	//@return param imgW - width of image
	//@param input - name of the input image (without the format sufix)
	//@param maxTries - max amount of attempts in dart throwing in poisson disk sampling
	//@param bounds - borders for sampling
	//@param lowerThreshold - radius for positions coresponding to black pixels
	//@param upperThreshold - radius for positions coresponding to white pixels (aside from complete white which remains empty)
	void PoissonDiskForWholeBoundaries(std::vector<float>& positions, unsigned char* &radiusValues, unsigned& imgW, unsigned& imgH, std::string input, const int maxTries,
		const float bounds[]);

	// Divides the space into subspaces that can be handled by GPU and only part on index is generated
	//for adaptive sampling only
	//@return param positions - array of positions 
	//@return param radiusValues - array of max distances between positions created based on an image
	//@return param imgH - height of image
	//@return param imgW - width of image
	//@param input - name of the input image (without the format sufix)
	//@param maxTries - max amount of attempts in dart throwing in poisson disk sampling
	//@param bounds - borders for sampling
	//@param lowerThreshold - radius for positions coresponding to black pixels
	//@param upperThreshold - radius for positions coresponding to white pixels (aside from complete white which remains empty)
	//@param index - index of part of the grass space that we want to generate
	void PoissonDiskForPart(std::vector<float>& positions, unsigned char* &radValues, unsigned& imgW, unsigned& imgH, std::string input, const int maxTries, const float bounds[],
		int index);

	// Computes optimal squares within the segment (based on set subSpaceMaxWidth), adjusting smaller dimension to
	// preserve subSquares
	//@return param width - input width of space (can be adjusted within function)
	//@return param height - input height of space (can be adjusted within function)  
	//@return param xSegments - amount of segments in x axis 
	//@return param ySegments - amount of segments in y axis 
	//@return param segmentSize - width/height(they are equal) of the segment
	int DetermineAmountOfSegments(float& width, float& height, int& xSegments, int& ySegments, float& segmentSize);

	// Recomputes subbounds for underlying segment 
	//@param bounds - borders of space
	//@return param subBounds - borders of computed subspace
	//@param xIdx - index of subspace on x axis
	//@param yIdx - index of subspace on y axis
	//@param xSegments - amount of segments in x axis
	//@param ySegments - amount of segments in y axis
	//@param segmentSize - size of segment
	void CreateSubBounds(const float bounds[], float subBounds[], int xIdx, int yIdx, int xSegments, int ySegments, float segmentSize);

	//creates a notification window about an error within Unreal
	//@param title - Title of the error message
	//@param message - message that gets written to user
	void GenerateErrorMessage(const FString& title, const FString& message);

	//creates a warning popup window (gives options Ok and Cancel)
	//@param title - Title of the warning message
	//@param message - message that gets written to user
	EAppReturnType::Type GenerateWarningMessage(const FString& title, const FString& message);

	//***Helper functions***

	//Generate positions in given spacial domain using class attributes
	//@return param positions - returns generated positions by poissonSampling
	//@return param radValues - if adaptiveSampling is turned on, returns pointer to image pixel array
	//@return param imgW - if adaptiveSampling is turned on, returns image width
	//@return param imgH - if adaptiveSampling is turned on, returns image height
	//@param bounds - determines spacial domain for which we want to generate positions
	int GeneratePositions(std::vector<float> &positions, unsigned char* &radValues, unsigned &imgW, unsigned &imgH, const float bounds[]);

	//Spawn one turf
	//@param xCoord - coordinates on x axis where to place center of turf
	//@param yCoord - coordinates on y axis where to place center of turf
	//@param radValues - pointer to array with image pixel values for adaptive sampling
	//@param imgW - image width used for adaptive sampling
	//@param imgH - image height used for adaptive sampling
	//@param bounds - determines spacial domain for which we want to generate positions
	int SpawnTurf(float xCoord, float yCoord, unsigned char* radValues, unsigned imgW, unsigned imgH, const float bounds[]);

	//Check that bounds are square
	int CheckBounds();

	//Check if amount of generated positions is within limit set in configuration file
	int CheckInstanceLimit(int amountOfPositions);

	//Checks if RAM barrier is set. If so checks that RAM performance doesnt go over set limit
	int CheckRAMLimit();

	int ComputeSegmentsVal(int oneDSize);

	float2 FormFloat2(float x, float y);
	float4 FormFloat4(float x, float y, float z, float w);
	
	unsigned long long getTotalSystemMemory()
	{
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);
		return status.ullTotalPhys;
	}

	//Functions to get information about system operations
	int GetTotalRAM();

	int GetAvailRAM();

	int GetTotalMemory();

	int GetAvailMemory();

//************************************************
//*********USABLE FUNCTIONS (not active)**********
//************************************************
	//crude and simple implementation of poisson disk sampling (not used anymore) and helper functions
	TArray<FVector> poissonDiskSampling(int radius, int k, FVector4 bounds);
	void InsertPointToGrid(FVector point, float cellsize, int gWidth, int gHeight);
	// Validity is determined by being within bounds and further than radius from neighbouring points within the grid 
	bool IsValidPoint(int cellsize, int gwidth, int gheight, FVector p, int radius, int width, int height);
	bool PointWithinBounds(FVector p, int width, int height);

public:
	//*** RANDOM DISTRIBUTION ATTRIBUTES ***//
//Square roots
/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int amountOfGrassSquareRooted = 150;*/

	//density (if poisson sampling not used)
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int density = 2;*/

	//Generates random x, y coordinates in bounds and spawns the grass there
	//Just simple temporary implementation (currently not usedf)
	void SpawnGrassBladesCompletelyRandDistr();

	//If true generated grass based on poisson disk otherwise just randomly
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool poissonDisk = true;
private:
	UPROPERTY()
	TArray<FVector> grid;

	
};
