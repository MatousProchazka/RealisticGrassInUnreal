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

#if WITH_EDITOR
#include "GrassPluginEdModeDetails.h"

#define LOCTEXT_NAMESPACE "FGrassPluginDetails"

TSharedRef<IDetailCustomization> FGrassPluginEdModeDetails::MakeInstance()
{
	return MakeShareable(new FGrassPluginEdModeDetails);
}

void FGrassPluginEdModeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> outObjects;
	DetailBuilder.GetObjectsBeingCustomized(outObjects);
	if (outObjects.Num() > 0)
		FGrassPluginEdModeDetails::ObjectBeingCustomized = outObjects[0];

	IDetailCategoryBuilder& GeneralSettingsCategory = DetailBuilder.EditCategory("General Settings");
	IDetailCategoryBuilder& GeneralPoissonCategory = DetailBuilder.EditCategory("General Poisson Disk Settings");
	IDetailCategoryBuilder& PoissonDiskCategory = DetailBuilder.EditCategory("Regular Poisson Sampling");
	IDetailCategoryBuilder& AdaptivePoissonCategory = DetailBuilder.EditCategory("Adaptive Poisson Sampling");

	//general settings
	TSharedRef<IPropertyHandle> poissonDiskBool = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, poissonDisk));
	TSharedRef<IPropertyHandle> shouldSnapToTer = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, shouldSnapToTerrain));
	TSharedRef<IPropertyHandle> lengthOfRay = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, rayLength));
	TSharedRef<IPropertyHandle> grassBShape = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, grassShape));
	TSharedRef<IPropertyHandle> overridePrev =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, overridePrevious));
	TSharedRef<IPropertyHandle> experLOD = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, experimentalLODSystem));

	//poissonDisk sampling settings
	TSharedRef<IPropertyHandle> topLeft = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, topLeftCorner));
	TSharedRef<IPropertyHandle> botRight = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, botRightCorner));
	TSharedRef<IPropertyHandle> poissonDiskTry = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, poissonDiskTries));
	TSharedRef<IPropertyHandle> radiusOfTurf = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, turfGrassRadius));
	TSharedRef<IPropertyHandle> turfDensity = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, numOfBladesWithinTurf));
	TSharedRef<IPropertyHandle> adaptiveSamplingOn = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, adaptiveSampling));

	//atributes specific to adaptive sampling
	TSharedRef<IPropertyHandle> lowerThresh = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, lowerThreshold));
	TSharedRef<IPropertyHandle> upperThresh = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, upperThreshold));
	TSharedRef<IPropertyHandle> picName = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, pictureName));
	TSharedRef<IPropertyHandle> div = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, divideIntoSmaller));
	TSharedRef<IPropertyHandle> partAm = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, amountOfParts));
	TSharedRef<IPropertyHandle> part = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, renderPart));
	
	//attributes specific to regular sampling
	TSharedRef<IPropertyHandle> poissonRadius = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UGrassRendering, turfRadius));
	   	 
	// clang-format off

	GeneralSettingsCategory.AddProperty(poissonDiskBool);
	GeneralSettingsCategory.AddProperty(shouldSnapToTer);
	GeneralSettingsCategory.AddProperty(lengthOfRay);
	GeneralSettingsCategory.AddProperty(grassBShape);
	GeneralSettingsCategory.AddProperty(overridePrev);
	GeneralSettingsCategory.AddProperty(experLOD);

	GeneralPoissonCategory.AddProperty(topLeft);
	GeneralPoissonCategory.AddProperty(botRight);
	GeneralPoissonCategory.AddProperty(poissonDiskTry);
	GeneralPoissonCategory.AddProperty(radiusOfTurf);
	GeneralPoissonCategory.AddProperty(turfDensity);

	PoissonDiskCategory.AddProperty(poissonRadius);


	AdaptivePoissonCategory.AddProperty(adaptiveSamplingOn);
	AdaptivePoissonCategory.AddProperty(lowerThresh);
	AdaptivePoissonCategory.AddProperty(upperThresh);
	AdaptivePoissonCategory.AddProperty(picName);
	AdaptivePoissonCategory.AddProperty(div);
	AdaptivePoissonCategory.AddProperty(partAm);
	AdaptivePoissonCategory.AddProperty(part);
	// clang-format on

}
#endif