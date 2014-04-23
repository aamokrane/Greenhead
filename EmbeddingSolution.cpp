/*
 * EmbeddingSolution.cpp
 *
 *  Created on: May 23, 2013
 *      Author: ahmedamokrane
 */

#include "EmbeddingSolution.h"




EmbeddingSolution::EmbeddingSolution(VDCRequestPartitioned* vdcRequestsPartitioned, int nbDatacenters){
	this->requestsPartitioned = vdcRequestsPartitioned;
	this->assignedPartitionToDatacenter = vector<int>(this->requestsPartitioned->numberOfPartitions);
	for(int i=0; i<this->requestsPartitioned->numberOfPartitions; i++){
		this->assignedPartitionToDatacenter[i] = -1;
	}
	this->nbPartitionsAssignedSoFar = 0;
	this->numberOfDatacenters = nbDatacenters;
}


EmbeddingSolution::EmbeddingSolution(EmbeddingSolution* solutionToExtend){
	this->requestsPartitioned = solutionToExtend->requestsPartitioned;
	this->assignedPartitionToDatacenter = vector<int>(this->requestsPartitioned->numberOfPartitions);
	for(int i=0; i<this->requestsPartitioned->numberOfPartitions; i++){
			this->assignedPartitionToDatacenter[i] = solutionToExtend->assignedPartitionToDatacenter[i];
	}
	this->nbPartitionsAssignedSoFar = solutionToExtend->nbPartitionsAssignedSoFar;
	this->numberOfDatacenters = solutionToExtend->numberOfDatacenters;
}


EmbeddingSolution::~EmbeddingSolution() {

}

