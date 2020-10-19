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

#include "GrassBlade.h"


AGrassBlade::AGrassBlade(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{

	grassBlades = objectInitializer.CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(this, TEXT("grassInstances"));
	triangleGrassBlades = objectInitializer.CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(this, TEXT("triGrassInstances"));
	triangleQuadGrassBlades = objectInitializer.CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(this, TEXT("triQuadGrassInstances"));

	billboardTurfInstances = objectInitializer.CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(this, TEXT("billboardInstaces"));
	RootComponent = grassBlades;
	InitiateMesh();

	InitAllInstancesSelectability();
	InitCullDistance();
	InitShadowCast();

	InitAllInstancesCollision();
	
	SetActiveGrassBlades(EGPGrassShape::Quad);
}


void AGrassBlade::InitiateMesh()
{
	UGVar* configVars = UGVar::StaticClass()->GetDefaultObject<UGVar>();

	InitiateHierarchicalInstanceMesh(grassBlades, configVars->quadMeshLocation);
	InitiateHierarchicalInstanceMesh(triangleGrassBlades, configVars->triangleMeshLocation);
	InitiateHierarchicalInstanceMesh(triangleQuadGrassBlades, configVars->triangleQuadMeshLocation);

	InitiateHierarchicalInstanceMesh(billboardTurfInstances, configVars->billBoardMeshLocation);
}



void AGrassBlade::ClearInstances()
{
	ClearHierarchicalInstances(grassBlades);
	ClearHierarchicalInstances(triangleGrassBlades);
	ClearHierarchicalInstances(triangleQuadGrassBlades);

	ClearHierarchicalInstances(billboardTurfInstances);
}

void AGrassBlade::SpawnGrassBlades(int amount, int startIndex, FVector4 bounds, FVector patchPosition, FVector textureCorner, int textureWidth, bool shouldSnapToTerrain)
{
	for (int i = startIndex; i < startIndex + amount; i++) {
		uint16 randomAngle = FMath::RandRange(0, 359);
		FRotator bladeRotation(0, randomAngle, 0);
		FQuat bladeQ(bladeRotation);

		FRotator cubeRotation(0, randomAngle, 90);
		FQuat boxQ(cubeRotation);

		int randomX = FMath::RandRange(bounds.X, bounds.Z);
		int randomY = FMath::RandRange(bounds.Y, bounds.W);
		FVector bladePosition = patchPosition + FVector(randomX, randomY, 0);
		//setting index
		
		FTransform transform;
		if (shouldSnapToTerrain) {
			FQuat normalQuat;
			if (!SnapingAdjustments(bladePosition, normalQuat))
				continue;
			transform.SetRotation(normalQuat * bladeQ);
		}
		else
			transform.SetRotation(bladeQ);

		transform.SetLocation(bladePosition);
		
		int32 index = activeGrassBladesInstances->AddInstance(transform);
		int indexOfPos = 4*(i);
		//encoding sign of vector into uint
		int sign = HelperFunctions::CompressSignValues(bladePosition);
		//put position within array
	}
}

void AGrassBlade::SpawnGrassBladesAroundPosition(int amount, int radius, FVector position, bool shouldSnapToTerrain, FQuat normalQuat)
{
	//spawn of billboard garss turf
	if(!experimentalLOD)
		SpawnBillboardGrassTurf(position, radius, shouldSnapToTerrain, normalQuat);

	float precision = 1000;
	for (int i = 0; i < amount; i++) {
		FVector pos = GenRandomPositionWithinRad(position, radius, precision);
		uint16 randomAngle = FMath::RandRange(0, 359);
		uint16 randomSize = FMath::RandRange(1,2);
		FVector size = FVector(1,1,randomSize);
		FRotator bladeRotation(0, randomAngle, 0);
		FQuat bladeQ(bladeRotation);

		FRotator cubeRotation(0, randomAngle, 90);
		FQuat boxQ(cubeRotation);

		FTransform transform;
		
		if (shouldSnapToTerrain) {
			if (!SnapingAdjustments(pos, normalQuat))
				continue;
			transform.SetRotation(normalQuat * bladeQ);
		}
		else
			transform.SetRotation(bladeQ);
		if (pos == FVector::ZeroVector)
			continue;
		transform.SetLocation(pos);
		transform.SetScale3D(size);
		
		activeGrassBladesInstances->AddInstance(transform);

	}

}

void AGrassBlade::SpawnFlowersAroundPosition(FVector position, int minAmount, int maxAmount, float innerFlowerRadius, EGPFlower flowerKind, bool shouldSnapToTerrain, FQuat normalQuat, float spawnWeight)
{
	float weight = 2 - spawnWeight;
	int amount = floor(minAmount + (maxAmount - minAmount) * (pow(FMath::RandRange(0.f,1.f), weight)));

	float precision = 1000;
	for (int i = 0; i < amount; i++)
	{
		FVector pos = GenRandomPositionWithinRad(position, innerFlowerRadius, precision); 
		uint16 randomAngle = FMath::RandRange(0, 359);
		FRotator bladeRotation(0, randomAngle, 0);
		FQuat bladeQ(bladeRotation);
		
		FTransform transform;

		if (shouldSnapToTerrain) {
			if (!SnapingAdjustments(pos, normalQuat))
				continue;
			transform.SetRotation(normalQuat * bladeQ);
		}
		else
			transform.SetRotation(bladeQ);
		if (pos == FVector::ZeroVector)
			continue;
		transform.SetLocation(pos);
		transform.SetScale3D(FVector(1,1,1));
	}
}

void AGrassBlade::SpawnDefaultObject(int index)
{
	FTransform transform;
	transform.SetLocation(FVector(-5500, 1060, 0));
	activeGrassBladesInstances->AddInstance(transform);
	
}


void AGrassBlade::InitializeMaterials(UMaterialInstanceDynamic* material, UMaterialInstanceDynamic* billboardMat)
{
	dynMaterial = material;
	grassBlades->SetMaterial(0, material);
	triangleGrassBlades->SetMaterial(0, material);
	triangleQuadGrassBlades->SetMaterial(0, material);

	bbMat = billboardMat;
	billboardTurfInstances->SetMaterial(0, billboardMat);
	
}

void AGrassBlade::SetMaterialTextureSize(int textureWidth, int textureHeight)
{
	dynMaterial->SetScalarParameterValue(FName("textureWidth"), textureWidth);
	dynMaterial->SetScalarParameterValue(FName("textureHeight"), textureHeight);
	activeGrassBladesInstances->SetMaterial(0, dynMaterial);
}

void AGrassBlade::SetMaterialMovementPosition(const FTransform &transform)
{
	dynMaterial->SetVectorParameterValue(FName("movementPosition"), transform.GetLocation());
	activeGrassBladesInstances->SetMaterial(0, dynMaterial);
}


void AGrassBlade::SpawnBillboardGrassTurf(FVector position, int radius, bool shouldSnapToTerrain, FQuat normalQuat)
{
	uint16 randomAngle = FMath::RandRange(0, 359);
	uint16 randomSize = FMath::RandRange(1, 2);
	FVector size = FVector(1, 1, randomSize);
	FRotator bladeRotation(0, randomAngle, 0);
	FQuat bladeQ(bladeRotation);

	FRotator cubeRotation(0, randomAngle, 90);
	FQuat boxQ(cubeRotation);

	FTransform transform;

	if (shouldSnapToTerrain) {
		if (!SnapingAdjustments(position, normalQuat))
			return;
		transform.SetRotation(normalQuat * bladeQ);
	}
	else
		transform.SetRotation(bladeQ);
	if (position == FVector::ZeroVector)
		return;
	transform.SetLocation(position);
	transform.SetScale3D(size);

	billboardTurfInstances->AddInstance(transform);

}
void AGrassBlade::SetupDefaultTexture(UTexture2D*& texture, int textureWidth, int textureHeight, EPixelFormat format)
{
	texture->AddToRoot();
	texture->PlatformData = new FTexturePlatformData();
	texture->PlatformData->SizeX = textureWidth;
	texture->PlatformData->SizeY = textureHeight;
	texture->PlatformData->NumSlices = 1;
	texture->PlatformData->PixelFormat = EPixelFormat::PF_R8G8B8A8;
}


void AGrassBlade::SetActiveGrassBlades(EGPGrassShape shape)
{
	switch (shape) {
	case 0:
		activeGrassBladesInstances = grassBlades;
		break;
	case 1:
		activeGrassBladesInstances = triangleGrassBlades;
		break;
	case 2:
		activeGrassBladesInstances = triangleQuadGrassBlades;
		break;
	}
}

void AGrassBlade::ClearHierarchicalInstances(UHierarchicalInstancedStaticMeshComponent* instances) {
	if (instances != NULL)
		if (instances->GetNumRenderInstances() > 1)
			instances->ClearInstances();
}

void AGrassBlade::InitiateHierarchicalInstanceMesh(UHierarchicalInstancedStaticMeshComponent* instances, FString meshLocation) {
	UStaticMesh* staticMeshOb = LoadObject<UStaticMesh>(nullptr, *meshLocation);
	if (staticMeshOb == NULL)
		staticMeshOb = Cast<UStaticMesh>(FSoftObjectPath(meshLocation).TryLoad());

	instances->SetStaticMesh(staticMeshOb);
}

void AGrassBlade::FindLandScapeRayTrace(FVector start, FVector end, FHitResult & hitResult)
{
	ECollisionChannel colChannel = ECollisionChannel::ECC_WorldStatic;

	FCollisionQueryParams TraceParams(FName(TEXT("landscape trace")), true);

	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bFindInitialOverlaps = false;

	hitResult = FHitResult(ForceInit);

	
#if WITH_EDITOR
	UEditorEngine* editor = GEditor;
	UWorld* world = editor->GetLevelViewportClients()[0]->GetWorld();
#else
	UWorld* world = GetWorld();
#endif

	FVector st = start;

	while (st.Z > end.Z) {
		hitResult = FHitResult(ForceInit);
		world->LineTraceSingleByChannel(hitResult, st, end, colChannel, TraceParams);
		if (!hitResult.IsValidBlockingHit()) {
			break;
		}else if (GetNameSafe(hitResult.GetActor()).Contains(FString("Landscape"))) {
			break;
		}
		else if (IsValid(hitResult.GetActor())) {
			if (!hitResult.GetActor()->Tags.Contains(FName("grassEnable"))) {
				hitResult.Reset();

				break;
			}
			else
			{
				break;
			}
		}
		else {
			
			st = hitResult.Location + FVector(0,0,-1);
		}
	}
	
}

int AGrassBlade::AdjustPosition(FVector& position, FVector& impactNormal)
{
		FHitResult res;
		position = position + FVector(0, 0, rayLength / 2);
		FindLandScapeRayTrace(position, FVector(0, 0, -rayLength) + position, res);
		position = res.Location;
		impactNormal = res.Normal;
		return res.IsValidBlockingHit();
}

FQuat AGrassBlade::FindQuatOfNormal(const FVector & upVector, const FVector & normal)
{
	FVector rotationAxis = FVector::CrossProduct(upVector, normal);
	rotationAxis.Normalize();

	float dotproduct = FVector::DotProduct(upVector, normal);
	float rotationAngle = acosf(dotproduct);

	FQuat resultQuat = FQuat(rotationAxis, rotationAngle);

	return resultQuat;
}

FVector AGrassBlade::GenRandomPositionWithinRad(FVector position, int radius, float precision)
{
	float t = 2 * PI * (FMath::RandRange(0.f, precision) / precision);
	float r = radius * FMath::Square(FMath::RandRange(-radius * precision, radius * precision) / precision);
	return FVector(r * FMath::Cos(t), r * FMath::Sin(t), 0) + position;
}

void AGrassBlade::InitAllInstancesSelectability()
{
	billboardTurfInstances->bSelectable = false;
	grassBlades->bSelectable = false;
	triangleGrassBlades->bSelectable = false;
	triangleQuadGrassBlades->bSelectable = false;
}

void AGrassBlade::InitAllInstancesCollision()
{
	grassBlades->bDisableCollision = true;
	triangleGrassBlades->bDisableCollision = true;
	triangleQuadGrassBlades->bDisableCollision = true;
	billboardTurfInstances->bDisableCollision = true;
}

void AGrassBlade::InitShadowCast()
{
	grassBlades->SetCastShadow(false);
	triangleGrassBlades->SetCastShadow(false);
	triangleQuadGrassBlades->SetCastShadow(false);
	billboardTurfInstances->SetCastShadow(false);
}

void AGrassBlade::InitCullDistance()
{
	UGVar* vars = (UGVar*)NewObject<UGVar>(UGVar::StaticClass());
	int cullDistance = vars->detailedGrassCullDistance;

	grassBlades->InstanceEndCullDistance = cullDistance;
	triangleGrassBlades->InstanceEndCullDistance = cullDistance;
	triangleQuadGrassBlades->InstanceEndCullDistance = cullDistance;

	billboardTurfInstances->InstanceStartCullDistance = cullDistance;
	billboardTurfInstances->InstanceEndCullDistance = vars->lodCullDistanceFar;
}

int AGrassBlade::SnapingAdjustments(FVector & position, FQuat& normalQuat)
{
	FVector upVector = FVector(0, 0, 1);
	FVector normal = upVector;
	int output = AdjustPosition(position, normal);
	normalQuat = FindQuatOfNormal(upVector, normal);
	normalQuat.Normalize();
	return output;
}

