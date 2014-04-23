/*
 * VDCRequestPartitioned.cpp
 *
 *  Created on: Dec 27, 2012
 *      Author: ahmedamokrane
 */

#include "VDCRequestPartitioned.h"



/*
VDCRequestPartitioned::VDCRequestPartitioned(int id, int nbPartitions) {
	idRequest = id;
	numberOfPartitions = nbPartitions;

	//Initialize bandwidth demands and delay constraints
	vector<double> bd[nbPartitions][nbPartitions];
	vector<double> dl[nbPartitions][nbPartitions];
	bandwidthDemand = bd;
	delayDemand = dl;
	partitions = vector();

}
 */

VDCRequestPartitioned::VDCRequestPartitioned(request_t* rqst) {
	//Initialize with the request
	originalRequest = rqst;
	numberOfPartitions = 0;
	idRequest = rqst->reqID;
	partitions = new vector<partition_t*>();
	//linkMapping = new vector< vector<mapping_virtual_link_to_substrate_path*>* >();

}



VDCRequestPartitioned::~VDCRequestPartitioned() {
	// TODO Auto-generated destructor stub

	for(unsigned int i=0; i<this->partitions->size(); i++){
		for(unsigned int j=0; j<this->partitions->size(); j++){
			if(i!=j){
				for(unsigned int k=0; k<linksBetweenPartirions[i][j]->size(); k++){
					link_t * l = linksBetweenPartirions[i][j]->at(k);
					if(l->mapping != NULL)
						delete l->mapping;
					//for(int m=0; m<l->numberOfMappings; m++){
						//delete l->mappings[m];
					//}
					delete linksBetweenPartirions[i][j]->at(k);
				}

				linksBetweenPartirions[i][j]->clear();
				delete linksBetweenPartirions[i][j];

				//bandwidthDemand[i][j]->clear();
				//delete bandwidthDemand[i][j];

				//delayDemand[i][j]->clear();
				//delete delayDemand[i][j];
			}
		}
	}

	for(unsigned int i=0; i<this->partitions->size(); i++){

		//Delete what is inside the partition
		//for(int j=0; j<this->partitions->at(i)->numberOfMappings; j++){
		//	delete this->partitions->at(i)->mappings[j];
		//}
		if(this->partitions->at(i)->mapping != NULL)
			delete this->partitions->at(i)->mapping;

		for(int j=0; j<this->partitions->at(i)->links; j++){
			link_t * l = this->partitions->at(i)->link[j];
			//for(int k=0; k<l->numberOfMappings; k++){
			//	delete l->mappings[k];
			//}
			if(l->mapping != NULL)
					delete l->mapping;
			delete this->partitions->at(i)->link[j];
		}

		delete this->partitions->at(i);
	}



	this->partitions->clear();
	delete this->partitions;

	for(int i=0; i<this->originalRequest->nodes; i++){
		delete this->originalRequest->nodeLocation[i];
	}

	/*
	for(int i=0; i<this->originalRequest->links; i++){
		link_t * l = this->originalRequest->link[i];
		for(int k=0; k<l->numberOfMappings; k++){
			delete l->mappings[k];
		}
		delete l;
	}
	*/

	delete this->originalRequest;

}


bool VDCRequestPartitioned::partitionRequestNoPartitioning(SubstrateInfrastructure substrateInf){
	//Partition the VDC request based on the incoming VDC request

	return true;
}



