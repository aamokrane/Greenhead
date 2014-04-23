/*
 * EmbeddingSolution.h
 *
 *  Created on: May 23, 2013
 *      Author: ahmedamokrane
 */
#include<vector>
#include "Embed.h"
#include "VDCRequestPartitioned.h"

#ifndef EMBEDDINGSOLUTION_H_
#define EMBEDDINGSOLUTION_H_

/*
 *
 */
class EmbeddingSolution {
public:

	VDCRequestPartitioned* requestsPartitioned;
	vector<int> assignedPartitionToDatacenter; //For every partition i, assignedPartitionToDatacenter[i] gives the datacenter to which it has been assigned
	int nbPartitionsAssignedSoFar;
	int numberOfDatacenters;
	EmbeddingSolution(VDCRequestPartitioned* vdcRequestsPartitioned, int nbDatacenters);
	EmbeddingSolution(EmbeddingSolution* solutionToExtend);
	virtual ~EmbeddingSolution();
};

#endif /* EMBEDDINGSOLUTION_H_ */
