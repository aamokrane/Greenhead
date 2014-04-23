/*
 * BranchAndBound.cpp
 *
 *  Created on: May 23, 2013
 *      Author: ahmedamokrane
 */

#include "BranchAndBound.h"

BranchAndBound::BranchAndBound() {
	// TODO Auto-generated constructor stub

}

vector<EmbeddingSolution*>* BranchAndBound::generateAllPossibleSolutions(
		VDCRequestPartitioned* vdcRequestsPartitioned, SubstrateInfrastructure* instrastructure) {
	vector<EmbeddingSolution*>* result = new vector<EmbeddingSolution*>();
	int nbDatacenters = instrastructure->numberOfDatacenters;
	EmbeddingSolution* sol = new EmbeddingSolution(vdcRequestsPartitioned, nbDatacenters);

	vector<EmbeddingSolution*> queueForSolutions = vector<EmbeddingSolution*>();
	vector<EmbeddingSolution*>* listOfPossibleSolutions = new vector<EmbeddingSolution*>();
	queueForSolutions.push_back(sol);

	while(!queueForSolutions.empty()){
		EmbeddingSolution* solFromQueue = queueForSolutions.back();
		int positionToErase = queueForSolutions.size() - 1;
		for(int i=0; i<nbDatacenters; i++){
			EmbeddingSolution* solExtended = new EmbeddingSolution(solFromQueue);

			//Add here the possibility of embedding based on the location constraints
			//If the datacenter
			if(!instrastructure->isPossibleToEmbedPartitionInDataCenter(vdcRequestsPartitioned->partitions->at(solExtended->nbPartitionsAssignedSoFar), i)){
				continue;
			}

			solExtended->assignedPartitionToDatacenter[solExtended->nbPartitionsAssignedSoFar] = i;
			solExtended->nbPartitionsAssignedSoFar++;

			if(solExtended->nbPartitionsAssignedSoFar == solExtended->requestsPartitioned->numberOfPartitions){
				//This is one complete possible solution
				listOfPossibleSolutions->push_back(solExtended);
			}else{
				//This solution is not complete
				queueForSolutions.push_back(solExtended);
			}
		}

		queueForSolutions.erase(queueForSolutions.begin() + positionToErase);
		delete solFromQueue;
	}

	return listOfPossibleSolutions;

}

BranchAndBound::~BranchAndBound() {
	// TODO Auto-generated destructor stub
}

