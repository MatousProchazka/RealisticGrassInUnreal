#pragma once
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

struct quadNode {
	float2 position;
	float2 topLeft, botRight; // OPTIM remove these and adjust algoritm
	struct quadNode *topLeftTree, *topRightTree, *botLeftTree, *botRightTree;
	bool realNode;
	__host__ __device__
	quadNode(float2 pos, float2 tL, float2 bR) {
		position = pos;
		topLeft = tL;
		botRight = bR;
		topLeftTree = NULL;
		topRightTree = NULL;
		botLeftTree = NULL;
		botRightTree = NULL;
		realNode = true;
	}
	__host__ __device__
	quadNode()
	{
		float2 z;
		position = z;
		topLeft = z;
		botRight = z;
		topLeftTree = NULL;
		topRightTree = NULL;
		botLeftTree = NULL;
		botRightTree = NULL;
		realNode = true;
	}

	//free up the pointers
	~quadNode() {
		/*if (topLeftTree) free(topLeftTree); if (topRightTree) free(topRightTree); if (botLeftTree) free(botLeftTree); if (botRightTree) free(botRightTree);*/
	}
};