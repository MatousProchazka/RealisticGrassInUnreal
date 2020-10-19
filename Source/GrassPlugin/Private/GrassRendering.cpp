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

#include "GrassRendering.h"


UGrassRendering::UGrassRendering()
{
	grassPatch = (AGrassBlade*) NewObject<AGrassBlade>();
	
	UGVar* configVars = UGVar::StaticClass()->GetDefaultObject<UGVar>();

	InitMaterial(grassMaterial, configVars->grassMatLocation);
	InitMaterial(billboardMaterial, configVars->billBoardMatLocation);	
}

void UGrassRendering::InitMaterial(UMaterial *& mat, FString matLoc)
{
	ConstructorHelpers::FObjectFinder<UMaterial> matFinder(*matLoc);
	if (matFinder.Succeeded())
	{
		mat = matFinder.Object;
	}
	else {
		UE_LOG(LogTemp, Display, TEXT("%s Material not found."), *matLoc);
	}
	UGVar* configVars = UGVar::StaticClass()->GetDefaultObject<UGVar>();
	configVars->SaveConfig();
}


void UGrassRendering::SpawnGrassBladesInTurfs()
{
	const float bounds[4] = { topLeftCorner.X, topLeftCorner.Y, botRightCorner.X, botRightCorner.Y };
	std::vector<float> poissonPos;
	unsigned char* radValues = 0;
	unsigned imgW = 0, imgH = 0;

	SpawnPatchIfNotSpawned();

	grassPatch->SetRayLength(rayLength);
	grassPatch->SetExperimentalLOD(experimentalLODSystem);
	
	if (!CheckBounds())
		return;
	
	if (!GeneratePositions(poissonPos, radValues, imgW, imgH, bounds))
		return;
	
	UE_LOG(LogTemp, Display, TEXT("size of array %i"), poissonPos.size()/2);
	if (!CheckInstanceLimit(poissonPos.size()))
		return;
		
	FScopedSlowTask loadingDialogForSpawn(
		poissonPos.size() / 2, NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning instances of grass"), true);
	loadingDialogForSpawn.MakeDialogDelayed(1, true, true);
	
	for (int i = 0; i < poissonPos.size(); i+=2)
	{
		loadingDialogForSpawn.EnterProgressFrame(
			1, NSLOCTEXT("GrassSpawn", "Spawning Grass", "Grass turfs are being generated."));

		if (!SpawnTurf(poissonPos[i], poissonPos[i + 1], radValues, imgW, imgH, bounds))
			return;

		if (GWarn->ReceivedUserCancel())
		{
			UE_LOG(LogTemp, Warning, TEXT("Generating of new grass interupted."));
			return;
		}

		if (!CheckRAMLimit())
			return;
	}
	if (radValues)
		free(radValues);
	
	UE_LOG(LogTemp, Display, TEXT("The Total RAM %i, available RAM %i"), GetTotalRAM(), GetAvailRAM());
}

void UGrassRendering::RefreshGrassMode()
{
	grassPatch->SetActiveGrassBlades(grassShape);
}

void UGrassRendering::ClearGrass()
{
	if (grassPatch != NULL)
		grassPatch->ClearInstances();
}


void UGrassRendering::SpawnPatchIfNotSpawned() {
	int count = 0;
#if WITH_EDITOR
	UEditorEngine* editor = GEditor;
	UWorld* world = editor->GetLevelViewportClients()[0]->GetWorld();
#else
	UWorld* world = GetWorld();
#endif

	for (TActorIterator<AGrassBlade> iterator(world); iterator; ++iterator)
	{
		count++;
	}
	if (count < 1)
	{
		FActorSpawnParameters spawnInfo;
		spawnInfo.bNoFail = true;
		grassPatch = world->SpawnActor<AGrassBlade>(FVector(0, 0, 0), FRotator(0, 0, 0), spawnInfo);
		grassPatch->SetActiveGrassBlades(grassShape);
		UMaterialInstanceDynamic* grassDynMaterial = UMaterialInstanceDynamic::Create(grassMaterial, this, FName("GrassInstances"));
		UMaterialInstanceDynamic* billboardDynMaterial = UMaterialInstanceDynamic::Create(billboardMaterial, this, FName("billboardInstaces"));
		grassPatch->InitializeMaterials(grassDynMaterial,billboardDynMaterial);
		grassPatch->SetMaterialTextureSize(abs(topLeftCorner.X - botRightCorner.X), abs(topLeftCorner.Y - botRightCorner.Y));
	}
}
#pragma optimize("", off)




void UGrassRendering::PoissonDiskForWholeBoundaries(std::vector<float>& positions, const int radius, const int maxTries,
	const float bounds[])
{
	float width = abs(bounds[0] - bounds[2]);
	float height = abs(bounds[1] - bounds[3]);
	int xSegments, ySegments;
	float segmentSize;
	DetermineAmountOfSegments(width, height, xSegments, ySegments, segmentSize);
	float subBounds[4];
	UE_LOG(LogTemp, Display, TEXT("width is %f height is %f, segments are %i and %i, segments size %f\n"), width, height,
		xSegments, ySegments, segmentSize);
	FScopedSlowTask loadingDialogForSpawn(
		xSegments * ySegments, NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning subSpaces of grass"), true);
	loadingDialogForSpawn.MakeDialogDelayed(1, true, true);

	for (int i = 0; i < xSegments; i++)
	{
		for (int j = 0; j < ySegments; j++)
		{
			loadingDialogForSpawn.EnterProgressFrame(
				1, FText::Format(NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning {0} subSpace of grass"),
					j + ySegments * i));
			CreateSubBounds(bounds, subBounds, i, j, xSegments, ySegments, segmentSize);
			cudaPoissonSampling::PoissonDiskDistribution(positions, radius, maxTries, subBounds);

			if (GWarn->ReceivedUserCancel())
			{
				UE_LOG(LogTemp, Warning,
					TEXT("Generating of new positions interupted. Grass will be generated only for positions "
						"generated until now./n"));
				return;
			}
		}
	}
}

void UGrassRendering::PoissonDiskForWholeBoundaries(std::vector<float>& positions, unsigned char* &radiusValues, unsigned& imgW, unsigned& imgH, std::string input,
	const int maxTries, const float bounds[])
{
	float width = abs(bounds[0] - bounds[2]);
	float height = abs(bounds[1] - bounds[3]);
	int xSegments, ySegments;
	float segmentSize;
	DetermineAmountOfSegments(width, height, xSegments, ySegments, segmentSize);
	float subBounds[4];
	UE_LOG(LogTemp, Error, TEXT("width is %f height is %f, segments are %i and %i, segments size %f\n"), width, height,
		xSegments, ySegments, segmentSize);
	FScopedSlowTask loadingDialogForSpawn(
		xSegments * ySegments, NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning subSpaces of grass"), true);
	loadingDialogForSpawn.MakeDialogDelayed(1, true, true);

	for (int i = 0; i < xSegments; i++)
	{
		for (int j = 0; j < ySegments; j++)
		{

			loadingDialogForSpawn.EnterProgressFrame(
				1, FText::Format(NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning {0} subScape of grass"),
					j + ySegments * i));
			CreateSubBounds(bounds, subBounds, i, j, xSegments, ySegments, segmentSize);
			cudaPoissonSampling::partitionAttributes partition = { xSegments, ySegments, i + xSegments * j };
			cudaPoissonSampling::PoissonDiskDistribution(positions, radiusValues, imgW, imgH, input, maxTries, subBounds, lowerThreshold,
				upperThreshold, partition);
			if (GWarn->ReceivedUserCancel())
			{
				UE_LOG(LogTemp, Warning,
					TEXT("Generating of new positions interupted. Grass will be generated only for positions "
						"generated until now./n"));
				return;
			}
		}
	}
}

void UGrassRendering::PoissonDiskForPart(std::vector<float>& positions, unsigned char* &radValues, unsigned& imgW, unsigned& imgH, std::string input, const int maxTries,
	const float bounds[], int index)
{
	float width = abs(bounds[0] - bounds[2]);
	float height = abs(bounds[1] - bounds[3]);
	int xSegments, ySegments;
	float segmentSize;
	DetermineAmountOfSegments(width, height, xSegments, ySegments, segmentSize);
	float subBounds[4];
	UE_LOG(LogTemp, Error, TEXT("width is %f height is %f, segments are %i and %i, segments size %f\n"), width, height,
		xSegments, ySegments, segmentSize);
	int totalSegments = xSegments * ySegments;
	if (totalSegments < amountOfParts)
		amountOfParts = totalSegments;
	float partSize = (float)totalSegments / (float)amountOfParts;
	int lowerIndex = FMath::RoundToInt(renderPart * partSize);
	int upperIndex = FMath::RoundToInt((renderPart + 1) * partSize);

	FScopedSlowTask loadingDialogForSpawn(
		upperIndex - lowerIndex, NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning subSpaces of grass"), true);
	loadingDialogForSpawn.MakeDialogDelayed(1, true, true);

	for (int k = lowerIndex; k < upperIndex; k++)
	{
		int i = k % xSegments;
		int j = k / xSegments;
		loadingDialogForSpawn.EnterProgressFrame(
			1, FText::Format(NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning {0} subScape of grass"), k));
		CreateSubBounds(bounds, subBounds, i, j, xSegments, ySegments, segmentSize);
		cudaPoissonSampling::partitionAttributes partition = { xSegments, ySegments, i + xSegments * j };
		cudaPoissonSampling::PoissonDiskDistribution(positions, radValues, imgW, imgH, input, maxTries, subBounds, lowerThreshold,
			upperThreshold, partition);
		if (GWarn->ReceivedUserCancel())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Generating of new positions interupted. Grass will be generated only for positions "
					"generated until now./n"));
			return;
		}
	}
}

int UGrassRendering::DetermineAmountOfSegments(float& width, float& height, int& xSegments, int& ySegments,
	float& segmentSize)
{

	if (height > width)
	{
		ySegments = ComputeSegmentsVal(height);
		segmentSize = height / ySegments;
		xSegments = ComputeSegmentsVal(width);
		width = segmentSize * xSegments;
	}
	else
	{
		xSegments = ComputeSegmentsVal(width);
		segmentSize = width / xSegments;
		ySegments = ComputeSegmentsVal(height);
		height = segmentSize * ySegments;
	}

	return 1;
}
void UGrassRendering::CreateSubBounds(const float bounds[], float subBounds[], int xIdx, int yIdx, int xSegments,
	int ySegments, float segmentSize)
{
	if (bounds[1] < bounds[3])
	{
		subBounds[0] = bounds[0] + xIdx * segmentSize;
		subBounds[1] = bounds[1] + yIdx * segmentSize;
		subBounds[2] = bounds[2] - (xSegments - 1 - xIdx) * segmentSize;
		subBounds[3] = bounds[3] - (ySegments - 1 - yIdx) * segmentSize;
	}
	else
	{
		subBounds[0] = bounds[0] + xIdx * segmentSize;
		subBounds[1] = bounds[1] - (ySegments - 1 - yIdx) * segmentSize;
		subBounds[2] = bounds[2] - (xSegments - 1 - xIdx) * segmentSize;
		subBounds[3] = bounds[3] + yIdx * segmentSize;
	}
}
void UGrassRendering::GenerateErrorMessage(const FString& title, const FString& message)
{
	FText fullTitle = FText::Format(NSLOCTEXT("HeatMaps", "File_Error", "{0}"), FText::AsCultureInvariant(title));
	UE_LOG(LogTemp, Error, TEXT("%s"), *message);
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(message), &fullTitle);
}

EAppReturnType::Type UGrassRendering::GenerateWarningMessage(const FString& title, const FString& message)
{
	FText fullTitle = FText::Format(NSLOCTEXT("GrassPlugin", "File_Warning", "{0}"), FText::AsCultureInvariant(title));
	UE_LOG(LogTemp, Warning, TEXT("%s"), *message);
	return FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(message), &fullTitle);
}

int UGrassRendering::GeneratePositions(std::vector<float>& positions, unsigned char *& radValues, unsigned & imgW, unsigned & imgH, const float bounds[])
{
	if (adaptiveSampling)
	{
		FString path = FPaths::ProjectPluginsDir() + FString("GrassPlugin/Content/Textures/") + pictureName + FString(".png");
		std::string input = std::string(TCHAR_TO_UTF8(*(path)));

		if (!FPaths::FileExists(path))
		{
			GenerateErrorMessage(FString("GrassPlugin"),
				FString("Image not found. Make sure the image file is in Texture folder."));
			return 0;
		}
		else
		{
			if (divideIntoSmaller)
				PoissonDiskForPart(positions, radValues, imgW, imgH, input, poissonDiskTries, bounds, renderPart);
			else
				PoissonDiskForWholeBoundaries(positions, radValues, imgW, imgH, input, poissonDiskTries, bounds);
		}
	}
	else
		PoissonDiskForWholeBoundaries(positions, turfRadius, poissonDiskTries, bounds);

	return 1;
}

int UGrassRendering::SpawnTurf(float xCoord, float yCoord, unsigned char* radValues, unsigned imgW, unsigned imgH, const float bounds[])
{
	FVector turfPosition;
	FQuat normalQuat;
	turfPosition = FVector(xCoord, yCoord, 0);
	normalQuat = FQuat::Identity;
	int rad = 0; //0 - 255
	if (adaptiveSampling)
	{
		float2 pos2D = FormFloat2(xCoord, yCoord);
		float4 boundaries = FormFloat4(bounds[0], bounds[1], bounds[2], bounds[3]);
		if (!cudaPoissonSampling::GetPixelValueOnPosition(radValues, imgW, imgH, pos2D, rad, boundaries, lowerThreshold,
			upperThreshold, { 1, 1, 0 }))
			return 0;
	}
	int adjustedNum = ((float)(256 - rad) / 255.f) * numOfBladesWithinTurf;
	int adjustedRad = ((float)(256 - rad) / 255.f) * 2.f + turfGrassRadius;

	int numOfGrass = adaptiveSampling ? adjustedNum : numOfBladesWithinTurf;
	int radOfTurf = adaptiveSampling ? adjustedRad : turfGrassRadius;
	grassPatch->SpawnGrassBladesAroundPosition(numOfGrass, radOfTurf, turfPosition, shouldSnapToTerrain, normalQuat);
	return 1;
}

int UGrassRendering::CheckBounds()
{
	int widthX = abs(topLeftCorner.X - botRightCorner.X);
	int widthY = abs(topLeftCorner.Y - botRightCorner.Y);

	if (widthX != widthY)
	{
		GenerateErrorMessage(FString("GrassPlugin"),
			FString("The inserted field ratio is not square! Please adjust it."));
		return 0;
	}
	return 1;
}

int UGrassRendering::CheckInstanceLimit(int amountOfPositions)
{
	UGVar* configVars = UGVar::StaticClass()->GetDefaultObject<UGVar>();

	if (amountOfPositions > configVars->maxInstanceLimitPG)
	{

		if (EAppReturnType::Yes !=
			GenerateWarningMessage(FString("GrassPlugin"),
				FString("Warning: Exceeding limit of %i objects. This might lead to memory "
					"overflow, possibly causing Unreal to crash. Do you wish to proceed?",
					configVars->maxInstanceLimitPG)))
			return 0;
	}
	return 1;
}

int UGrassRendering::CheckRAMLimit()
{
	UGVar* configVars = UGVar::StaticClass()->GetDefaultObject<UGVar>();
	if (configVars->memoryBarrier)
		if (GetAvailRAM() < configVars->minMemoryRemaining)
		{
			GenerateErrorMessage(FString("GrassPlugin"),
				FString("Available memory dropped under allowed capacity. Computation will be stopped."));
			return 0;
		}

	return 1;
}

int UGrassRendering::ComputeSegmentsVal(int oneDSize)
{
	UGVar* configVars = UGVar::StaticClass()->GetDefaultObject<UGVar>();

	int val = FMath::RoundToInt(oneDSize / configVars->subSpaceMaxWidth);
	return val < 1 ? 1 : val;
}

float2 UGrassRendering::FormFloat2(float x, float y)
{
	float2 result;
	result.x = x;
	result.y = y;
	return result;
}

float4 UGrassRendering::FormFloat4(float x, float y, float z, float w)
{
	float4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}

// get memory info...
int UGrassRendering::GetTotalRAM()
{
	int ret = 0;
#ifdef WIN32
	DWORD v = GetVersion();
	DWORD major = (DWORD)(LOBYTE(LOWORD(v)));
	DWORD minor = (DWORD)(HIBYTE(LOWORD(v)));
	DWORD build;
	if (v < 0x80000000) build = (DWORD)(HIWORD(v));
	else build = 0;

	// because compiler static links the function...
	BOOL(__stdcall*GMSEx)(LPMEMORYSTATUSEX) = 0;

	HINSTANCE hIL = LoadLibrary(L"kernel32.dll");
	GMSEx = (BOOL(__stdcall*)(LPMEMORYSTATUSEX))GetProcAddress(hIL, "GlobalMemoryStatusEx");

	if (GMSEx)
	{
		MEMORYSTATUSEX m;
		m.dwLength = sizeof(m);
		if (GMSEx(&m))
		{
			ret = (int)(m.ullTotalPhys >> 20);
		}
	}
	else
	{
		MEMORYSTATUS m;
		m.dwLength = sizeof(m);
		GlobalMemoryStatus(&m);
		ret = (int)(m.dwTotalPhys >> 20);
	}
#endif
	return ret;
}

int UGrassRendering::GetAvailRAM()
{
	int ret = 0;
#ifdef WIN32
	DWORD v = GetVersion();
	DWORD major = (DWORD)(LOBYTE(LOWORD(v)));
	DWORD minor = (DWORD)(HIBYTE(LOWORD(v)));
	DWORD build;
	if (v < 0x80000000) build = (DWORD)(HIWORD(v));
	else build = 0;

	// because compiler static links the function...
	BOOL(__stdcall*GMSEx)(LPMEMORYSTATUSEX) = 0;

	HINSTANCE hIL = LoadLibrary(L"kernel32.dll");
	GMSEx = (BOOL(__stdcall*)(LPMEMORYSTATUSEX))GetProcAddress(hIL, "GlobalMemoryStatusEx");

	if (GMSEx)
	{
		MEMORYSTATUSEX m;
		m.dwLength = sizeof(m);
		if (GMSEx(&m))
		{
			ret = (int)(m.ullAvailPhys >> 20);
		}
	}
	else
	{
		MEMORYSTATUS m;
		m.dwLength = sizeof(m);
		GlobalMemoryStatus(&m);
		ret = (int)(m.dwAvailPhys >> 20);
	}
#endif
	return ret;
}

int UGrassRendering::GetTotalMemory()
{
	int ret = 0;
#ifdef WIN32
	DWORD v = GetVersion();
	DWORD major = (DWORD)(LOBYTE(LOWORD(v)));
	DWORD minor = (DWORD)(HIBYTE(LOWORD(v)));
	DWORD build;
	if (v < 0x80000000) build = (DWORD)(HIWORD(v));
	else build = 0;

	// because compiler static links the function...
	BOOL(__stdcall*GMSEx)(LPMEMORYSTATUSEX) = 0;

	HINSTANCE hIL = LoadLibrary(L"kernel32.dll");
	GMSEx = (BOOL(__stdcall*)(LPMEMORYSTATUSEX))GetProcAddress(hIL, "GlobalMemoryStatusEx");

	if (GMSEx)
	{
		MEMORYSTATUSEX m;
		m.dwLength = sizeof(m);
		if (GMSEx(&m))
		{
			ret = (int)(m.ullTotalPhys >> 20) + (int)(m.ullTotalVirtual >> 20);
		}
	}
	else
	{
		MEMORYSTATUS m;
		m.dwLength = sizeof(m);
		GlobalMemoryStatus(&m);
		ret = (int)(m.dwTotalPhys >> 20) + (int)(m.dwTotalVirtual >> 20);
	}
#endif
	return ret;
}

int UGrassRendering::GetAvailMemory()
{
	int ret = 0;
#ifdef WIN32
	DWORD v = GetVersion();
	DWORD major = (DWORD)(LOBYTE(LOWORD(v)));
	DWORD minor = (DWORD)(HIBYTE(LOWORD(v)));
	DWORD build;
	if (v < 0x80000000) build = (DWORD)(HIWORD(v));
	else build = 0;

	// because compiler static links the function...
	BOOL(__stdcall*GMSEx)(LPMEMORYSTATUSEX) = 0;

	HINSTANCE hIL = LoadLibrary(L"kernel32.dll");
	GMSEx = (BOOL(__stdcall*)(LPMEMORYSTATUSEX))GetProcAddress(hIL, "GlobalMemoryStatusEx");

	if (GMSEx)
	{
		MEMORYSTATUSEX m;
		m.dwLength = sizeof(m);
		if (GMSEx(&m))
		{
			ret = (int)(m.ullAvailPhys >> 20) + (int)(m.ullAvailVirtual >> 20);
		}
	}
	else
	{
		MEMORYSTATUS m;
		m.dwLength = sizeof(m);
		GlobalMemoryStatus(&m);
		ret = (int)(m.dwAvailPhys >> 20) + (int)(m.dwAvailVirtual >> 20);
	}
#endif
	return ret;
}
//************************************************
//*********USABLE FUNCTIONS (not active)**********
//************************************************

void UGrassRendering::SpawnGrassBladesCompletelyRandDistr()
{
	SpawnPatchIfNotSpawned();
	grassPatch->SetRayLength(rayLength);

	FVector4 patchBounds = FVector4(topLeftCorner.X, topLeftCorner.Y, botRightCorner.X, botRightCorner.Y);

	int widthX = abs(topLeftCorner.X - botRightCorner.X);
	int widthY = abs(topLeftCorner.Y - botRightCorner.Y);

	int density = 2;
	int amountOfGrassSquareRooted = 150;
	float edgeIndex = ((float)density * 0.5) - 0.5;

	int patchNum = 1;
	FVector patchPosition = FVector(topLeftCorner.X + widthX, topLeftCorner.Y + widthY, 0);

	FScopedSlowTask loadingDialogForSpawn(density * density, NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning instances of grass"), true);
	loadingDialogForSpawn.MakeDialogDelayed(1, true, true);

	int textureSideSize = density * amountOfGrassSquareRooted;
	int textureSize = textureSideSize * textureSideSize;
	int amountOfGrass = FMath::Pow(amountOfGrassSquareRooted, 2);

	FVector textureCorner;

	int startIndex = 0;

	for (float i = -edgeIndex; i <= edgeIndex; i++) {
		UE_LOG(LogTemp, Display, TEXT("density %i densityDiv %f res %f"), density, (float)density*0.5, edgeIndex);
		for (float j = -edgeIndex; j <= edgeIndex; j++) {
			FVector offsetVector = FVector(i, j, 0);

			loadingDialogForSpawn.EnterProgressFrame(1, FText::Format(NSLOCTEXT("GrassSpawn", "Spawning Grass", "Spawning {0} patch of grass"), patchNum++));

			grassPatch->SpawnGrassBlades(amountOfGrass, startIndex, patchBounds, widthX * offsetVector + patchPosition, textureCorner, widthX * density, shouldSnapToTerrain);
			startIndex += amountOfGrass;
		}
	}

	grassPatch->SpawnDefaultObject(startIndex);
#if WITH_EDITOR
	UEditorEngine* editor = GEditor;
	UWorld* world = editor->GetLevelViewportClients()[0]->GetWorld();
#else
	UWorld* world = GetWorld();
#endif

	grassPatch->SetMaterialTextureSize(textureSideSize, textureSideSize);
	
	UMaterialInstanceDynamic* grassDynMaterial = UMaterialInstanceDynamic::Create(grassMaterial, this, FName("GrassInstances"));
}


TArray<FVector> UGrassRendering::poissonDiskSampling(int radius, int maxTries, const FVector4 bounds)
{
	/**Initialization of array**/
	int dimensions = 2;
	TArray<FVector> points = TArray<FVector>();

	TArray<FVector> active = TArray<FVector>();


	int width = abs(bounds.Z - bounds.X);
	int height = abs(bounds.W - bounds.Y);
	int cellsize = floor(radius / sqrt(dimensions));

	int ncells_width = floor(width / cellsize) + 1;
	int ncells_height = floor(height / cellsize) + 1;
	grid.Empty();

	for (int i = 0; i < ncells_width*ncells_height; i++) {
		grid.Add(FVector::ZeroVector);
	}

	//initial point
	int randomX = width / 2;
	int randomY = height / 2;
	FVector p0 = FVector(randomX, randomY, 0);
	InsertPointToGrid(p0, cellsize, ncells_width, ncells_height);
	int xindex = floor(p0.X / cellsize);
	int yindex = floor(p0.Y / cellsize);
	points.Add(p0);
	active.Add(p0);

	int iter = 0;

	//Active slow taks
	FScopedSlowTask poissonDiskST(2, NSLOCTEXT("PoissonDisk", "PoissonDiskGenerating", "Loading points for Poisson Disk"), true);
	poissonDiskST.MakeDialogDelayed(1, true, true);
	poissonDiskST.EnterProgressFrame(1, NSLOCTEXT("PoissonDisk", "PoissonDiskGenerating", "Loading points for Poisson Disk"));
	/**Computation of other points**/
	while (active.Num() > 0) {
		int random_index = int(FMath::RandRange(0, active.Num() - 1)); //?? possible issue with the array
		FVector p = active[random_index];
		bool found = false;
		for (int tries = 0; tries < maxTries; tries++) {
			float theta = FMath::RandRange(0, 359);
			float new_radius = FMath::RandRange(radius, 2 * radius);

			int pnewx = floor(p.X + new_radius * FMath::Cos(theta));
			int pnewy = floor(p.Y + new_radius * FMath::Sin(theta));
			if (pnewx < 0 || pnewx >= width || pnewy < 0 || pnewy >= height)
				continue;
			FVector pnew = FVector(pnewx, pnewy, 0);

			if (IsValidPoint(cellsize, ncells_width, ncells_height, pnew, radius, width, height))
			{
				points.Add(pnew);
				InsertPointToGrid(pnew, cellsize, ncells_width, ncells_height);
				active.Add(pnew);

				found = true;
			}
		}
		if (!found)
			active.RemoveAt(random_index, 1);
	}
	poissonDiskST.EnterProgressFrame(1, NSLOCTEXT("PoissonDisk", "PoissonDiskSample", "PoissonDiskSamplingFinished"));
	UE_LOG(LogTemp, Display, TEXT("points : %i"), points.Num());
	return points;
}
void UGrassRendering::InsertPointToGrid(FVector point, float cellsize, int gWidth, int gHeight) {
	int xindex = floor(point.X / cellsize);
	int yindex = floor(point.Y / cellsize);
	if (xindex < gWidth && xindex >= 0) {
		if (yindex < gHeight && yindex >= 0) {
			grid.RemoveAt(xindex + yindex * gWidth);
			grid.Insert(point, xindex + yindex * gWidth);// = point;
		}
	}
}

bool UGrassRendering::IsValidPoint(int cellsize, int gWidth, int gHeight, FVector p, int radius, int width, int height) {
	if (!PointWithinBounds(p, width, height))
		return false;

	int xindex = floor(p.X / cellsize);
	int yindex = floor(p.Y / cellsize);
	int i0 = FMath::Max(xindex - 1, 0);
	int i1 = FMath::Min(xindex + 1, gWidth - 1);
	int j0 = FMath::Max(yindex - 1, 0);
	int j1 = FMath::Min(yindex + 1, gHeight - 1);

	for (int i = i0; i <= i1; i++)
		for (int j = j0; j <= j1; j++) {
			if (grid[i + j * gWidth] != FVector::ZeroVector) {
				FVector distVec = grid[i + j * gWidth] - p;
				float length = distVec.Size();

				if (length < radius)
					return false;
			}
		}


	return true;
}

bool UGrassRendering::PointWithinBounds(FVector p, int width, int height) {
	if (p.X < 0 || p.X >= width || p.Y < 0 || p.Y >= height) {
		return false;
	}
	else
		return true;

}



#pragma optimize("", on)
