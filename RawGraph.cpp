/*
 * RawGraph.cpp
 *
 *  Created on: Jan 11, 2013
 *      Author: ahmed
 */

#include "RawGraph.h"

RawGraph::RawGraph(request_t* request) {

	this->nbNodes = request->nodes;
	this->nbPartitions = request->nodes;
	this->VDCRequest = request;
	this->sumAllWeights = 0.0;

	this->nodes = new vector<int>();
	this->locationConstraint = vector<location_t*>();
	this->weightsIn = new vector<double>();

	links = new vector<vector<vector<double>* >* >();
	this->node2partitions = new vector<int>();

	//Build the list of nodes
	for(int i=0; i<this->nbNodes; i++){
		this->node2partitions->push_back(i);
		this->nodes->push_back(request->nodesID[i]);
		this->locationConstraint.push_back(request->nodeLocation[i]);
		this->weightsIn->push_back(0);
		vector<vector<double>* >* col = new vector<vector<double>* >();
		for(int i=0; i<this->nbNodes; i++){
			vector<double>* ll = new vector<double>();
			col->push_back(ll);
		}
		links->push_back(col);
	}

	//Build the list of links
	for(int i=0; i<request->links; i++){
		int source = request->link[i]->nodeFrom;
		int dest = request->link[i]->nodeTo;
		double weig =  request->link[i]->bw;
		this->links->at(source)->at(dest)->push_back(weig);
		this->links->at(dest)->at(source)->push_back(weig);
		this->weightsIn->at(source) += weig;
		this->weightsIn->at(dest) += weig;
		this->sumAllWeights += 2*weig;// a link in the request is counted only once
	}

	this->modularity = this->computeModularity();
}



RawGraph::~RawGraph() {
	// TODO Auto-generated destructor stub
	//delete this->VDCRequest;
	//vector<vector<vector<double>* >* >* links;

	for(unsigned int i=0; i<this->links->size(); i++){
		for(unsigned int j=0; j<this->links->at(i)->size(); j++){
			this->links->at(i)->at(j)->clear();
		}
	}
	this->links->clear();
	delete this->links;
	//vector<location_t*>* locationConstraint;
	//for(unsigned int i=0; i<this->locationConstraint->size(); i++){
	//	delete this->locationConstraint->at(i);
	//}
	//this->locationConstraint->clear();
	//delete this->locationConstraint;

	this->node2partitions->clear();
	delete this->node2partitions;

	this->weightsIn->clear();
	delete this->weightsIn;

}


double RawGraph::computeModularity(){
	double mod = 0.0;

	for(int i=0; i<this->nbNodes; i++){
		for(int j=0; j<this->nbNodes; j++){
			if(this->node2partitions->at(i) == this->node2partitions->at(j)){
				double weigIJ = this->getWeight(i,j);
				mod += (weigIJ - ( (this->weightsIn->at(i)*this->weightsIn->at(j)) / (2*this->sumAllWeights)));
			}
		}
	}
	return mod/(2*this->sumAllWeights);
}


double RawGraph::getWeight(int nodei, int nodej){
	double w = 0.0;
	for(unsigned int i=0; i<this->links->at(nodei)->at(nodej)->size(); i++){
		w += this->links->at(nodei)->at(nodej)->at(i);
	}
	return w;
}

double RawGraph::computeModularityWhenMovingNode(int node, int partitionSource, int partitionDestination){

	double mod;
	this->node2partitions->at(node) = partitionDestination;
	mod = this->computeModularity();
	this->node2partitions->at(node) = partitionSource;
	return mod;
}


VDCRequestPartitioned* RawGraph::partitionUsingLouvainAlgprithm(){

	//Build the VDC partitioned request, the result to return
	bool improve = true;

	while(improve){
		improve = false;

		for(int i = 0; i<this->nbNodes; i++){
			int j = 0;
			while(j<this->nbPartitions){

				int partitionNodei = this->node2partitions->at(i);
				int newPartitionNodei = (int)j;

				if(partitionNodei != newPartitionNodei){
					//Verify the constraint of node location here, if not possible because there is a node that cannot be put with this one,
					//skip and go to the next one
					bool movePossible = true;
					if(this->locationConstraint.at(i)->xLocation > -1){
						for(int k = 0; k<this->nbNodes; k++){
							if(k != i &&  this->node2partitions->at(k) == newPartitionNodei && this->locationConstraint.at(k)->xLocation > -1){
								//There is a VM that has a location constraint here
								if((this->locationConstraint.at(k)->xLocation != this->locationConstraint.at(i)->xLocation) || (this->locationConstraint.at(k)->yLocation != this->locationConstraint.at(i)->yLocation)){
									movePossible = false;
									break;
								}
							}
						}
					}

					if(movePossible){
						//If there is no location constraint on both of them then
						//It is possible to move nodei to this partition j
						double modu = this->computeModularityWhenMovingNode(i,partitionNodei, newPartitionNodei);

						if(this->modularity < modu){
							//There is an improvement in the modularity

							improve = true;
							//Move this node to the new partition
							this->modularity = modu;
							//We are moving nodei to a new partition
							this->node2partitions->at(i) = newPartitionNodei;
							bool remove = true;
							for(int k = 0; k<this->nbNodes; k++){
								if(this->node2partitions->at(k) == partitionNodei){
									remove = false;
									break;
								}
							}
							if(remove){
								this->nbPartitions--;
								for(int k = 0; k<this->nbNodes; k++){
									if(this->node2partitions->at(k) > partitionNodei){
										this->node2partitions->at(k)--;
									}
								}
								j--;
							}

						}//END if(this->modularity < modu)
						else{
							//There is no improvement in the modularity, do nothing

						}
					}
				}else{
					//This node is in the same partition, nothing to do

				}//END IF ELSE (partitionNodei != newPartitionNodei)

				j++;
			}//END WHILE NUMBER OF PARTITIONS
		}

	}//END WHILE IMPROVE

	VDCRequestPartitioned* result = this->getVDCRequestPartitioned();
	return result;

}

VDCRequestPartitioned* RawGraph::partitionIntoEqualPartitionsAlgorithmWithoutLocationConstraints(int nbPartitions) {

	this->nbPartitions = nbPartitions;
	int currentPartition = 0;

	for(int i = 0; i< this->VDCRequest->nodes; i++){
		//We should pay attention to the location constraints, not addressed in this version
		this->node2partitions->at(i) = currentPartition;
		currentPartition++;
		if(currentPartition == nbPartitions){
			currentPartition = 0;
		}
	}

	VDCRequestPartitioned* result = this->getVDCRequestPartitioned();
	return result;
}

VDCRequestPartitioned* RawGraph::partitionIntoEqualPartitionsAlgorithmConsideringLocationConstraints(int nbPartitions) {

	this->nbPartitions = nbPartitions;
	int assignedVMsToPartitions[nbPartitions];

	for(int i = 0; i< nbPartitions; i++){
		assignedVMsToPartitions[i] = 0;
	}

	for(int i = 0; i< this->VDCRequest->nodes; i++){

		//Check if there is already a partition with the same location constraint
		bool done = false;
		if(this->locationConstraint.at(i)->xLocation > -1){

			for(int k = 0; k<i; k++){
				if(this->locationConstraint.at(k)->xLocation > -1){
					//There is a VM that has a location constraint here
					if((this->locationConstraint.at(k)->xLocation == this->locationConstraint.at(i)->xLocation) && (this->locationConstraint.at(k)->yLocation == this->locationConstraint.at(i)->yLocation)){
						done = true;
						int currentPartition = this->node2partitions->at(k);
						this->node2partitions->at(i) = currentPartition;
						assignedVMsToPartitions[currentPartition]++;
						break;
					}
				}
			}
		}
		if(done) continue;


		//Build the list of partitions ranked according to their size
		vector<int> partitions = vector<int>(nbPartitions);
		for(int j = 0; j< nbPartitions; j++){
			partitions[j] = j;
		}
		//rank the partitions to have the partition with the smallest number of nodes
		vector<int> partitionRanking = vector<int>();
		while(partitions.size()>0){
			int min = assignedVMsToPartitions[partitions[0]];
			int indexMin = 0;
			for(unsigned int l = 1; l< partitions.size(); l++){
				if(assignedVMsToPartitions[partitions[l]] < min){
					min = assignedVMsToPartitions[partitions[l]];
					indexMin = l;
				}
			}
			partitionRanking.push_back(partitions[indexMin]);
			partitions.erase(partitions.begin()+indexMin);
		}

		int currentPartition = partitionRanking.at(0);
		//int currentPartition = -1;
		//Choose the partition that is the least loaded and that satisfies the location constraint
		for(unsigned int j = 0; j< partitionRanking.size(); j++){
			bool doneForVMI = true;
			if(this->locationConstraint.at(i)->xLocation > -1){
				for(int k = 0; k<i; k++){
					if(this->node2partitions->at(k) == partitionRanking[j] && this->locationConstraint.at(k)->xLocation > -1){
						//There is a VM that has a location constraint here
						if((this->locationConstraint.at(k)->xLocation != this->locationConstraint.at(i)->xLocation) || (this->locationConstraint.at(k)->yLocation != this->locationConstraint.at(i)->yLocation)){
							doneForVMI = false;
							break;
						}
					}
				}
			}
			if(doneForVMI){
				currentPartition = partitionRanking[j];
				break;
			}
		}
		if(currentPartition == -1){
			double x = -3;
		}
		this->node2partitions->at(i) = currentPartition;
		assignedVMsToPartitions[currentPartition]++;

	}//END FOR I ON this->VDCRequest->nodes

	VDCRequestPartitioned* result = this->getVDCRequestPartitioned();
	return result;

}



VDCRequestPartitioned* RawGraph::partitionToGetOneSinglePartition(){
	this->nbPartitions = 1;
	int currentPartition = 0;
	for(int i = 0; i< this->VDCRequest->nodes; i++){
		//We should pay attention to the location constraints, not addressed in this version
		this->node2partitions->at(i) = currentPartition;
	}
	VDCRequestPartitioned* result = this->getVDCRequestPartitioned();
	return result;
}






VDCRequestPartitioned* RawGraph::getVDCRequestPartitioned(){
	VDCRequestPartitioned* result = new VDCRequestPartitioned(this->VDCRequest);
	result->numberOfPartitions = this->nbPartitions;
	//Create the partitions

	int partitions=0;
	//Create the set of partitions
	for(int i=0; i<result->numberOfPartitions; i++){
		partition_t* part = new partition_t;
		part->reqID = this->VDCRequest->reqID;
		part->request = this->VDCRequest;
		part->partitionID = i;
		strcpy(part->VDCName,this->VDCRequest->VDCName);
		strcpy(part->userName,this->VDCRequest->userName);
		strcpy(part->sessionName,this->VDCRequest->sessionName);
		strcpy(part->applicationName,this->VDCRequest->applicationName);
		part->availability = this->VDCRequest->availability;
		//part->numberOfMappings = 0;
		mapping_parition_to_datacenter* map = new mapping_parition_to_datacenter;
		map->datacenterID = -1;
		map->mapBegin = -1;
		map->partitioID =  i;
		map->requestID = part->reqID;
		part->mapping = map;
		part->nodes = 0;
		part->links = 0;
		part->migrationCost = 0;
		part->numberOfMigrations = 0;
		result->partitions->push_back(part);
		for(int j=i+1; j<result->numberOfPartitions; j++){
			result->linksBetweenPartirions[i][j] = new vector<link_t*>();
			result->linksBetweenPartirions[j][i] = new vector<link_t*>();
		}
	}
	//Add the nodes to the partitions
	for(int i=0; i<this->nbNodes; i++){
		int index = result->partitions->at(this->node2partitions->at(i))->nodes;
		result->partitions->at(this->node2partitions->at(i))->nodesID[index] = this->nodes->at(i);
		result->partitions->at(this->node2partitions->at(i))->cpu[index] = this->VDCRequest->cpu[i];
		result->partitions->at(this->node2partitions->at(i))->mem[index] = this->VDCRequest->mem[i];
		result->partitions->at(this->node2partitions->at(i))->flavor[index] = this->VDCRequest->flavor[i];
		result->partitions->at(this->node2partitions->at(i))->disk[index] = this->VDCRequest->disk[i];
		result->partitions->at(this->node2partitions->at(i))->nodeLocation[index] = this->VDCRequest->nodeLocation[i];
		result->partitions->at(this->node2partitions->at(i))->migrationCost += this->VDCRequest->migrationCostOfVM[i];
		result->partitions->at(this->node2partitions->at(i))->nodes++;
	}

	//Add the links to the partitions
	for(int i=0; i<this->VDCRequest->links; i++){
		link_t* l = this->VDCRequest->link[i];
		int source = l->nodeFrom;
		int destination = l->nodeTo;
		int sourcePartition = this->node2partitions->at(source);
		int destinationPartition = this->node2partitions->at(destination);
		l->partitionFrom = this->node2partitions->at(source);
		l->partitionTo = this->node2partitions->at(destination);
		mapping_virtual_link_to_substrate_path* mapStructLink = new mapping_virtual_link_to_substrate_path;
		mapStructLink->DCId = -1;
		mapStructLink->mapBegin = -1;
		mapStructLink->mapEnd = -1;
		mapStructLink->requestID = this->VDCRequest->reqID;
		mapStructLink->substratePath = NULL;
		mapStructLink->virtualLink = l;
		l->mapping = mapStructLink;
		if(sourcePartition != destinationPartition){
			result->linksBetweenPartirions[sourcePartition][destinationPartition]->push_back(l);
		}else{
			//The nodes are in the same partition
			result->partitions->at(sourcePartition)->link[result->partitions->at(sourcePartition)->links] = l;
			result->partitions->at(sourcePartition)->links++;
		}
	}

	//Return the result
	return result;

}



