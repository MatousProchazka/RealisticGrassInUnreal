/**************************************************************************
 * Copyright (C) 1996-2020 Bohemia Interactive, a.s. All Rights Reserved.
 *
 * Owner: Matous
 **************************************************************************/

#pragma once

//#include <string>
//#include <thrust/system_error.h>
//#include <thrust/system/cuda/error.h>
#include "cuda_runtime.h"
#include "external/lodepng.h"

#include <vector>

#define BLOCK = 32;
//#define MAXGRIDSIZE = 65535;
//#define DIMENSION= 2;
#define TREENODESIZE = 3 * sizeof(float2) + 4 * sizeof(quadNode*);


#define DEBUG_FUNCTION(ans){if(CPLEnableDebug) ans;}
#define SOFT_DEBUG_FUNCTION(ans){if(CPLSoftEnableDebug) ans;}
#define DEBUG_TEXT(text, ...){if(CPLEnableDebug) printf(text, ##__VA_ARGS__);}
#define SOFT_DEBUG_TEXT(text, ...){if (CPLSoftEnableDebug || CPLEnableDebug) printf(text, ##__VA_ARGS__);}

#define LOWERTHRESHHOLD 100
#define HIGHERTHRESHHOLD 1000

__constant__ 
//int treeNodeSize = 3 * sizeof(float2) + 4 * sizeof(quadNode*);
const bool CPLEnableDebug = false;
const bool CPLSoftEnableDebug = false;

// parallel poisson distribution in 2D
//
// potential improvements
//	- add function for radius as an input - will allow adaptive sampling
// Computes each tree level nodes in parallel
// Creates quad tree.
// Cycles through the levels of tree until the tree level remains empty instead of being subdivided.
// In every cycle we increase current level, compute phase groups, for each group execute cuda_SampleThrow
// and then add newly created nodes into tree.  
// 
// @positions is output of the function (random positions)
// @radius is minimal distance between points, determines density of points
// @trials is maximal number of attempts for dart throwing 
// @bounds is spacial domain of algorithm in 2 dimensions
// cudaError_t - serves for debuggin purposes of CUDA
//
// There are two versions of function available first takes constant radius, second loads given texture and based on its greyscale values 
// determines the radius for specific location within the bounds1

namespace cudaPoissonSampling {
	struct partitionAttributes {
		int widthPartitions;
		int heightPartitions;
		int partitionIdx;
	};

	cudaError_t PoissonDiskDistribution(std::vector<float>& positions, const int radius, const int maxTries, const float bounds[]);
	//cudaError_t PoissonDiskDistribution(std::vector<float>& positions, const std::string textureLocation, const int maxTries, const float bounds[], int lowerThreshold = 100, int upperThreshold = 1000, partitionAttributes partition = {1,1,0});
	cudaError_t PoissonDiskDistribution(std::vector<float>& positions, unsigned char* &radiusValues, unsigned& width, unsigned& height, const std::string textureLocation, const int maxTries, const float bounds[], int lowerThreshold = 100, int upperThreshold = 1000, partitionAttributes partition = {1,1,0});

	//Used to find pixel value of greyscale image, that is on the given position, process converts size of image to size of current level of quadNode tree 
	__host__ __device__
		int GetPixelValueOnPosition(unsigned char* image, int imgWidth, int imgHeight, float2 position, int &resultPosition, float4 bounds, int lowerThreshold, int upperThreshold, cudaPoissonSampling::partitionAttributes partition);

}




