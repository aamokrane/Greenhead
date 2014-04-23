/*
 * CentralController.cpp
 *
 *  Created on: Dec 27, 2012
 *      Author: ahmedamokrane
 */

#include "CentralController.h"

CentralController::CentralController(central_controller_parameters_t* parameters) {

	vdcRequestsEmbedded = new vector<VDCRequestPartitioned*>();
	vdcRequestsPreviouslyEmbedded = new vector<VDCRequestPartitioned*>();
	this->substrateInfrastructure = NULL;
	this->SLAviolatioOverTime = NULL;
	this->reoptimazationSummariesOverTime = NULL;
	this->wholeInfrastructureOverTime = NULL;
	this->datacenterParameterOverTime = NULL;

	this->centralControllerParameters = parameters;

	this->nbEmbeddedRequests =0;
	this->nbTotalRequests = 0;
	this->stopReoptimization = true;
	this->stopSLAViolationReporting = true;
	this->stopUtilizationReporting = true;

	this->dataBaseConnector = NULL;
}

CentralController::~CentralController() {

	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		delete this->vdcRequestsEmbedded->at(i);
	}

	for(unsigned int i=0; i<this->vdcRequestsPreviouslyEmbedded->size(); i++){
		delete this->vdcRequestsPreviouslyEmbedded->at(i);
	}

	this->vdcRequestsEmbedded->clear();
	delete this->vdcRequestsEmbedded;
	this->vdcRequestsPreviouslyEmbedded->clear();
	delete this->vdcRequestsPreviouslyEmbedded;
	delete this->substrateInfrastructure;

	for(unsigned int i=0; i<this->datacenterParameterOverTime->size(); i++){
		for(unsigned int j=0; j<this->datacenterParameterOverTime->at(i)->size(); j++){
			delete this->datacenterParameterOverTime->at(i)->at(j);
		}
	}
	this->datacenterParameterOverTime->clear();
	delete this->datacenterParameterOverTime;

	for(unsigned int i=0; i<this->wholeInfrastructureOverTime->size(); i++){
		delete this->wholeInfrastructureOverTime->at(i);
	}
	delete this->wholeInfrastructureOverTime;

}


double CentralController::getElectricityPrice(int idDC, double t) {
	//Returns the electricity price for datacenter idDC that corresponds to the time t

	//Convert to time slots to get the time

	return 0.5;
}





double CentralController::embedVDCRequestGreedy(VDCRequestPartitioned* request, double t){
	/*
	 * Greedy Embedding
	 * Start partition by partition, each time we embed a partition, we embed the corresponding links with the previously embedded partitions
	 */
	cout << "Embed the VDC request of ID : " << request->idRequest << " at time " << t  << endl;

	this->nbTotalRequests++;

	int nbPart = request->numberOfPartitions;
	unsigned int ub = (int)this->substrateInfrastructure->dataCenters->size();
	int nbDCs = *((int*)(&ub));
	//double costPartitionDC[request->numberOfPartitions][substrateInfrastructure->numberOfDatacenters];
	vector<vector<double> > costPartitionDC = vector<vector<double> >(request->numberOfPartitions);
	for(int i = 0; i<nbPart; i++){
		costPartitionDC.at(i) = vector<double>(substrateInfrastructure->numberOfDatacenters);
	}

	vector<vector<int> > bestDatacenter = vector<vector<int> >(request->numberOfPartitions);

	// Build the sorted list of the possible datacenters
	for(int i = 0; i<nbPart; i++){
		bestDatacenter[i] = vector<int>();
		for(int j=0; j<nbDCs; j++){
			bool possible = this->substrateInfrastructure->isPossibleToEmbedPartitionInDataCenter(request->partitions->at(i), j);
			if(possible){
				double powerInDC = substrateInfrastructure->dataCenters->at(j)->getCostEnergyForPartition(request->partitions->at(i), t);

				if(powerInDC < 0){
					cout << "There is a serious problem over here";
				}

				costPartitionDC[i][j] = powerInDC*substrateInfrastructure->dataCenters->at(j)->getPUE(t)*substrateInfrastructure->dataCenters->at(j)->getElectrictyPrice(t); //Electricity price
				costPartitionDC[i][j] += powerInDC*substrateInfrastructure->dataCenters->at(j)->getPUE(t)*substrateInfrastructure->dataCenters->at(j)->getCarbonFootPrintCoefficient(t)*substrateInfrastructure->dataCenters->at(j)->getCarbonFootPrintUnitCost(t); //Carbon cost

				//We choose the datacenter according to PUE*(electricityPrice + CarbonCoeff*CarbonCost)

				if(costPartitionDC[i][j] < 0){
					//It is not possible to embed this partition in this datacenter, the datacenter is overloaded
					costPartitionDC[i][j] = INFINITY;
				}else{
					//Insert the datacenter in the sorted queue;
					bool doneDCij = false;
					for(unsigned int k=0; k<bestDatacenter[i].size(); k++){
						if (costPartitionDC[i][bestDatacenter[i].at(k)] > costPartitionDC[i][j]){
							bestDatacenter[i].insert(bestDatacenter[i].begin()+k, j);
							doneDCij = true;
							break;
						}
					}
					if(!doneDCij){
						bestDatacenter[i].push_back(j);
					}

				}
			}else{
				//This datacenter is not possible
				costPartitionDC[i][j] = INFINITY;
			}
		}

		//See if there is at least one possible datacenter
		if(bestDatacenter[i].empty()){
			//There is no possible datacenter

			return FAILURE_PARTITION_EMBED;
		}
	}//END FOR i<nbPart

	//Start partition Embedding
	double cost=0.0;
	//vector<vector<mapping_virtual_link_to_substrate_path*> > linkAllocationList = vector<vector<mapping_virtual_link_to_substrate_path*> >(request->numberOfPartitions);
	//Add the request to the list of embedded requests
	for(int i=0; i<request->numberOfPartitions; i++){

		bool doneForPartitionI = true;
		int bestDC = -1;
		for(unsigned int m=0; m<bestDatacenter[i].size(); m++){ //See the best datacenter we find

			int currentDC = bestDatacenter[i].at(m);

			//linkAllocationList[i] = vector<mapping_virtual_link_to_substrate_path*>();
			//vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path*>();

			bool doneForLinksBetweenPartitionIandJ = true;

			for(int j=0; j<i; j++){ //j for the previously embedded partitions

				//int DCj = request->partitions->at(j)->mappings[request->partitions->at(j)->numberOfMappings-1]->datacenterID;
				int DCj = request->partitions->at(j)->mapping->datacenterID;

				//vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path>();
				bool donForlinkijk = true;
				vector<link_t*> linksBetweenIJ = vector<link_t*>();
				for(unsigned int k=0; k<request->linksBetweenPartirions[j][i]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[j][i]->at(k));
				}
				for(unsigned int k=0; k<request->linksBetweenPartirions[i][j]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[i][j]->at(k));
				}


				for(unsigned int k=0; k<linksBetweenIJ.size(); k++){

					link_t* l =linksBetweenIJ.at(k);
					double cost2 = 0;
					path_t* p = NULL;
					int DC = -1;
					if(currentDC == DCj){
						//The two partitions are embedded in the same datacenter
						DC = DCj;
						cost2 = substrateInfrastructure->dataCenters->at(currentDC)->embedLink(l, t);
					}
					else{
						//The partitions are embedded in different datacenters, embed the links between them in the provisioned network
						p = substrateInfrastructure->embedVirtualLinkBetweenDataCenters(currentDC, DCj,l->bw, l->delay, t, &cost2);
					}

					if(p!= NULL || DC > -1){
						//Add the mapping structure to the list of mapped links in the provisioned network
						//mapping_virtual_link_to_substrate_path* mapStructLink = new mapping_virtual_link_to_substrate_path;
						mapping_virtual_link_to_substrate_path* mapStructLink = l->mapping;
						mapStructLink->mapBegin = t;
						mapStructLink->mapEnd = -1;
						mapStructLink->costPerUnitOfTime = cost2;
						mapStructLink->requestID = request->idRequest;
						mapStructLink->substratePath = p;
						mapStructLink->DCId = DC;
						mapStructLink->virtualLink = l;
						//l->mapping = mapStructLink;

					}else{
						//We could not embed the virtual link in the provisioned network
						//Remove all the links and nodes that have been embedded before
						donForlinkijk = false;
						for(unsigned int z=0; z<linksBetweenIJ.size(); z++){
							mapping_virtual_link_to_substrate_path* mapStructLink = linksBetweenIJ.at(z)->mapping;
							if(mapStructLink->DCId >= 0){
								substrateInfrastructure->dataCenters->at(mapStructLink->DCId)->removeLink(mapStructLink->virtualLink,t); //request_t *, partition_t *, partition_t *, int, int, double
							}else{
								p = mapStructLink->substratePath;
								if(p != NULL){
									substrateInfrastructure->removeVirtualLinkGivenPath(p,t);
								}else{
									//The link have not been embeddded so far
								}
							}
							linksBetweenIJ.at(z)->mapping->DCId = -1;
							linksBetweenIJ.at(z)->mapping->substratePath = NULL;
						}
						//Move to the next datacenter
						break;
					}

				}//END FOR K LINKS BETWEEN PARTITION I and J

				//If we could embed all the links between the partitions
				if(donForlinkijk){
					//We could embed all the links between ij
					doneForPartitionI = true;
				}else{
					//We could not embed the virtual links between the two partitions i and j
					doneForLinksBetweenPartitionIandJ = false;
					break;
				}
			}//END FOR //j for the previously embedded partitions

			if(doneForLinksBetweenPartitionIandJ){
				//We could embed the virtual links with all the neighboring partitions
				bestDC = currentDC;
				doneForPartitionI = true;

				//Embed the partition in the best datacenter
				double temp = substrateInfrastructure->dataCenters->at(bestDC)->embedPartition(request->partitions->at(i), t);
				temp = (temp/1000) * substrateInfrastructure->dataCenters->at(bestDC)->getPUE(t)*(substrateInfrastructure->dataCenters->at(bestDC)->getElectrictyPrice(t) + substrateInfrastructure->dataCenters->at(bestDC)->getCarbonFootPrintCoefficient(t)*substrateInfrastructure->dataCenters->at(bestDC)->getCarbonFootPrintUnitCost(t));
				cost += temp;

				//Update the partition Mapping in the partition set in the VDC request
				//mapping_parition_to_datacenter* mapStructPartition = new mapping_parition_to_datacenter;
				mapping_parition_to_datacenter* mapStructPartition = request->partitions->at(i)->mapping;
				mapStructPartition->datacenterID = bestDC;
				mapStructPartition->costPerUnitOfTime = costPartitionDC[i][bestDC];
				mapStructPartition->mapBegin = t;
				mapStructPartition->mapEnd = -1;
				mapStructPartition->requestID = request->idRequest;
				mapStructPartition->partitioID = request->partitions->at(i)->partitionID;

				request->partitions->at(i)->mapping = mapStructPartition;
				break;

			}else{
				//We could not embed the links for this partition

			}//END IF doneForLinksBetweenPartitionIandJ


		}//End m on the set of possible datacenters

		if(bestDC>-1){
			//We could embed the ith patition
		}else{
			//We could not embed the partition in any of the possible datacenters
			double a=-1,b=-2;
			this->vdcRequestsEmbedded->push_back(request);
			this->removeRequestWithLinksBetweenPartitions(request->originalRequest, t, &a, &b);
			return FAILURE_LINK_EMBED;
		}

	}//END I FOR THE SET OF PARTITIONS

	this->vdcRequestsEmbedded->push_back(request);
	this->nbEmbeddedRequests++;

	//We update the mapping of the links within each partition
	for(int i=0; i<request->numberOfPartitions; i++){
		partition_t* partition  = request->partitions->at(i);
		for(int j=0; j<partition->links; j++){
			link_t* l = partition->link[j];
			l->mapping->DCId = partition->mapping->datacenterID;
			l->mapping->substratePath = NULL;
			l->mapping->mapBegin = t;
			l->mapping->mapEnd = -1;
		}
	}

	//We write the request to the database
	if(this->dataBaseConnector != NULL){
		this->dataBaseConnector->writeVDCRequestPartitionedToDataBase(request);
	}

	return cost;

}




double CentralController::embedVDCRequestGreedyUsingEstimationOfRenewables(VDCRequestPartitioned* request, double t){
	/*
	 * Greedy Embedding
	 * Start partition by partition, each time we embed a partition, we embed the corresponding links with the previously embedded partitions
	 * This method uses estimation of the future available renewables and
	 */
	cout << "Embed the VDC request of ID : " << request->idRequest << " at time " << t  << endl;

	this->nbTotalRequests++;

	int nbPart = request->numberOfPartitions;
	unsigned int ub = (int)this->substrateInfrastructure->dataCenters->size();
	int nbDCs = *((int*)(&ub));
	//double costPartitionDC[request->numberOfPartitions][substrateInfrastructure->numberOfDatacenters];
	vector<vector<double> > costPartitionDC = vector<vector<double> >(request->numberOfPartitions);
	for(int i = 0; i<nbPart; i++){
		costPartitionDC.at(i) = vector<double>(substrateInfrastructure->numberOfDatacenters);
	}

	vector<vector<int> > bestDatacenter = vector<vector<int> >(request->numberOfPartitions);

	// Build the sorted list of the possible datacenters
	for(int i = 0; i<nbPart; i++){
		bestDatacenter[i] = vector<int>();
		for(int j=0; j<nbDCs; j++){
			bool possible = this->substrateInfrastructure->isPossibleToEmbedPartitionInDataCenter(request->partitions->at(i), j);
			if(possible){
				double powerInDC = substrateInfrastructure->dataCenters->at(j)->getPowerForPartition(request->partitions->at(i));


				if(powerInDC < 0){
					cout << "There is a serious problem over here";
				}

				double timeEnd = request->originalRequest->arrtime + request->originalRequest->duration;
				int nbIntervals = t/this->centralControllerParameters->reoptimizationInterval;
				double timeNextReconfiguration = t + (this->centralControllerParameters->reoptimizationInterval-(t-this->centralControllerParameters->reoptimizationInterval*nbIntervals));
				if(timeEnd > t + this->centralControllerParameters->lookAheadTimeForForcasting){
					timeEnd = t + this->centralControllerParameters->lookAheadTimeForForcasting;
				}
				if(timeEnd > timeNextReconfiguration){
					timeEnd = timeNextReconfiguration;
				}

				double AvailableRenewableInDC =  (substrateInfrastructure->dataCenters->at(j)->getEstimationOfAmountOfRenewables(t,timeEnd)/(timeEnd-t)) - substrateInfrastructure->dataCenters->at(j)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(t, timeEnd)*substrateInfrastructure->dataCenters->at(j)->getPUE(t);

				if(AvailableRenewableInDC > 0){
					costPartitionDC[i][j] = 0;
				}else{
					costPartitionDC[i][j] = powerInDC*substrateInfrastructure->dataCenters->at(j)->getPUE(t)*substrateInfrastructure->dataCenters->at(j)->getEstimationOfAverageElectricityPrice(t,timeEnd); //Electricity price
					costPartitionDC[i][j] += powerInDC*substrateInfrastructure->dataCenters->at(j)->getPUE(t)*substrateInfrastructure->dataCenters->at(j)->getCarbonFootPrintCoefficient(t)*substrateInfrastructure->dataCenters->at(j)->getCarbonFootPrintUnitCost(t); //Carbon cost
				}
				//We choose the datacenter according to PUE*(electricityPrice + CarbonCoeff*CarbonCost)

				if(costPartitionDC[i][j] < 0){
					//It is not possible to embed this partition in this datacenter, the datacenter is overloaded
					costPartitionDC[i][j] = INFINITY;
				}else{
					//Insert the datacenter in the sorted queue;
					bool doneDCij = false;
					for(unsigned int k=0; k<bestDatacenter[i].size(); k++){
						if (costPartitionDC[i][bestDatacenter[i].at(k)] > costPartitionDC[i][j]){
							bestDatacenter[i].insert(bestDatacenter[i].begin()+k, j);
							doneDCij = true;
							break;
						}
					}
					if(!doneDCij){
						bestDatacenter[i].push_back(j);
					}

				}
			}else{
				//This datacenter is not possible
				costPartitionDC[i][j] = INFINITY;
			}
		}

		//See if there is at least one possible datacenter
		if(bestDatacenter[i].empty()){
			//There is no possible datacenter
			return FAILURE_PARTITION_EMBED;
		}
	}//END FOR i<nbPart

	//Start partition Embedding
	double cost=0.0;
	//vector<vector<mapping_virtual_link_to_substrate_path*> > linkAllocationList = vector<vector<mapping_virtual_link_to_substrate_path*> >(request->numberOfPartitions);
	//Add the request to the list of embedded requests
	for(int i=0; i<request->numberOfPartitions; i++){

		bool doneForPartitionI = true;
		int bestDC = -1;
		for(unsigned int m=0; m<bestDatacenter[i].size(); m++){ //See the best datacenter we find

			int currentDC = bestDatacenter[i].at(m);

			//linkAllocationList[i] = vector<mapping_virtual_link_to_substrate_path*>();
			//vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path*>();

			bool doneForLinksBetweenPartitionIandJ = true;

			for(int j=0; j<i; j++){ //j for the previously embedded partitions

				//int DCj = request->partitions->at(j)->mappings[request->partitions->at(j)->numberOfMappings-1]->datacenterID;
				int DCj = request->partitions->at(j)->mapping->datacenterID;

				//vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path>();
				bool donForlinkijk = true;
				vector<link_t*> linksBetweenIJ = vector<link_t*>();
				for(unsigned int k=0; k<request->linksBetweenPartirions[j][i]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[j][i]->at(k));
				}
				for(unsigned int k=0; k<request->linksBetweenPartirions[i][j]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[i][j]->at(k));
				}


				for(unsigned int k=0; k<linksBetweenIJ.size(); k++){

					link_t* l =linksBetweenIJ.at(k);
					double cost2 = 0;
					path_t* p = NULL;
					int DC = -1;
					if(currentDC == DCj){
						//The two partitions are embedded in the same datacenter
						DC = DCj;
						cost2 = substrateInfrastructure->dataCenters->at(currentDC)->embedLink(l, t);
					}
					else{
						//The partitions are embedded in different datacenters, embed the links between them in the provisioned network
						p = substrateInfrastructure->embedVirtualLinkBetweenDataCenters(currentDC, DCj,l->bw, l->delay, t, &cost2);
					}

					if(p!= NULL || DC > -1){
						//Add the mapping structure to the list of mapped links in the provisioned network
						//mapping_virtual_link_to_substrate_path* mapStructLink = new mapping_virtual_link_to_substrate_path;
						mapping_virtual_link_to_substrate_path* mapStructLink = l->mapping;
						mapStructLink->mapBegin = t;
						mapStructLink->mapEnd = -1;
						mapStructLink->costPerUnitOfTime = cost2;
						mapStructLink->requestID = request->idRequest;
						mapStructLink->substratePath = p;
						mapStructLink->DCId = DC;
						mapStructLink->virtualLink = l;
						//l->mapping = mapStructLink;

					}else{
						//We could not embed the virtual link in the provisioned network
						//Remove all the links and nodes that have been embedded before
						donForlinkijk = false;
						for(unsigned int z=0; z<linksBetweenIJ.size(); z++){
							mapping_virtual_link_to_substrate_path* mapStructLink = linksBetweenIJ.at(z)->mapping;
							if(mapStructLink->DCId >= 0){
								substrateInfrastructure->dataCenters->at(mapStructLink->DCId)->removeLink(mapStructLink->virtualLink,t); //request_t *, partition_t *, partition_t *, int, int, double
							}else{
								p = mapStructLink->substratePath;
								if(p != NULL){
									substrateInfrastructure->removeVirtualLinkGivenPath(p,t);
								}else{
									//The link have not been embeddded so far
								}
							}
							linksBetweenIJ.at(z)->mapping->DCId = -1;
							linksBetweenIJ.at(z)->mapping->substratePath = NULL;
						}
						//Move to the next datacenter
						break;
					}

				}//END FOR K LINKS BETWEEN PARTITION I and J

				//If we could embed all the links between the partitions
				if(donForlinkijk){
					//We could embed all the links between ij
					doneForPartitionI = true;
				}else{
					//We could not embed the virtual links between the two partitions i and j
					doneForLinksBetweenPartitionIandJ = false;
					break;
				}
			}//END FOR //j for the previously embedded partitions

			if(doneForLinksBetweenPartitionIandJ){
				//We could embed the virtual links with all the neighboring partitions
				bestDC = currentDC;
				doneForPartitionI = true;

				//Embed the partition in the best datacenter
				double temp = substrateInfrastructure->dataCenters->at(bestDC)->embedPartition(request->partitions->at(i), t);
				cost += temp;

				//Update the partition Mapping in the partition set in the VDC request
				//mapping_parition_to_datacenter* mapStructPartition = new mapping_parition_to_datacenter;
				mapping_parition_to_datacenter* mapStructPartition = request->partitions->at(i)->mapping;
				mapStructPartition->datacenterID = bestDC;
				mapStructPartition->costPerUnitOfTime = costPartitionDC[i][bestDC];
				mapStructPartition->mapBegin = t;
				mapStructPartition->mapEnd = -1;
				mapStructPartition->requestID = request->idRequest;
				mapStructPartition->partitioID = request->partitions->at(i)->partitionID;

				request->partitions->at(i)->mapping = mapStructPartition;
				break;

			}else{
				//We could not embed the links for this partition

			}//END IF doneForLinksBetweenPartitionIandJ


		}//End m on the set of possible datacenters

		if(bestDC>-1){
			//We could embed the ith patition
		}else{
			//We could not embed the partition in any of the possible datacenters
			double a=-1,b=-2;
			this->vdcRequestsEmbedded->push_back(request);
			this->removeRequestWithLinksBetweenPartitions(request->originalRequest, t, &a, &b);
			return FAILURE_LINK_EMBED;
		}

	}//END I FOR THE SET OF PARTITIONS

	this->vdcRequestsEmbedded->push_back(request);
	this->nbEmbeddedRequests++;

	//We update the mapping of the links within each partition
	for(int i=0; i<request->numberOfPartitions; i++){
		partition_t* partition  = request->partitions->at(i);
		for(int j=0; j<partition->links; j++){
			link_t* l = partition->link[j];
			l->mapping->DCId = partition->mapping->datacenterID;
			l->mapping->substratePath = NULL;
			l->mapping->mapBegin = t;
			l->mapping->mapEnd = -1;
		}
	}


	//Write the request to the database
	if(this->dataBaseConnector != NULL){
		this->dataBaseConnector->writeVDCRequestPartitionedToDataBase(request);
	}

	return cost;



}





VDCRequestPartitioned* CentralController::partitionVDCRequestNoPartitioning(request_t* initialRequest){
	//This is a modified version of the Louvain Algorithm to partition the VDC request
	RawGraph* gRequest = new RawGraph(initialRequest);
	VDCRequestPartitioned* result = gRequest->getVDCRequestPartitioned();
	delete gRequest;

	return result;
}


VDCRequestPartitioned* CentralController::partitionVDCRequestLouvainAlgorithmModifiedVersion(request_t* initialRequest){
	//This is a modified version of the Louvain Algorithm to partition the VDC request
	RawGraph* gRequest = new RawGraph(initialRequest);
	VDCRequestPartitioned* result = gRequest->partitionUsingLouvainAlgprithm();
	delete gRequest;
	return result;
}

VDCRequestPartitioned* CentralController::partitionVDCRequestIntoEqualSizePartitions(request_t* initialRequest){
	//This is a modified version of the Louvain Algorithm to partition the VDC request
	RawGraph* gRequest = new RawGraph(initialRequest);
	VDCRequestPartitioned* result = gRequest->partitionIntoEqualPartitionsAlgorithmConsideringLocationConstraints(this->substrateInfrastructure->numberOfDatacenters);
	delete gRequest;
	return result;
}


VDCRequestPartitioned* CentralController::partitionVDCRequestToGetOneSinglePartition(request_t* initialRequest){
	RawGraph* gRequest = new RawGraph(initialRequest);
	VDCRequestPartitioned* result = gRequest->partitionToGetOneSinglePartition();
	delete gRequest;
	return result;
}



double CentralController::removeRequestWithLinksBetweenPartitions(request_t* request, double t, double* costDataCenters, double* costProvisionedNet){
	//Remove the VDC request  request that have been embedded earlier

	cout << "The request " << request->reqID << " is leaving the system at time "<< t << endl;

	//find the request in the embedded list


	for(unsigned int i =0; i<this->vdcRequestsEmbedded->size(); i++){
		if(this->vdcRequestsEmbedded->at(i)->idRequest == request->reqID){
			//We found the request
			double costDCs = 0;
			double costProvisionedNetwork = 0;
			VDCRequestPartitioned* requestPartioned = this->vdcRequestsEmbedded->at(i);

			//remove the links that have been embedded
			for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){//FOR LINKS
				for(unsigned int k=0; k < requestPartioned->partitions->size(); k++){
					//Check whether it is the same datacenter or not
					if(k!=j && requestPartioned->partitions->at(j)->mapping->datacenterID != -1 && requestPartioned->partitions->at(k)->mapping->datacenterID != -1){
						//Check whether it is the same datacenter or not
						//int DCj = requestPartioned->partitions->at(j)->mapping->datacenterID;
						//int DCk =  requestPartioned->partitions->at(k)->mapping->datacenterID;
						for(unsigned int m=0; m<requestPartioned->linksBetweenPartirions[j][k]->size(); m++){
							link_t* l = requestPartioned->linksBetweenPartirions[j][k]->at(m);
							//if(l->numberOfMappings > 0){ //This test is added to ignore the case where a link was not embedded
							if(l->mapping->DCId != -1 || l->mapping->substratePath != NULL){
								l->mapping->mapEnd = t;
								if(l->mapping->DCId != -1){
									//The two partitions are in the same datacenter, the link is in the same datacenter
									costDCs += this->substrateInfrastructure->dataCenters->at(l->mapping->DCId)->removeLink(l,t);
									l->mapping->DCId = -1;
								}else{
									mapping_virtual_link_to_substrate_path* mapping = l->mapping;
									//free the resources in the provisioned network
									this->substrateInfrastructure->removeVirtualLinkGivenPath(mapping->substratePath, t);
									l->mapping->substratePath = NULL;
								}
							}
						}
					}
				}
			}////FOR LINKS


			//remove the nodes that have been embedded
			for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){
				//compute the cost of this request
				//if(requestPartioned->partitions->at(j)->numberOfMappings > 0){
				if(requestPartioned->partitions->at(j)->mapping->datacenterID  != -1){
					requestPartioned->partitions->at(j)->mapping->mapEnd = t;
					int DCid = requestPartioned->partitions->at(j)->mapping->datacenterID;
					//DCid represents the index of the datacenter
					this->substrateInfrastructure->dataCenters->at(DCid)->removePartition(requestPartioned->partitions->at(j), t);
				}
			}

			//Remove the mappings for partitions
			for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){
				//requestPartioned->partitions->at(j)->numberOfMappings = 0;
				requestPartioned->partitions->at(j)->mapping->datacenterID = -1;
			}

			*costDataCenters = costDCs;
			*costProvisionedNet = costProvisionedNetwork;
			VDCRequestPartitioned* rq = this->vdcRequestsEmbedded->at(i);
			this->vdcRequestsEmbedded->erase(this->vdcRequestsEmbedded->begin()+i);

			//Add this request to the previously embedded ones if it is not already there
			bool found = false;
			for(unsigned int i =0; i<this->vdcRequestsPreviouslyEmbedded->size(); i++){
				if(this->vdcRequestsPreviouslyEmbedded->at(i)->idRequest == rq->idRequest){
					found = true;
					break;
				}
			}
			if(!found){
				this->vdcRequestsPreviouslyEmbedded->push_back(rq);
			}

			if(costDCs + costProvisionedNetwork < 0){
				cout << "WEIRD HERE, THE COST FOR REQUEST"<< request->reqID <<" IS NEGATIVE"<< endl;
			}

			//Remove the request from the database
			if(this->dataBaseConnector != NULL){
				this->dataBaseConnector->removeVDCRequestFromDataBase(request,t);
			}

			//return costDCs + costProvisionedNetwork;
			return 1.0;

		}//END IF IDREQUEST=CURRENT REQUEST IN THE LIST OF EMBEDDED REQUESTS

	}//END FOR

	//Here we didn't find the request, there is a problem, rise exception

	for(unsigned int i =0; i<this->vdcRequestsPreviouslyEmbedded->size(); i++){
		if(this->vdcRequestsPreviouslyEmbedded->at(i)->idRequest == request->reqID){
			cout << "PROBLEM IS HERE, THE REQUEST"<< request->reqID <<"HAS ALREADY LEFT"<< endl;
		}
	}

	//cout << "SERIOUS PROBLEM: THE REQUEST "<< request->reqID <<" DISAPPEARED"<< endl;

	//throw new std::exception("There is a problem, the request was not found in the list of embedded partitions");

	return -1.0;
}



double CentralController::removeRequestWithoutRemovingLinks(request_t* request, double t, double* costDataCenters, double* costProvisionedNet, bool addToPreviouslyEmbeddedRequests){
	//Remove the VDC request  request that have been embedded earlier

	//cout << "The request " << request->reqID << " is leaving the system at time "<< t << endl;

	//find the request in the embedded list


	for(unsigned int i =0; i<this->vdcRequestsEmbedded->size(); i++){
		if(this->vdcRequestsEmbedded->at(i)->idRequest == request->reqID){
			//We found the request
			double costDCs = 0;
			double costProvisionedNetwork;
			VDCRequestPartitioned* requestPartioned = this->vdcRequestsEmbedded->at(i);
			//remove the nodes that have been embedded
			for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){
				//compute the cost of this request
				//if(requestPartioned->partitions->at(j)->numberOfMappings > 0){
				if(requestPartioned->partitions->at(j)->mapping != NULL){
					//requestPartioned->partitions->at(j)->mappings[requestPartioned->partitions->at(j)->numberOfMappings - 1]->mapEnd = t;
					requestPartioned->partitions->at(j)->mapping->mapEnd = t;
					//for(int k=0; k<requestPartioned->partitions->at(j)->numberOfMappings; k++){
					//	costDCs+=requestPartioned->partitions->at(j)->mappings[k]->costPerUnitOfTime*fabs(requestPartioned->partitions->at(j)->mappings[k]->mapEnd-requestPartioned->partitions->at(j)->mappings[k]->mapBegin);
					//}
					//Remove the current mapping
					//int DCid = requestPartioned->partitions->at(j)->mappings[requestPartioned->partitions->at(j)->numberOfMappings - 1]->datacenterID;
					int DCid = requestPartioned->partitions->at(j)->mapping->datacenterID;
					//DCid represents the index of the datacenter
					this->substrateInfrastructure->dataCenters->at(DCid)->removePartition(requestPartioned->partitions->at(j), t);
				}

			}


			//Remove the mappings for partitions
			for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){
				//requestPartioned->partitions->at(j)->numberOfMappings = 0;
				requestPartioned->partitions->at(j)->mapping = NULL;
			}

			*costDataCenters = costDCs;
			*costProvisionedNet = costProvisionedNetwork;
			VDCRequestPartitioned* rq = this->vdcRequestsEmbedded->at(i);
			this->vdcRequestsEmbedded->erase(this->vdcRequestsEmbedded->begin()+i);

			//Add this request to the previously embedded ones if it is not already there and if addToPreviouslyEmbeddedRequests is set to false
			if(addToPreviouslyEmbeddedRequests == true){
				bool found = false;
				for(unsigned int i =0; i<this->vdcRequestsPreviouslyEmbedded->size(); i++){
					if(this->vdcRequestsPreviouslyEmbedded->at(i)->idRequest == rq->idRequest){
						found = true;
						break;
					}
				}
				if(!found){
					this->vdcRequestsPreviouslyEmbedded->push_back(rq);
				}
			}

			//Remove the request from the database
			if(this->dataBaseConnector != NULL){
				this->dataBaseConnector->removeVDCRequestFromDataBase(request,t);
			}

			return costDCs + costProvisionedNetwork;

		}//END IF IDREQUEST=CURRENT REQUEST IN THE LIST OF EMBEDDED REQUESTS

	}//END FOR

	//Here we didn't find the request, there is a problem, rise exception

	//throw new std::exception("There is a problem, the request was not found in the list of embedded partitions");

	return -1.0;
}



//Get the instantaneous cost in the DCs and the provisioned network
infrastructure_utilization_t* CentralController::getInstanteousUtilizationWholeInfrastructure(double t){
	double costInDatcenters = 0.0;
	//double costInProvisionedNetwork = 0.0;

	infrastructure_utilization_t* par;
	par = this->substrateInfrastructure->getLinkUtilizationDistribution();
	par->costForUsedNodes = this->substrateInfrastructure->getProvisionedNetworkInstanteousCostPerUSedNodes();
	par->amountOfCarbon = this->substrateInfrastructure->getSumUsedBandwidth()*this->centralControllerParameters->carbonPerUnitOfBandwidthInBackbone;
	par->totalUsedBandwidth = this->substrateInfrastructure->getSumUsedBandwidth();
	par->totalUsedCPU = 0;
	par->generatedRenewables = 0;
	par->remainingRenewables = 0;

	//datacenters Cost
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		datacenter_utilization_t* instUtil = this->substrateInfrastructure->dataCenters->at(i)->getInstantneousUtilization(t);
		costInDatcenters += instUtil->instantaneousCost;
		par->amountOfCarbon += instUtil->amountOfCarbon;
		par->generatedRenewables += instUtil->availableRenwables;
		par->remainingRenewables += instUtil->remainingRenewables;
		par->totalUsedBandwidth += instUtil->totalUsedBandwidth;
		par->totalUsedCPU += instUtil->totalUsedCPU;

	}

	//In the backbone network


	par->carbonEmissionPerUnitOfVM = this->getCarbonFootprintPerUnitOfVMInAllDatacenters(t);
	//par->carbonEmissionPerUnitOfBandwidth = this->getCarbonFootprintPerUnitOfBandwidthInAllDatacenters(t);
	par->carbonEmissionPerUnitOfBandwidth = this->getCarbonFootprintPerUnitOfBandwidthInWholeInfrastructure(t);


	par->costDatacenters = costInDatcenters;

	par->instantaneousNumberOfEmbeddedRequests = this->getInstanteousNumberOfEmbeddedRequests(t);

	//The Carbon Emissions and the limits in Carbon Emissions
	double totalCarbonLimit = 0;
	double VMUnits = 0;
	double BWUnits = 0;
	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		totalCarbonLimit += this->vdcRequestsEmbedded->at(i)->originalRequest->limitOfCarbonEmissionPerServicePeriodGuarantee;
		for(int j=0; j<this->vdcRequestsEmbedded->at(i)->originalRequest->nodes; j++){
			VMUnits += this->vdcRequestsEmbedded->at(i)->originalRequest->cpu[j];
		}
		for(int j=0; j<this->vdcRequestsEmbedded->at(i)->originalRequest->links; j++){
			BWUnits += this->vdcRequestsEmbedded->at(i)->originalRequest->link[j]->bw;
		}
	}

	par->carbonEmissionLimitByAllRequests = totalCarbonLimit/this->centralControllerParameters->SLAServiceGuaranteePeriod;
	par->carbonEmissionLimitPerUnitOfVM = par->carbonEmissionLimitByAllRequests/VMUnits;
	par->carbonEmissionLimitPerUnitOfBandwidth = par->carbonEmissionLimitByAllRequests/BWUnits;

	par->revenue = this->getInstanteousRevenue(t);

	par->tBegin = t;

	return par;

}



double CentralController::getInstanteousRevenue(double t){
	double res = 0;
	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		double revI = this->vdcRequestsEmbedded->at(i)->originalRequest->revenue;
		res += revI;
	}
	return res;
}


infrastructure_utilization_t* CentralController::getInstanteousOperatingCostInDataCenter(double t, int DCid){

	double costInDatcenters = 0.0;

	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){

		double costDCsFori = 0;
		double costProvisionedNetworkFori = 0;
		VDCRequestPartitioned* requestPartioned = this->vdcRequestsEmbedded->at(i);
		//get the cost for the partitions that have been embedded in datacenters
		for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){
			//compute the cost of this request
			//requestPartioned->partitions->at(j)->mappings[requestPartioned->partitions->at(j)->numberOfMappings - 1]->mapEnd = t;
			//int k = requestPartioned->partitions->at(j)->numberOfMappings-1;
			//int DCj = requestPartioned->partitions->at(j)->mappings[k]->datacenterID;
			int DCj = requestPartioned->partitions->at(j)->mapping->datacenterID;
			if(DCj == DCid){
				//costDCsFori += requestPartioned->partitions->at(j)->mappings[k]->costPerUnitOfTime;
				costDCsFori += requestPartioned->partitions->at(j)->mapping->costPerUnitOfTime;
			}
		}

		//get the instantaneous cost in the provisioned network
		for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){//FOR LINKS
			for(unsigned int k=0; k < requestPartioned->partitions->size(); k++){

				//int DCj = requestPartioned->partitions->at(j)->mappings[requestPartioned->partitions->at(j)->numberOfMappings - 1]->datacenterID;
				//int DCk = requestPartioned->partitions->at(k)->mappings[requestPartioned->partitions->at(k)->numberOfMappings - 1]->datacenterID;
				int DCj = requestPartioned->partitions->at(j)->mapping->datacenterID;
				int DCk = requestPartioned->partitions->at(k)->mapping->datacenterID;
				if(j != k && DCj == DCid && DCk == DCid){ //There is no link between a partition and itself

					for(unsigned int m=0; m<requestPartioned->linksBetweenPartirions[j][k]->size(); m++){
						link_t* l = requestPartioned->linksBetweenPartirions[j][k]->at(m);
						if(l->mapping != NULL){
							costInDatcenters += l->mapping->costPerUnitOfTime;
						}
						//int s = l->numberOfMappings - 1;
						//if(s>=0){
						//	costInDatcenters += l->mappings[s]->costPerUnitOfTime;
						//}
					}
				}
			}
		}////FOR LINKS

		costInDatcenters += costDCsFori;
	}//END FOR

	//*costDCs = costInDatcenters;
	//*costProvisionedNetwork = costInProvisionedNetwork;

	infrastructure_utilization_t* par = new infrastructure_utilization_t;

	par->costDatacenters = costInDatcenters;
	par->costForUsedNodes = -1;
	par->averageLinkUtilizationInBackbone = -1;
	par->tBegin = t;

	return par;
}



infrastructure_utilization_t* CentralController::getInstanteousOperatingCostInProvisionedNetwork(double t) {

	double costInProvisionedNetwork = 0.0;

	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){


		double costProvisionedNetworkFori = 0;
		VDCRequestPartitioned* requestPartioned = this->vdcRequestsEmbedded->at(i);

		//get the instantaneous cost in the provisioned network
		for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){//FOR LINKS
			for(unsigned int k=0; k < requestPartioned->partitions->size(); k++){

				//int DCj = requestPartioned->partitions->at(j)->mappings[requestPartioned->partitions->at(j)->numberOfMappings - 1]->datacenterID;
				//int DCk = requestPartioned->partitions->at(k)->mappings[requestPartioned->partitions->at(k)->numberOfMappings - 1]->datacenterID;
				int DCj = requestPartioned->partitions->at(j)->mapping->datacenterID;
				int DCk = requestPartioned->partitions->at(k)->mapping->datacenterID;

				if(j != k && DCj != DCk){ //There is no link between a partition and itself
					for(unsigned int m=0; m<requestPartioned->linksBetweenPartirions[j][k]->size(); m++){
						link_t* l = requestPartioned->linksBetweenPartirions[j][k]->at(m);
						//int s = l->numberOfMappings - 1;
						//costProvisionedNetworkFori += l->mappings[s]->costPerUnitOfTime;
						costProvisionedNetworkFori += l->mapping->costPerUnitOfTime;
					}
				}
			}
		}////FOR LINKS

		costInProvisionedNetwork += costProvisionedNetworkFori;
	}//END FOR

	//*costDCs = costInDatcenters;
	//*costProvisionedNetwork = costInProvisionedNetwork;

	infrastructure_utilization_t* par = new infrastructure_utilization_t;

	par->costDatacenters = -1;
	par->costForUsedNodes = costInProvisionedNetwork;
	par->averageLinkUtilizationInBackbone = this->substrateInfrastructure->getAverageLinkUtilization();
	par->tBegin = t;

	return par;

}




//Get the costs of the remaining requests in the
double CentralController::getCostRemainingRequests(double t, double* costDCs, double* costProvisionedNetwork){

	double costInDatcenters = 0.0;
	double costInProvisionedNetwork = 0.0;

	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){

		double costDCsFori = 0;
		double costProvisionedNetworkFori = 0;
		VDCRequestPartitioned* requestPartioned = this->vdcRequestsEmbedded->at(i);
		//remove the nodes that have been embedded
		for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){
			//compute the cost of this request
			requestPartioned->partitions->at(j)->mapping->mapEnd = t;
			//for(int k=0; k<requestPartioned->partitions->at(j)->numberOfMappings; k++){
			//	costDCsFori+=requestPartioned->partitions->at(j)->mappings[k]->costPerUnitOfTime*(requestPartioned->partitions->at(j)->mappings[k]->mapEnd-requestPartioned->partitions->at(j)->mappings[k]->mapBegin);
			//}
		}

		//Compute the cost for the links that have been embedded
		for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){//FOR LINKS
			for(unsigned int k=0; k < requestPartioned->partitions->size(); k++){
				if(j != k){
					for(unsigned int m=0; m<requestPartioned->linksBetweenPartirions[j][k]->size(); m++){
						link_t* l = requestPartioned->linksBetweenPartirions[j][k]->at(m);
						if(l->mapping != NULL){
							l->mapping->mapEnd = t;
							//Compute the cost of embedding this link
							//for(int s=0; s<l->numberOfMappings-1; s++){
							if( l->mapping->DCId == -1){
								//This link is embedded in the provisioned network
								costProvisionedNetworkFori += l->mapping->costPerUnitOfTime*fabs(l->mapping->mapEnd - l->mapping->mapBegin);
							}else{
								//This link is embedded inside a datacenter
								costDCsFori += l->mapping->costPerUnitOfTime*fabs(l->mapping->mapEnd - l->mapping->mapBegin);
							}
							//}
						}
					}
				}//END IF J != K
			}
		}////FOR LINKS

		costInDatcenters += costDCsFori;
		costInProvisionedNetwork += costProvisionedNetworkFori;
	}//END FOR I ON THE EMBEDDED REQUESTS

	*costDCs = costInDatcenters;
	*costProvisionedNetwork = costInProvisionedNetwork;
	return costInDatcenters + costInDatcenters;

}


double CentralController::getElectricityPrice(double t, location_t* location) {
	//Returns the electricity price of this location
	//TODO: Generate the price according to the locaiton of the node

	return 0.20;
}

void CentralController::updateInstantaneousCostOfEmbeddedRequests(double t) {


	//Get the instanteous cost per datacenter
	double instCostDC[this->substrateInfrastructure->numberOfDatacenters];
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		instCostDC[i] = this->substrateInfrastructure->dataCenters->at(i)->getCostPerUnitOfPowerAndTime(t);
	}


	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){

		//double costDCsFori = 0;
		//double costProvisionedNetworkFori = 0;
		VDCRequestPartitioned* requestPartioned = this->vdcRequestsEmbedded->at(i);
		//remove the nodes that have been embedded
		for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){
			//compute the cost of this request
			requestPartioned->partitions->at(j)->mapping->mapEnd = t-1;
			mapping_parition_to_datacenter* oldMap = requestPartioned->partitions->at(j)->mapping;
			//create new mapping structure

			mapping_parition_to_datacenter* newMap = new mapping_parition_to_datacenter;
			newMap->datacenterID = oldMap->datacenterID;
			newMap->costPerUnitOfTime = instCostDC[newMap->datacenterID];
			newMap->mapBegin = t;
			newMap->mapEnd = -1;
			newMap->requestID = oldMap->requestID;
			newMap->partitioID = oldMap->partitioID;
			//request->partitionMapping->at(i)->push_back(mapStructPartition);

			//request->partitions->at(i)->mappings->push_back(mapStructPartition);
			requestPartioned->partitions->at(j)->mapping = newMap;
			//requestPartioned->partitions->at(j)->numberOfMappings++;
		}

		//Compute the cost for the links that have been embedded
		for(unsigned int j=0; j < requestPartioned->partitions->size(); j++){//FOR LINKS
			for(unsigned int k=0; k < requestPartioned->partitions->size(); k++){
				if(j != k){
					for(unsigned int m=0; m<requestPartioned->linksBetweenPartirions[j][k]->size(); m++){
						link_t* l = requestPartioned->linksBetweenPartirions[j][k]->at(m);

						if(l->mapping != NULL){

							l->mapping->mapEnd = t-0.001;

							if( l->mapping->DCId == -1){
								//This link is embedded in the provisioned network
								//No update
							}else{
								//This link is embedded inside a datacenter
								mapping_virtual_link_to_substrate_path* mapStructLink = new mapping_virtual_link_to_substrate_path;
								mapStructLink->mapBegin = t;
								mapStructLink->mapEnd = -1;
								mapStructLink->costPerUnitOfTime = instCostDC[l->mapping->DCId];
								mapStructLink->requestID = requestPartioned->idRequest;
								mapStructLink->substratePath = NULL;
								mapStructLink->DCId =  l->mapping->DCId;
								mapStructLink->virtualLink = l;
								l->mapping = mapStructLink;
								//l->numberOfMappings++;
							}
						}
					}
				}//END IF J != K
			}
		}////FOR LINKS

	}//END FOR

}

double CentralController::getInstanteousNumberOfEmbeddedRequests(double t) {

	return (int)this->vdcRequestsEmbedded->size();

}

double CentralController::embedVDCRequestForLoadBalancingWithoutPartitioning(VDCRequestPartitioned* request, double t) {

	/*
	 * Greedy Embedding
	 * Start partition by partition, each time we embed a partition, we embed the corresponding links with the previously embedded partitions
	 */
	cout << "Embed the VDC request of ID : " << request->idRequest << " at time " << t  << endl;

	this->nbTotalRequests++;

	int nbPart = request->numberOfPartitions;
	unsigned int ub = (int)this->substrateInfrastructure->dataCenters->size();
	//int nbDCs = *((int*)(&ub));
	//double costPartitionDC[request->numberOfPartitions][substrateInfrastructure->numberOfDatacenters];
	vector<vector<double> > costPartitionDC = vector<vector<double> >(request->numberOfPartitions);
	for(int i = 0; i<nbPart; i++){
		costPartitionDC.at(i) = vector<double>(substrateInfrastructure->numberOfDatacenters);
	}

	vector<vector<int> > bestDatacenter = vector<vector<int> >(request->numberOfPartitions);

	// Build the sorted list of the possible datacenters
	for(int i = 0; i<nbPart; i++){

		//Check whether this partition has location constraint
		vector<location_t*> constrainedNodes = vector<location_t*>();
		//Ingore the lcoation constraints for this version of the amgorithm
		/*
			for(int k=0; k<request->partitions->at(i)->nodes; k++){
				if(request->partitions->at(i)->nodeLocation[k]->xLocation > -1){
					//This node has a location constraint
					int x = request->partitions->at(i)->nodeLocation[k]->xLocation;
					int y = request->partitions->at(i)->nodeLocation[k]->yLocation;
					if(x > 100 || y>100 || x<0 || y<0){
						cout << "There is a serious problem over here";
					}
					constrainedNodes.push_back(request->partitions->at(i)->nodeLocation[k]);
				}
			}
		 */
		bestDatacenter[i] = vector<int>();

		//for(int j=0; j<nbDCs; j++){
		double r = ((double) rand() / (RAND_MAX));
		//int j = (int)(r*this->substrateInfrastructure->numberOfDatacenters);
		int j = nbPart % this->substrateInfrastructure->numberOfDatacenters;

		{
			bool possible = true;
			for(unsigned int k=0; k<constrainedNodes.size(); k++){

				int x = constrainedNodes.at(k)->xLocation;
				int y = constrainedNodes.at(k)->yLocation;
				if(x > 100 || y>100 || x<0 || y<0){
					cout << "There is a serious problem over here";
				}

				double euclidianDist = (constrainedNodes.at(k)->xLocation - substrateInfrastructure->dataCenters->at(j)->location->xLocation)*(constrainedNodes.at(k)->xLocation - substrateInfrastructure->dataCenters->at(j)->location->xLocation) + (constrainedNodes.at(k)->yLocation - substrateInfrastructure->dataCenters->at(j)->location->yLocation)*(constrainedNodes.at(k)->yLocation - substrateInfrastructure->dataCenters->at(j)->location->yLocation);

				if(euclidianDist > (constrainedNodes.at(k)->maxDistance * constrainedNodes.at(k)->maxDistance)){
					possible = false;
				}
			}
			if(possible){
				double powerInDC = substrateInfrastructure->dataCenters->at(j)->getCostEnergyForPartition(request->partitions->at(i), t);

				if(powerInDC < 0){
					cout << "There is a serious problem over here";
				}


				costPartitionDC[i][j] = powerInDC*substrateInfrastructure->dataCenters->at(j)->getPUE(t)*substrateInfrastructure->dataCenters->at(j)->getElectrictyPrice(t); //Electricity price
				costPartitionDC[i][j] += powerInDC*substrateInfrastructure->dataCenters->at(j)->getPUE(t)*substrateInfrastructure->dataCenters->at(j)->getCarbonFootPrintCoefficient(t)*substrateInfrastructure->dataCenters->at(j)->getCarbonFootPrintUnitCost(t); //Carbon cost

				//We choose the datacenter according to PUE*(electricityPrice + CarbonCoeff*CarbonCost)

				if(costPartitionDC[i][j] < 0){
					//It is not possible to embed this partition in this datacenter, the datacenter is overloaded
					costPartitionDC[i][j] = INFINITY;
				}else{
					//Insert the datacenter in the sorted queue;
					/*
						bool doneDCij = false;
						for(unsigned int k=0; k<bestDatacenter[i].size(); k++){
							if (costPartitionDC[i][bestDatacenter[i].at(k)] > costPartitionDC[i][j]){
								bestDatacenter[i].insert(bestDatacenter[i].begin()+k, j);
								doneDCij = true;
								break;
							}
						}
						if(!doneDCij){
							bestDatacenter[i].push_back(j);
						}
					 */

					//No cost, we are performing load balancing
					bestDatacenter[i].push_back(j);

				}
			}else{
				//This datacenter is not possible
				costPartitionDC[i][j] = INFINITY;
			}
		}



		//See if there is at least one possible datacenter
		if(bestDatacenter[i].empty()){
			//There is no possible datacenter

			return FAILURE_PARTITION_EMBED;
		}
	}//END FOR i<nbPart

	//Start partition Embedding
	double cost=0.0;
	//vector<vector<mapping_virtual_link_to_substrate_path*> > linkAllocationList = vector<vector<mapping_virtual_link_to_substrate_path*> >(request->numberOfPartitions);
	//Add the request to the list of embedded requests
	for(int i=0; i<request->numberOfPartitions; i++){

		bool doneForPartitionI = true;
		int bestDC = -1;
		for(unsigned int m=0; m<bestDatacenter[i].size(); m++){ //See the best datacenter we find

			int currentDC = bestDatacenter[i].at(m);

			//double r = ((double) rand() / (RAND_MAX));

			//currentDC = (int)(r*this->substrateInfrastructure->numberOfDatacenters);

			//linkAllocationList[i] = vector<mapping_virtual_link_to_substrate_path*>();
			vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path*>();

			bool doneForLinksBetweenPartitionIandJ = true;

			for(int j=0; j<i; j++){ //j for the previously embedded partitions

				//int DCj = request->partitions->at(j)->mappings[request->partitions->at(j)->numberOfMappings-1]->datacenterID;
				int DCj = request->partitions->at(j)->mapping->datacenterID;
				//vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path>();
				bool donForlinkijk = true;
				vector<link_t*> linksBetweenIJ = vector<link_t*>();
				for(unsigned int k=0; k<request->linksBetweenPartirions[j][i]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[j][i]->at(k));
				}
				for(unsigned int k=0; k<request->linksBetweenPartirions[i][j]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[i][j]->at(k));
				}


				for(unsigned int k=0; k<linksBetweenIJ.size(); k++){

					link_t* l =linksBetweenIJ.at(k);
					double cost2 = 0;
					path_t* p = NULL;
					int DC = -1;
					if(currentDC == DCj){
						//The two partitions are embedded in the same datacenter
						DC = DCj;
						cost2 = substrateInfrastructure->dataCenters->at(currentDC)->embedLink(l, t);
					}
					else{
						//The partitions are embedded in different datacenters, embed the links between them in the provisioned network
						p = substrateInfrastructure->embedVirtualLinkBetweenDataCenters(currentDC, DCj,l->bw, l->delay, t, &cost2);
					}

					if(p!= NULL || DC > -1){
						//Add the mapping structure to the list of mapped links in the provisioned network
						mapping_virtual_link_to_substrate_path* mapStructLink = new mapping_virtual_link_to_substrate_path;
						mapStructLink->mapBegin = t;
						mapStructLink->mapEnd = -1;
						mapStructLink->costPerUnitOfTime = cost2;
						mapStructLink->requestID = request->idRequest;
						mapStructLink->substratePath = p;
						mapStructLink->DCId = DC;
						mapStructLink->virtualLink = l;

						linkAllocationList.push_back(mapStructLink);

					}else{
						//We could not embed the virtual link in the provisioned network
						//Remove all the links and nodes that have been embedded before
						donForlinkijk = false;
						for(unsigned int z=0; z<linkAllocationList.size(); z++){
							mapping_virtual_link_to_substrate_path* mapStructLink = linkAllocationList.at(z);
							if(mapStructLink->DCId >= 0){
								substrateInfrastructure->dataCenters->at(mapStructLink->DCId)->removeLink(mapStructLink->virtualLink, t); //request_t *, partition_t *, partition_t *, int, int, double
							}else{
								p = mapStructLink->substratePath;
								substrateInfrastructure->removeVirtualLinkGivenPath(p,t);
							}
							delete mapStructLink;
						}

						//Move to the next datacenter
						break;
					}

				}//END FOR K LINKS BETWEEN PARTITION I and J

				//If we could embed all the links between the partitions
				if(donForlinkijk){
					//We could embed all the links between ij
					doneForPartitionI = true;
				}else{
					//We could not embed the virtual links between the two partitions i and j
					doneForLinksBetweenPartitionIandJ = false;
					break;
				}
			}//END FOR //j for the previously embedded partitions

			if(doneForLinksBetweenPartitionIandJ){
				//We could embed the virtual links with all the neighboring partitions
				bestDC = currentDC;
				doneForPartitionI = true;
				//Put the mappings in the virtual links
				for(unsigned int z=0; z<linkAllocationList.size(); z++){
					mapping_virtual_link_to_substrate_path* mapStructLink = linkAllocationList.at(z);
					link_t* l = mapStructLink->virtualLink;
					l->mapping = mapStructLink;
					//l->numberOfMappings++;
				}

				//Embed the partition in the best datacenter
				double temp = substrateInfrastructure->dataCenters->at(bestDC)->embedPartition(request->partitions->at(i), t);
				cost += temp;

				//Update the partition Mapping in the partition set in the VDC request
				mapping_parition_to_datacenter* mapStructPartition = new mapping_parition_to_datacenter;
				mapStructPartition->datacenterID = bestDC;
				mapStructPartition->costPerUnitOfTime = costPartitionDC[i][bestDC];
				mapStructPartition->mapBegin = t;
				mapStructPartition->mapEnd = -1;
				mapStructPartition->requestID = request->idRequest;
				mapStructPartition->partitioID = request->partitions->at(i)->partitionID;

				request->partitions->at(i)->mapping = mapStructPartition;
				//request->partitions->at(i)->numberOfMappings++;

				break;

			}else{
				//We could not embed the links for this partition

			}//END IF doneForLinksBetweenPartitionIandJ


		}//End m on the set of possible datacenters

		if(bestDC>-1){
			//We could embed the ith patition
		}else{
			//We could not embed the partition in any of the possible datacenters
			double a=-1,b=-2;
			this->vdcRequestsEmbedded->push_back(request);
			this->removeRequestWithLinksBetweenPartitions(request->originalRequest, t, &a, &b);
			return FAILURE_LINK_EMBED;
		}

	}//END I FOR THE SET OF PARTITIONS

	this->vdcRequestsEmbedded->push_back(request);
	this->nbEmbeddedRequests++;

	//We update the mapping of the links within each partition
	for(int i=0; i<request->numberOfPartitions; i++){
		partition_t* partition  = request->partitions->at(i);
		for(int j=0; j<partition->links; j++){
			link_t* l = partition->link[j];
			l->mapping->DCId = partition->mapping->datacenterID;
			l->mapping->substratePath = NULL;
			l->mapping->mapBegin = t;
			l->mapping->mapEnd = -1;
		}
	}

	//write the request to the database
	if(this->dataBaseConnector != NULL){
		this->dataBaseConnector->writeVDCRequestPartitionedToDataBase(request);
	}

	return cost;


}




double CentralController::embedVDCRequestGreedyLoadBalancing(VDCRequestPartitioned* request, double t){
	/*
	 * Greedy Embedding
	 * Start partition by partition, each time we embed a partition, we embed the corresponding links with the previously embedded partitions
	 */
	cout << "Embed the VDC request of ID : " << request->idRequest << " at time " << t  << endl;
	this->nbTotalRequests++;

	int nbPart = request->numberOfPartitions;
	//unsigned int ub = (int)this->substrateInfrastructure->dataCenters->size();
	//int nbDCs = *((int*)(&ub));
	int nbDCs = this->substrateInfrastructure->numberOfDatacenters;
	//double costPartitionDC[request->numberOfPartitions][substrateInfrastructure->numberOfDatacenters];
	vector<vector<double> > costPartitionDC = vector<vector<double> >(request->numberOfPartitions);
	for(int i = 0; i<nbPart; i++){
		costPartitionDC.at(i) = vector<double>(substrateInfrastructure->numberOfDatacenters);
	}

	vector<vector<int> > bestDatacenter = vector<vector<int> >(request->numberOfPartitions);

	// Build the sorted list of the possible datacenters
	for(int i = 0; i<nbPart; i++){

		//Check whether this partition has location constraint
		vector<location_t*> constrainedNodes = vector<location_t*>();
		for(int k=0; k<request->partitions->at(i)->nodes; k++){
			if(request->partitions->at(i)->nodeLocation[k]->xLocation > -1){
				//This node has a location constraint
				int x = request->partitions->at(i)->nodeLocation[k]->xLocation;
				int y = request->partitions->at(i)->nodeLocation[k]->yLocation;
				if(x > 100 || y>100 || x<0 || y<0){
					cout << "There is a serious problem over here";
				}
				constrainedNodes.push_back(request->partitions->at(i)->nodeLocation[k]);
			}
		}
		bestDatacenter[i] = vector<int>();
		for(int j=0; j<nbDCs; j++){
			bool possible = true;
			for(unsigned int k=0; k<constrainedNodes.size(); k++){

				int x = constrainedNodes.at(k)->xLocation;
				int y = constrainedNodes.at(k)->yLocation;
				if(x > 100 || y>100 || x<0 || y<0){
					cout << "There is a serious problem over here";
				}

				double euclidianDist = (constrainedNodes.at(k)->xLocation - substrateInfrastructure->dataCenters->at(j)->location->xLocation)*(constrainedNodes.at(k)->xLocation - substrateInfrastructure->dataCenters->at(j)->location->xLocation) + (constrainedNodes.at(k)->yLocation - substrateInfrastructure->dataCenters->at(j)->location->yLocation)*(constrainedNodes.at(k)->yLocation - substrateInfrastructure->dataCenters->at(j)->location->yLocation);

				if(euclidianDist > (constrainedNodes.at(k)->maxDistance * constrainedNodes.at(k)->maxDistance)){
					possible = false;
				}
			}
			if(possible){
				double powerInDC = substrateInfrastructure->dataCenters->at(j)->getCostEnergyForPartition(request->partitions->at(i), t);

				if(powerInDC < 0){
					cout << "There is a serious problem over here";
				}


				costPartitionDC[i][j] = (substrateInfrastructure->dataCenters->at(j)->getInstantneousUtilization(t))->numberOfVMs;
				//We choose the datacenter according to PUE*(electricityPrice + CarbonCoeff*CarbonCost)

				if(costPartitionDC[i][j] < 0){
					//It is not possible to embed this partition in this datacenter, the datacenter is overloaded
					costPartitionDC[i][j] = INFINITY;
				}else{
					//Insert the datacenter in the sorted queue;
					bool doneDCij = false;
					for(unsigned int k=0; k<bestDatacenter[i].size(); k++){
						if (costPartitionDC[i][bestDatacenter[i].at(k)] > costPartitionDC[i][j]){
							bestDatacenter[i].insert(bestDatacenter[i].begin()+k, j);
							doneDCij = true;
							break;
						}
					}
					if(!doneDCij){
						bestDatacenter[i].push_back(j);
					}

				}
			}else{
				//This datacenter is not possible
				costPartitionDC[i][j] = INFINITY;
			}
		}



		//See if there is at least one possible datacenter
		if(bestDatacenter[i].empty()){
			//There is no possible datacenter

			return FAILURE_PARTITION_EMBED;
		}
	}//END FOR i<nbPart

	//Start partition Embedding
	double cost=0.0;
	//vector<vector<mapping_virtual_link_to_substrate_path*> > linkAllocationList = vector<vector<mapping_virtual_link_to_substrate_path*> >(request->numberOfPartitions);
	//Add the request to the list of embedded requests
	for(int i=0; i<request->numberOfPartitions; i++){

		bool doneForPartitionI = true;
		int bestDC = -1;
		for(unsigned int m=0; m<bestDatacenter[i].size(); m++){ //See the best datacenter we find

			int currentDC = bestDatacenter[i].at(m);

			//linkAllocationList[i] = vector<mapping_virtual_link_to_substrate_path*>();
			vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path*>();

			bool doneForLinksBetweenPartitionIandJ = true;

			for(int j=0; j<i; j++){ //j for the previously embedded partitions

				//int DCj = request->partitions->at(j)->mappings[request->partitions->at(j)->numberOfMappings-1]->datacenterID;
				int DCj = request->partitions->at(j)->mapping->datacenterID;

				//vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path>();
				bool donForlinkijk = true;
				vector<link_t*> linksBetweenIJ = vector<link_t*>();
				for(unsigned int k=0; k<request->linksBetweenPartirions[j][i]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[j][i]->at(k));
				}
				for(unsigned int k=0; k<request->linksBetweenPartirions[i][j]->size(); k++){
					linksBetweenIJ.push_back(request->linksBetweenPartirions[i][j]->at(k));
				}


				for(unsigned int k=0; k<linksBetweenIJ.size(); k++){

					link_t* l =linksBetweenIJ.at(k);
					double cost2 = 0;
					path_t* p = NULL;
					int DC = -1;
					if(currentDC == DCj){
						//The two partitions are embedded in the same datacenter
						DC = DCj;
						cost2 = substrateInfrastructure->dataCenters->at(currentDC)->embedLink(l, t);
					}
					else{
						//The partitions are embedded in different datacenters, embed the links between them in the provisioned network
						p = substrateInfrastructure->embedVirtualLinkBetweenDataCenters(currentDC, DCj,l->bw, l->delay, t, &cost2);
					}

					if(p!= NULL || DC > -1){
						//Add the mapping structure to the list of mapped links in the provisioned network
						mapping_virtual_link_to_substrate_path* mapStructLink = new mapping_virtual_link_to_substrate_path;
						mapStructLink->mapBegin = t;
						mapStructLink->mapEnd = -1;
						mapStructLink->costPerUnitOfTime = cost2;
						mapStructLink->requestID = request->idRequest;
						mapStructLink->substratePath = p;
						mapStructLink->DCId = DC;
						mapStructLink->virtualLink = l;

						linkAllocationList.push_back(mapStructLink);

					}else{
						//We could not embed the virtual link in the provisioned network
						//Remove all the links and nodes that have been embedded before
						donForlinkijk = false;
						for(unsigned int z=0; z<linkAllocationList.size(); z++){
							mapping_virtual_link_to_substrate_path* mapStructLink = linkAllocationList.at(z);
							if(mapStructLink->DCId >= 0){
								substrateInfrastructure->dataCenters->at(mapStructLink->DCId)->removeLink(mapStructLink->virtualLink, t); //request_t *, partition_t *, partition_t *, int, int, double
							}else{
								p = mapStructLink->substratePath;
								substrateInfrastructure->removeVirtualLinkGivenPath(p,t);
							}
							delete mapStructLink;
						}

						//Move to the next datacenter
						break;
					}

				}//END FOR K LINKS BETWEEN PARTITION I and J

				//If we could embed all the links between the partitions
				if(donForlinkijk){
					//We could embed all the links between ij
					doneForPartitionI = true;
				}else{
					//We could not embed the virtual links between the two partitions i and j
					doneForLinksBetweenPartitionIandJ = false;
					break;
				}
			}//END FOR //j for the previously embedded partitions

			if(doneForLinksBetweenPartitionIandJ){
				//We could embed the virtual links with all the neighboring partitions
				bestDC = currentDC;
				doneForPartitionI = true;
				//Put the mappings in the virtual links
				for(unsigned int z=0; z<linkAllocationList.size(); z++){
					mapping_virtual_link_to_substrate_path* mapStructLink = linkAllocationList.at(z);
					link_t* l = mapStructLink->virtualLink;
					l->mapping = mapStructLink;
					//l->numberOfMappings++;
				}

				//Embed the partition in the best datacenter
				double temp = substrateInfrastructure->dataCenters->at(bestDC)->embedPartition(request->partitions->at(i), t);
				cost += temp;

				//Update the partition Mapping in the partition set in the VDC request
				mapping_parition_to_datacenter* mapStructPartition = new mapping_parition_to_datacenter;
				mapStructPartition->datacenterID = bestDC;
				mapStructPartition->costPerUnitOfTime = costPartitionDC[i][bestDC];
				mapStructPartition->mapBegin = t;
				mapStructPartition->mapEnd = -1;
				mapStructPartition->requestID = request->idRequest;
				mapStructPartition->partitioID = request->partitions->at(i)->partitionID;

				request->partitions->at(i)->mapping = mapStructPartition;
				//request->partitions->at(i)->numberOfMappings++;

				break;

			}else{
				//We could not embed the links for this partition

			}//END IF doneForLinksBetweenPartitionIandJ


		}//End m on the set of possible datacenters

		if(bestDC>-1){
			//We could embed the ith patition
		}else{
			//We could not embed the partition in any of the possible datacenters
			double a=-1,b=-2;
			this->vdcRequestsEmbedded->push_back(request);
			this->removeRequestWithLinksBetweenPartitions(request->originalRequest, t, &a, &b);
			return FAILURE_LINK_EMBED;
		}

	}//END I FOR THE SET OF PARTITIONS

	this->vdcRequestsEmbedded->push_back(request);
	this->nbEmbeddedRequests++;

	//We update the mapping of the links within each partition
	for(int i=0; i<request->numberOfPartitions; i++){
		partition_t* partition  = request->partitions->at(i);
		for(int j=0; j<partition->links; j++){
			link_t* l = partition->link[j];
			l->mapping->DCId = partition->mapping->datacenterID;
			l->mapping->substratePath = NULL;
			l->mapping->mapBegin = t;
			l->mapping->mapEnd = -1;
		}
	}

	//Remove the request from the database
	if(this->dataBaseConnector != NULL){
		this->dataBaseConnector->writeVDCRequestPartitionedToDataBase(request);
	}

	return cost;

}








double CentralController::embedFromGivenEmbeddingSolution(EmbeddingSolution* embeddingSolution, double t) {

	//cout << "Embed the VDC request of ID : " << embeddingSolution->requestsPartitioned->idRequest << " at time " << t  << endl;

	double cost = 0;
	VDCRequestPartitioned* request = embeddingSolution->requestsPartitioned;
	this->nbTotalRequests++;

	for(int i=0; i<embeddingSolution->requestsPartitioned->numberOfPartitions; i++){

		bool doneForPartitionI = true;


		int currentDC = embeddingSolution->assignedPartitionToDatacenter.at(i);

		//linkAllocationList[i] = vector<mapping_virtual_link_to_substrate_path*>();
		vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path*>();

		bool doneForLinksBetweenPartitionIandJ = true;

		for(int j=0; j<i; j++){ //j for the previously embedded partitions

			int DCj = request->partitions->at(j)->mapping->datacenterID;

			//vector<mapping_virtual_link_to_substrate_path*> linkAllocationList = vector<mapping_virtual_link_to_substrate_path>();
			bool doneForlinkijk = true;
			vector<link_t*> linksBetweenIJ = vector<link_t*>();
			for(unsigned int k=0; k<request->linksBetweenPartirions[j][i]->size(); k++){
				linksBetweenIJ.push_back(request->linksBetweenPartirions[j][i]->at(k));
			}
			for(unsigned int k=0; k<request->linksBetweenPartirions[i][j]->size(); k++){
				linksBetweenIJ.push_back(request->linksBetweenPartirions[i][j]->at(k));
			}


			for(unsigned int k=0; k<linksBetweenIJ.size(); k++){

				link_t* l =linksBetweenIJ.at(k);
				double cost2 = 0;
				path_t* p = NULL;
				int DC = -1;
				if(currentDC == DCj){
					//The two partitions are embedded in the same datacenter
					DC = DCj;
					cost2 = substrateInfrastructure->dataCenters->at(currentDC)->embedLink(l, t);
				}
				else{
					//The partitions are embedded in different datacenters, embed the links between them in the provisioned network
					p = substrateInfrastructure->embedVirtualLinkBetweenDataCenters(currentDC, DCj,l->bw, l->delay, t, &cost2);
				}

				if(p!= NULL || DC > -1){
					//Add the mapping structure to the list of mapped links in the provisioned network
					mapping_virtual_link_to_substrate_path* mapStructLink = new mapping_virtual_link_to_substrate_path;
					mapStructLink->mapBegin = t;
					mapStructLink->mapEnd = -1;
					mapStructLink->costPerUnitOfTime = cost2;
					mapStructLink->requestID = request->idRequest;
					mapStructLink->substratePath = p;
					mapStructLink->DCId = DC;
					mapStructLink->virtualLink = l;

					linkAllocationList.push_back(mapStructLink);

				}else{
					//We could not embed the virtual link in the provisioned network
					//Remove all the links and nodes that have been embedded before
					doneForlinkijk = false;
					for(unsigned int z=0; z<linkAllocationList.size(); z++){
						mapping_virtual_link_to_substrate_path* mapStructLink = linkAllocationList.at(z);
						if(mapStructLink->DCId >= 0){
							substrateInfrastructure->dataCenters->at(mapStructLink->DCId)->removeLink(mapStructLink->virtualLink, t); //request_t *, partition_t *, partition_t *, int, int, double
						}else{
							p = mapStructLink->substratePath;
							substrateInfrastructure->removeVirtualLinkGivenPath(p,t);
						}
						delete mapStructLink;
					}

					//Move to the next datacenter
					break;
				}

			}//END FOR K LINKS BETWEEN PARTITION I and J

			//If we could embed all the links between the partitions
			if(doneForlinkijk){
				//We could embed all the links between ij
				doneForPartitionI = true;
			}else{
				//We could not embed the virtual links between the two partitions i and j
				doneForLinksBetweenPartitionIandJ = false;
				break;
			}
		}//END FOR //j for the previously embedded partitions

		if(doneForLinksBetweenPartitionIandJ){
			//We could embed the virtual links with all the neighboring partitions

			doneForPartitionI = true;
			//Put the mappings in the virtual links
			for(unsigned int z=0; z<linkAllocationList.size(); z++){
				mapping_virtual_link_to_substrate_path* mapStructLink = linkAllocationList.at(z);
				link_t* l = mapStructLink->virtualLink;
				l->mapping = mapStructLink;
				//l->numberOfMappings++;
			}

			//Embed the partition in the best datacenter
			double temp = substrateInfrastructure->dataCenters->at(currentDC)->embedPartition(request->partitions->at(i), t);
			cost += temp;

			//Update the partition Mapping in the partition set in the VDC request
			mapping_parition_to_datacenter* mapStructPartition = new mapping_parition_to_datacenter;
			mapStructPartition->datacenterID = currentDC;
			mapStructPartition->costPerUnitOfTime =  substrateInfrastructure->dataCenters->at(currentDC)->getCostPerUnitOfPowerAndTime(t);
			mapStructPartition->mapBegin = t;
			mapStructPartition->mapEnd = -1;
			mapStructPartition->requestID = request->idRequest;
			mapStructPartition->partitioID = request->partitions->at(i)->partitionID;

			request->partitions->at(i)->mapping = mapStructPartition;
			//request->partitions->at(i)->numberOfMappings++;


		}else{
			//We could not embed the links for this partition
			doneForPartitionI = false;
		}//END IF doneForLinksBetweenPartitionIandJ




		if(doneForPartitionI){
			//We could embed the ith patition

		}else{
			//We could not embed the partition in any of the possible datacenters
			double a=-1,b=-2;
			this->vdcRequestsEmbedded->push_back(request);
			this->removeRequestWithLinksBetweenPartitions(request->originalRequest, t, &a, &b);
			return FAILURE_LINK_EMBED;
		}

	}//END I FOR THE SET OF PARTITIONS

	this->vdcRequestsEmbedded->push_back(request);
	this->nbEmbeddedRequests++;

	//We update the mapping of the links within each partition
	for(int i=0; i<request->numberOfPartitions; i++){
		partition_t* partition  = request->partitions->at(i);
		for(int j=0; j<partition->links; j++){
			link_t* l = partition->link[j];
			l->mapping->DCId = partition->mapping->datacenterID;
			l->mapping->substratePath = NULL;
			l->mapping->mapBegin = t;
			l->mapping->mapEnd = -1;
		}
	}

	//Remove the request from the database
	if(this->dataBaseConnector != NULL){
		this->dataBaseConnector->writeVDCRequestPartitionedToDataBase(request);
	}

	return cost;

}




double CentralController::embedVDCRequestToGetOptimalSolution(VDCRequestPartitioned* request, double t) {

	vector<EmbeddingSolution*>* solutions = BranchAndBound::generateAllPossibleSolutions(request,this->substrateInfrastructure);

	EmbeddingSolution* solMin = solutions->front();
	double costMin=this->embedFromGivenEmbeddingSolutionWithoutEmbeddingLinksInBackboneNetwork(solMin,t);
	double a,b;
	this->removeRequestWithoutRemovingLinks(solMin->requestsPartitioned->originalRequest, t, &a, &b, false);
	solutions->erase(solutions->begin());

	while(solutions->size()>0){
		int s = solutions->size();
		EmbeddingSolution* solFront = solutions->front();
		double costFront=this->embedFromGivenEmbeddingSolutionWithoutEmbeddingLinksInBackboneNetwork(solFront,t);
		this->removeRequestWithoutRemovingLinks(solFront->requestsPartitioned->originalRequest, t, &a, &b, false);
		solutions->erase(solutions->begin());
		if((costMin<0 && costFront>0) || (costFront>0 && costFront<costMin)){
			//This solution is better than the previous so far
			delete solMin;
			costMin = costFront;
			solMin = solFront;
		}else{
			//This Front solution is not feasible, just erase it
			delete solFront;
		}
	}
	int k=0;
	costMin = this->embedFromGivenEmbeddingSolutionWithoutEmbeddingLinksInBackboneNetwork(solMin,t);
	return costMin;
}




double CentralController::embedFromGivenEmbeddingSolutionWithoutEmbeddingLinksInBackboneNetwork(EmbeddingSolution* embeddingSolution, double t) {

	//cout << "Embed the VDC request of ID : " << embeddingSolution->requestsPartitioned->idRequest << " at time " << t  << endl;
	//Embed from the given solution without considering the links in the backbone network. This an "ideal optimal" solution

	double cost = 0;
	VDCRequestPartitioned* request = embeddingSolution->requestsPartitioned;
	this->nbTotalRequests++;

	for(int i=0; i<embeddingSolution->requestsPartitioned->numberOfPartitions; i++){

		int currentDC = embeddingSolution->assignedPartitionToDatacenter.at(i);
		//We could embed the virtual links with all the neighboring partitions

		//Put the mappings in the virtual links

		//Embed the partition in the best datacenter
		double temp = substrateInfrastructure->dataCenters->at(currentDC)->embedPartition(request->partitions->at(i), t);
		cost += temp;

		//Update the partition Mapping in the partition set in the VDC request
		mapping_parition_to_datacenter* mapStructPartition = new mapping_parition_to_datacenter;
		mapStructPartition->datacenterID = currentDC;
		mapStructPartition->costPerUnitOfTime =  substrateInfrastructure->dataCenters->at(currentDC)->getCostPerUnitOfPowerAndTime(t);
		mapStructPartition->mapBegin = t;
		mapStructPartition->mapEnd = -1;
		mapStructPartition->requestID = request->idRequest;
		mapStructPartition->partitioID = request->partitions->at(i)->partitionID;

		request->partitions->at(i)->mapping = mapStructPartition;
		//request->partitions->at(i)->numberOfMappings++;

		if(temp>=0){
			//We could embed the ith patition

		}else{
			//We could not embed the partition in any of the possible datacenters
			double a=-1,b=-2;
			this->vdcRequestsEmbedded->push_back(request);
			this->removeRequestWithoutRemovingLinks(request->originalRequest, t, &a, &b, false);
			return FAILURE_PARTITION_EMBED;
		}

	}//END I FOR THE SET OF PARTITIONS

	this->vdcRequestsEmbedded->push_back(request);
	this->nbEmbeddedRequests++;

	//We update the mapping of the links within each partition
	for(int i=0; i<request->numberOfPartitions; i++){
		partition_t* partition  = request->partitions->at(i);
		for(int j=0; j<partition->links; j++){
			link_t* l = partition->link[j];
			l->mapping->DCId = partition->mapping->datacenterID;
			l->mapping->substratePath = NULL;
			l->mapping->mapBegin = t;
			l->mapping->mapEnd = -1;
		}
	}

	//Remove the request from the database
	if(this->dataBaseConnector != NULL){
		this->dataBaseConnector->writeVDCRequestPartitionedToDataBase(request);
	}

	return cost;

}



vector<datacenter_utilization_t*>* CentralController::getInstaneousUtilizationInDatacenters(double t) {
	/*
	 * Returns the current utilization in the different datacenters
	 */

	vector<datacenter_utilization_t*>* result = new vector<datacenter_utilization_t*>();
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		datacenter_utilization_t* instUtil = this->substrateInfrastructure->dataCenters->at(i)->getInstantneousUtilization(t);
		infrastructure_utilization_t* res = this->getInstanteousOperatingCostInDataCenter(t, i);
		instUtil->instantaneousCost = res->costDatacenters;
		result->push_back(instUtil);
	}

	return result;

}


sla_violation_summary_t* CentralController::getViolatedSLAsSummaryForAServiceGuaranteePeriod(double t) {
	vector<sla_violation_per_request_t*>* violationList = this->getListViolatedSLAsForAServiceGuaranteePeriod(t);
	sla_violation_summary_t* result = new sla_violation_summary_t;
	result->tEnd = t;
	double violationCarbon = 0;
	for(unsigned int i=0; i<violationList->size(); i++){
		violationCarbon+=violationList->at(i)->amountOfAdditionalCarbonGenerated;
	}
	result->totalCarbonViolation = violationCarbon;

	double violationCost = 0;
	for(unsigned int i=0; i<violationList->size(); i++){
		violationCost+=violationList->at(i)->cost;
	}
	result->totalPenaltyCostToPay = violationCost;

	result->numberOfRequestViolated = violationList->size();

	return result;
}

vector<sla_violation_per_request_t*>* CentralController::getListViolatedSLAsForAServiceGuaranteePeriod(double t){
	/*
	 * Returns the list of request whose SLAs in terms of Carbon Emissions are violated
	 * t is the beginning of the next time slot
	 */

	double carbonPerVMUnit = this->getCarbonFootprintPerUnitOfVMInAllDatacentersForAServiceGuaranteePeriod(t);
	double carbonPerBWUnit = this->getCarbonFootprintPerUnitOfBandwidthInAllDatacentersForAServiceGuaranteePeriod(t);

	vector<sla_violation_per_request_t*>* result = new vector<sla_violation_per_request_t*>();

	//For the requests that are actually in the system
	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		double arrival = this->vdcRequestsEmbedded->at(i)->originalRequest->arrtime;
		double departure = arrival +  this->vdcRequestsEmbedded->at(i)->originalRequest->duration;
		if(arrival<t && departure>t - centralControllerParameters->SLAServiceGuaranteePeriod){
			double infBound = t - centralControllerParameters->SLAServiceGuaranteePeriod;
			if(infBound<0)
				infBound = 0;
			if(infBound<arrival)
				infBound = arrival;
			double supBound = t;
			if(departure<t)
				supBound = departure;

			double durationInPeriod = supBound-infBound;

			double carbonEmissionForRequest = this->getCarbonFootrpintForARequest(this->vdcRequestsEmbedded->at(i)->originalRequest, carbonPerVMUnit,carbonPerBWUnit);
			carbonEmissionForRequest  = carbonEmissionForRequest*durationInPeriod;
			if(carbonEmissionForRequest > this->vdcRequestsEmbedded->at(i)->originalRequest->limitOfCarbonEmissionPerServicePeriodGuarantee){
				//This request have been violated
				sla_violation_per_request_t* strct = new sla_violation_per_request_t;
				strct->request =  this->vdcRequestsEmbedded->at(i)->originalRequest;
				strct->amountOfAdditionalCarbonGenerated = carbonEmissionForRequest - this->vdcRequestsEmbedded->at(i)->originalRequest->limitOfCarbonEmissionPerServicePeriodGuarantee;
				//strct->cost = strct->amountOfAdditionalCarbonGenerated*this->SLAViolationCostPerUnitOfCarbon;
				strct->cost = strct->request->revenue*this->centralControllerParameters->SLAViolationProportionOfRevenueToRefund*durationInPeriod;
				strct->tEnd = t;
				result->push_back(strct);
			}
		}
	}

	//For the requests that are actually in the system
	for(unsigned int i=0; i<this->vdcRequestsPreviouslyEmbedded->size(); i++){
		double arrival = this->vdcRequestsPreviouslyEmbedded->at(i)->originalRequest->arrtime;
		double departure = arrival +  this->vdcRequestsPreviouslyEmbedded->at(i)->originalRequest->duration;
		if(arrival<t && departure>t-centralControllerParameters->SLAServiceGuaranteePeriod){
			double infBound = t-centralControllerParameters->SLAServiceGuaranteePeriod;
			if(infBound<arrival)
				infBound = arrival;
			double supBound = t;
			if(departure>t)
				supBound = departure;

			double durationInPeriod = supBound-infBound;

			double carbonEmissionForRequest = this->getCarbonFootrpintForARequest(this->vdcRequestsPreviouslyEmbedded->at(i)->originalRequest, carbonPerVMUnit,carbonPerBWUnit);
			carbonEmissionForRequest  = carbonEmissionForRequest*durationInPeriod;
			if(carbonEmissionForRequest > this->vdcRequestsPreviouslyEmbedded->at(i)->originalRequest->limitOfCarbonEmissionPerServicePeriodGuarantee){
				//This request have been violated
				sla_violation_per_request_t* strct = new sla_violation_per_request_t;
				strct->request =  this->vdcRequestsPreviouslyEmbedded->at(i)->originalRequest;
				strct->amountOfAdditionalCarbonGenerated = carbonEmissionForRequest - this->vdcRequestsPreviouslyEmbedded->at(i)->originalRequest->limitOfCarbonEmissionPerServicePeriodGuarantee;
				strct->cost = strct->request->revenue*this->centralControllerParameters->SLAViolationProportionOfRevenueToRefund*durationInPeriod;
				strct->tEnd = t;
				result->push_back(strct);
			}
		}
	}

	return result;
}



double CentralController::getCarbonFootprintPerUnitOfVMInAllDatacenters(double t) {
	double sumCarbon = 0;
	double sumUsedCPU = 0;
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		sumCarbon += this->substrateInfrastructure->dataCenters->at(i)->getAmountOfCarbonOfServers(t);
		sumUsedCPU += this->substrateInfrastructure->dataCenters->at(i)->actualUtilization->totalUsedCPU;
	}
	double res = 0;
	if(sumUsedCPU > 0) res = sumCarbon/sumUsedCPU;
	return res;
}


double CentralController::getEstiamtionCarbonFootprintPerUnitOfVMInAllDatacentersForAPeriodOfTime(double tBegin, double tEnd) {
	double sumCarbon = 0;
	double sumUsedCPU = 0;
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		double availableRenwables =  this->substrateInfrastructure->dataCenters->at(i)->getEstimationOfAmountOfRenewables(tBegin, tEnd)/(tEnd - tBegin);
		double consumedPower = this->substrateInfrastructure->dataCenters->at(i)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(tBegin, tEnd)*this->substrateInfrastructure->dataCenters->at(i)->getPUE(tBegin);
		if(availableRenwables < consumedPower){
			sumCarbon += ((consumedPower - availableRenwables)/1000)*this->substrateInfrastructure->dataCenters->at(i)->getCarbonFootPrintCoefficient(tBegin)*this->substrateInfrastructure->dataCenters->at(i)->fractionOfPowerForServers;
		}
		sumUsedCPU += this->substrateInfrastructure->dataCenters->at(i)->getEstimationOfAverageCPUUsedDuringUpcomingTimePeriod(tBegin, tEnd);
	}
	double res = 0;
	if(sumUsedCPU > 0) res = sumCarbon/sumUsedCPU;
	return res;
}


double CentralController::getEstiamtionCarbonFootprintPerUnitOfBandwidthInWholeInfrastructureForAPeriodOfTime(double tBegin, double tEnd){
	double sumCarbon = 0;
	//In The Data Centers
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		double availableRenwables =  this->substrateInfrastructure->dataCenters->at(i)->getEstimationOfAmountOfRenewables(tBegin, tEnd)/(tEnd - tBegin);
		double consumedPower = this->substrateInfrastructure->dataCenters->at(i)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(tBegin, tEnd)*this->substrateInfrastructure->dataCenters->at(i)->getPUE(tBegin);
		if(availableRenwables < consumedPower){
			sumCarbon +=  ((consumedPower-availableRenwables)/1000)*this->substrateInfrastructure->dataCenters->at(i)->getCarbonFootPrintCoefficient(tBegin)*(1-this->substrateInfrastructure->dataCenters->at(i)->fractionOfPowerForServers);
		}
	}
	//In The Backbone Network
	sumCarbon += this->substrateInfrastructure->getCarbonFootprintOfBackboneNetwork(this->centralControllerParameters->carbonPerUnitOfBandwidthInBackbone);
	//sumUsedBW += this->substrateInfrastructure->getSumUsedBandwidth();

	double sumUsedBW = 0;

	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		VDCRequestPartitioned* request = this->vdcRequestsEmbedded->at(i);
		double bwForRequest = 0;

		for(int k=0; k<request->originalRequest->links; k++){
			bwForRequest += request->originalRequest->link[k]->bw;
		}

		double endTime = request->originalRequest->arrtime + request->originalRequest->duration;
		if(endTime > tEnd){
			endTime = tEnd;
		}
		double timeVDCIsThere = endTime-tBegin;
		bwForRequest = bwForRequest*timeVDCIsThere;
		sumUsedBW += bwForRequest;
	}

	sumUsedBW = sumUsedBW/(tEnd-tBegin);
	double res = 0;
	if(sumUsedBW > 0) res = sumCarbon/sumUsedBW;
	return res;
}




/*
double CentralController::getCarbonFootprintPerUnitOfBandwidthInAllDatacenters(double t) {
	double sumCarbon = 0;
	double sumUsedBW = 0;
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		sumCarbon += this->substrateInfrastructure->dataCenters->at(i)->getAmountOfCarbonOfNetwork(t);
		sumUsedBW += this->substrateInfrastructure->dataCenters->at(i)->actualUtilization->totalUsedBandwidth;
	}
	double res = 0;
	if(sumUsedBW > 0) res = sumCarbon/sumUsedBW;
	return res;
}
 */




double CentralController::getCarbonFootprintPerUnitOfBandwidthInWholeInfrastructure(double t) {
	double sumCarbon = 0;
	double sumUsedBW = 0;
	//In The Data Centers
	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		sumCarbon += this->substrateInfrastructure->dataCenters->at(i)->getAmountOfCarbonOfNetwork(t);
		sumUsedBW += this->substrateInfrastructure->dataCenters->at(i)->actualUtilization->totalUsedBandwidth;
	}

	//In The Backbone Network
	sumCarbon += this->substrateInfrastructure->getCarbonFootprintOfBackboneNetwork(this->centralControllerParameters->carbonPerUnitOfBandwidthInBackbone);
	sumUsedBW += this->substrateInfrastructure->getSumUsedBandwidth();

	double res = 0;
	if(sumUsedBW > 0) res = sumCarbon/sumUsedBW;
	return res;
}




int CentralController::getPeriodFromTime(double t) {
	double res = t/this->centralControllerParameters->SLAServiceGuaranteePeriod;
	int result = (int)(floor(res));
	return result;
}

int CentralController::getPeriodFromTimeSlot(int slot) {
	double res = ((double)slot)/(this->centralControllerParameters->SLAServiceGuaranteePeriod/this->centralControllerParameters->timeSlotDuration);
	int result = (int)(floor(res));
	return result;
}

double CentralController::getCarbonFootprintPerUnitOfVMInAllDatacentersForAServiceGuaranteePeriod(double t) {
	/*
	 * Gives the carbon emission per unit of VM in data centers for the reporting period that ends at t
	 * t is the last time (last time slot, last second, last minute...) of the reporting period to which t belongs
	 */
	double sum = 0;
	double sumCPU = 0;
	int i = this->wholeInfrastructureOverTime->size()-1;
	while(i>=0 && this->wholeInfrastructureOverTime->at(i)->tBegin >= (t-this->centralControllerParameters->SLAServiceGuaranteePeriod)){
		//sum += this->wholeInfrastructureOverTime->at(i)->carbonEmissionPerUnitOfVM;
		sum += this->wholeInfrastructureOverTime->at(i)->carbonEmissionPerUnitOfVM*this->wholeInfrastructureOverTime->at(i)->totalUsedCPU;
		sumCPU += this->wholeInfrastructureOverTime->at(i)->totalUsedCPU;
		i--;
	}
	double result = sum/sumCPU;
	return result;
}


double CentralController::getCarbonFootprintPerUnitOfBandwidthInAllDatacentersForAServiceGuaranteePeriod(double t) {
	/*
	 * Gives the carbon emission per unit of Bandwidth in data centers for the reporting period that ends at t
	 * t is the last time (last time slot, last second, last minute...) of the reporting period to which t belongs
	 */

	double sum = 0;
	double sumBandwidth = 0;
	int i = this->wholeInfrastructureOverTime->size()-1;
	while(i>=0 && this->wholeInfrastructureOverTime->at(i)->tBegin >= (t-this->centralControllerParameters->SLAServiceGuaranteePeriod)){
		//sum += this->wholeInfrastructureOverTime->at(i)->carbonEmissionPerUnitOfBandwidth;
		sum += this->wholeInfrastructureOverTime->at(i)->carbonEmissionPerUnitOfBandwidth*this->wholeInfrastructureOverTime->at(i)->totalUsedBandwidth;
		sumBandwidth += this->wholeInfrastructureOverTime->at(i)->totalUsedBandwidth;
		i--;
	}
	double result = sum/sumBandwidth;
	return result;
}


double CentralController::getInstantaneousCarbonFootrpintForARequest(
		request_t* request, double t) {

	double carbonVM = this->getCarbonFootprintPerUnitOfVMInAllDatacenters(t);
	double carbonBW = this->getCarbonFootprintPerUnitOfBandwidthInWholeInfrastructure(t);

	double sumCarbon = 0;
	//Cabon for the VMs
	for(int i=0; i<request->nodes; i++){
		sumCarbon += carbonVM*request->cpu[i];
	}
	//Cabon for links
	for(int i=0; i<request->links; i++){
		sumCarbon += carbonBW*request->link[i]->bw;
	}

	return sumCarbon;
}


/*
double CentralController::getCarbonFootrpintForARequestForAServiceGuaranteePeriod(request_t* request, double t) {

	return 0;
}
 */

double CentralController::getCarbonFootrpintForARequest(request_t* request, double CarbonPerVMUnit, double CarbonPerBWUnit) {
	double sumCarbon = 0;
	//Carbon for the VMs
	for(int i=0; i<request->nodes; i++){
		sumCarbon += CarbonPerVMUnit*request->cpu[i];
	}
	//Carbon for links
	for(int i=0; i<request->links; i++){
		sumCarbon += CarbonPerBWUnit*request->link[i]->bw;
	}

	return sumCarbon;
}


int CentralController::getTimeSlotFromTime(double t) {
	double res = t/this->centralControllerParameters->timeSlotDuration;
	int result = (int)(floor(res));
	return result;
}


//Migration


double CentralController::getCostInBackboneToMigratePartition(partition_t* partition, VDCRequestPartitioned* requestPartitioned, int dataCenterSource, int dataCenterDestination){
	/*
	 * Returns the cost in the backbone network in case we want to migrate the partition
	 * The method does not migrate the partition
	 */

	//get the index of the partition
	int indexPartition = -1;
	for(int i=0; i<requestPartitioned->numberOfPartitions; i++){
		if(requestPartitioned->partitions->at(i)->partitionID == partition->partitionID){
			indexPartition = i;
			break;
		}
	}

	if(indexPartition == -1){
		return 9999999999;
	}

	//get the sum of bandwidth
	double sumBW = 0;
	for(int i=0; i<requestPartitioned->numberOfPartitions; i++){
		//The partition is not embedded in the data center destination, which means the link should be embedded in
		if(i != indexPartition && requestPartitioned->partitions->at(i)->mapping->datacenterID != dataCenterDestination){

			for(unsigned int j=0; j<requestPartitioned->linksBetweenPartirions[indexPartition][i]->size(); j++){
				sumBW += requestPartitioned->linksBetweenPartirions[indexPartition][i]->at(j)->bw;
			}
			for(unsigned int j=0; j<requestPartitioned->linksBetweenPartirions[i][indexPartition]->size(); j++){
				sumBW += requestPartitioned->linksBetweenPartirions[i][indexPartition]->at(j)->bw;
			}

		}
	}
	return sumBW;
}




double CentralController::migratePartition(partition_t* partition, VDCRequestPartitioned* requestPartitioned, int dataCenterSource, int dataCenterDestination, double t) {

	/*
	 * Migrate the partition and its pending links from dataCenterSource to dataCenterDestination
	 * Returns the cost of migration if successful, and returns FAILURE_MIGRATE if migration could be completed
	 */

	//Check if the destination data center can accomodate the partition

	if(partition->request->reqID == 20){

		for(int i=0; i<requestPartitioned->numberOfPartitions; i++){
			partition_t* part = requestPartitioned->partitions->at(i);
			int k = 0;
			int x=0; //Breakpoint here
		}


	}

	if(dataCenterSource == dataCenterDestination){
		return 0;
	}

	if(!this->substrateInfrastructure->isPossibleToEmbedPartitionInDataCenter(partition, dataCenterDestination)){
		return FAILURE_PARTITION_LOCATION_CONSTRAINT_VIOLATION;
	}

	int embedInDC = this->substrateInfrastructure->getDataCenters()->at(dataCenterDestination)->embedPartition(partition, t);

	if(embedInDC == FAILURE_PARTITION_EMBED){
		//Failed to embed the partition in the data center
		return FAILURE_MIGRATE;
	}

	//The Partition can be accommodated in the data center destination

	this->substrateInfrastructure->getDataCenters()->at(dataCenterSource)->removePartition(partition, t);
	partition->mapping->datacenterID = dataCenterDestination;
	//Migrate the pending links in the backbone network

	vector<link_t*> pendingLinks = vector<link_t*>();
	vector< mapping_virtual_link_to_substrate_path*>* oldMappings = new vector<mapping_virtual_link_to_substrate_path*>();
	vector<int> dataCenterOfPendingLinks = vector<int>();

	// Get the index of the partition
	int indexPartition = -1;
	for(int i=0; i<requestPartitioned->numberOfPartitions; i++){
		if(requestPartitioned->partitions->at(i)->partitionID == partition->partitionID){
			indexPartition = i;
			break;
		}
	}

	//Get the pending links
	for(int i=0; i<requestPartitioned->numberOfPartitions; i++){
		if(indexPartition != i){
			for(unsigned int j=0; j<requestPartitioned->linksBetweenPartirions[i][indexPartition]->size(); j++){
				link_t* l = requestPartitioned->linksBetweenPartirions[i][indexPartition]->at(j);
				pendingLinks.push_back(l);
				mapping_virtual_link_to_substrate_path* map = requestPartitioned->linksBetweenPartirions[i][indexPartition]->at(j)->mapping;
				mapping_virtual_link_to_substrate_path* mapping = new mapping_virtual_link_to_substrate_path;
				mapping->DCId = map->DCId;
				mapping->substratePath = map->substratePath;
				mapping->requestID = map->requestID;
				mapping->virtualLink = map->virtualLink;
				oldMappings->push_back(mapping);
				if(mapping->DCId != -1){
					dataCenterOfPendingLinks.push_back(mapping->DCId);
				}else{
					int DC2 = mapping->substratePath->link[mapping->substratePath->len-1];
					int DC1 = mapping->substratePath->link[0];
					if(DC1 == dataCenterSource){
						dataCenterOfPendingLinks.push_back(DC2);
					}else{
						if(DC2 == dataCenterSource){
							dataCenterOfPendingLinks.push_back(DC1);
						}else{
							cout << "OUCH, THERE IS A SERIOUS PROBLEM, THE LINK SEEMS TO ORIGINATE FROM A DATA CENTER OTHER THAN THE ONE OF THE PARTITION"<<endl;
						}
					}
				}
			}
			for(unsigned int j=0; j<requestPartitioned->linksBetweenPartirions[indexPartition][i]->size(); j++){
				link_t* l = requestPartitioned->linksBetweenPartirions[indexPartition][i]->at(j);
				pendingLinks.push_back(l);
				mapping_virtual_link_to_substrate_path* map = requestPartitioned->linksBetweenPartirions[indexPartition][i]->at(j)->mapping;
				mapping_virtual_link_to_substrate_path* mapping = new mapping_virtual_link_to_substrate_path;
				mapping->requestID = map->requestID;
				mapping->virtualLink = map->virtualLink;
				mapping->DCId = map->DCId;
				mapping->substratePath = map->substratePath;
				oldMappings->push_back(mapping);
				if(mapping->DCId != -1){
					dataCenterOfPendingLinks.push_back(mapping->DCId);
				}else{
					int DC2 = mapping->substratePath->link[mapping->substratePath->len-1];
					int DC1 = mapping->substratePath->link[0];
					if(DC1 == dataCenterSource){
						dataCenterOfPendingLinks.push_back(DC2);
					}else{
						if(DC2 == dataCenterSource){
							dataCenterOfPendingLinks.push_back(DC1);
						}else{
							cout << "OUCH, THERE IS A SERIOUS PROBLEM, THE LINK SEEMS TO ORIGINATE FROM A DATA CENTER OTHER THAN THE ONE OF THE PARTITION"<<endl;
						}
					}
				}
			}
		}
	}

	//remove the links from the embedding
	for(unsigned int i=0; i<pendingLinks.size(); i++){
		link_t* l = pendingLinks.at(i);
		mapping_virtual_link_to_substrate_path* mapping = l->mapping;
		if(mapping->DCId != -1){
			this->substrateInfrastructure->getDataCenters()->at(mapping->DCId)->removeLink(l,t);
			mapping->DCId = -1;
		}else{
			this->substrateInfrastructure->removeVirtualLinkGivenPath(mapping->substratePath,t);
			mapping->substratePath = NULL;
		}
	}

	//Migrate the pending links
	bool doneForLinks = true;
	int lastLinkEmbedded = -1;
	for(unsigned int i=0; i<pendingLinks.size(); i++){
		link_t* l = pendingLinks.at(i);
		//mapping_virtual_link_to_substrate_path* mapping = oldMappings->at(i);
		int DCDestinationOfThePendingLink = dataCenterOfPendingLinks.at(i);

		if(DCDestinationOfThePendingLink == dataCenterDestination){
			//Embed the link inside data center destination
			//The new embedding will be in the same data center
			this->substrateInfrastructure->getDataCenters()->at(dataCenterDestination)->embedLink(l,t);
			//delete l->mapping->substratePath;
			l->mapping->substratePath = NULL;
			l->mapping->DCId = dataCenterDestination;
			lastLinkEmbedded = i;
		}else{
			//Embed the between data center destination and DCDestinationOfThePendingLink
			double res1 = 0;
			path_t* path = this->substrateInfrastructure->embedVirtualLinkBetweenDataCenters(dataCenterDestination, DCDestinationOfThePendingLink,l->bw, l->delay,t, &res1);
			if(path != NULL){
				//the link has been successfully re-embedded
				l->mapping->substratePath = path;
				l->mapping->DCId = -1;
				lastLinkEmbedded = i;
			}else{
				doneForLinks = false;
				break;
			}
		}
	}//End For PENDING LINKS


	if(doneForLinks == false){
		//there was a problem, go back to the previous version before the migration

		//Put the partition back to the data center where it was before the migration
		this->substrateInfrastructure->getDataCenters()->at(dataCenterDestination)->removePartition(partition, t);
		this->substrateInfrastructure->getDataCenters()->at(dataCenterSource)->embedPartition(partition,t);
		partition->mapping->datacenterID = dataCenterSource;

		//Put the links back to their mapping before the attempt to migrate

		//Remove the mapping of the links
		for(int i = 0; i<=lastLinkEmbedded; i++){
			link_t* l = pendingLinks.at(i);
			mapping_virtual_link_to_substrate_path* mapping = l->mapping;
			if(mapping->DCId != -1){
				this->substrateInfrastructure->getDataCenters()->at(mapping->DCId)->removeLink(l,t);
			}else{
				this->substrateInfrastructure->removeVirtualLinkGivenPath(mapping->substratePath,t);
			}
		}

		for(unsigned int i = 0; i<pendingLinks.size(); i++){
			link_t* l = pendingLinks.at(i);
			mapping_virtual_link_to_substrate_path* mapping = oldMappings->at(i);
			if(mapping->DCId != -1){
				//The link was embedded in the same DC
				this->substrateInfrastructure->getDataCenters()->at(dataCenterSource)->embedLink(l,t);
				l->mapping->DCId = dataCenterSource;
				l->mapping->substratePath = NULL;
			}else{
				int DC2 = mapping->substratePath->link[mapping->substratePath->len-1];
				int DC1 = mapping->substratePath->link[0];
				double res1 = 0;
				path_t* path = NULL;
				if(DC2 == dataCenterSource){
					path = this->substrateInfrastructure->embedVirtualLinkBetweenDataCenters(DC1, dataCenterSource,l->bw, l->delay,t, &res1);
				}else{
					if(DC1 == dataCenterSource){
						path = this->substrateInfrastructure->embedVirtualLinkBetweenDataCenters(dataCenterSource, DC2,l->bw, l->delay,t, &res1);
					}else{
						cout << "OUCH OUCH, THERE IS A SERIOUS PROBLEM, I TRIED TO OPTIMZE NO I LOST THE INITIAL CONFIGURATION I STARTED FROM"<<endl;
					}
				}
				if(path != NULL){
					l->mapping->substratePath = path;
					l->mapping->DCId = -1;
				}else{
					cout << "OUCH OUCH, THERE IS A SERIOUS PROBLEM, I TRIED TO OPTIMZE AND I CAN NO LONGER ROLL BACK"<<endl;
				}

			}
		}
		//Put the partition nack to its data center
		return FAILURE_MIGRATE;

	}else{
		//The pending links have successfully been embedded
		//return the cost of the migration
		partition->mapping->datacenterID = dataCenterDestination;
		double cost = partition->migrationCost;
		partition->numberOfMigrations ++;

		//Update the mapping of the partition and the links in the database
		if(this->dataBaseConnector != NULL){
			this->dataBaseConnector->updateVDCPartitionMappingInTheDataBase(partition, t);
			for(unsigned int i = 0; i<pendingLinks.size(); i++){
				this->dataBaseConnector->updateVirtualLinkMappingInTheDataBase(pendingLinks.at(i), t);
			}
		}
		return cost;
	}
}

vector<partition_t*>* CentralController::getPartitionsEmbeddedInDataCenter(int dataCenterID) {
	/*
	 * Returns a list of partitions that have been embedded in Data Center dataCenterID
	 */
	vector<partition_t*>* result = new vector<partition_t*>();

	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		for(int j=0; j<this->vdcRequestsEmbedded->at(i)->numberOfPartitions; j++){
			partition_t* partition = this->vdcRequestsEmbedded->at(i)->partitions->at(j);
			if(partition->mapping->datacenterID == dataCenterID){
				result->push_back(partition);
			}
		}
	}
	return result;
}

VDCRequestPartitioned* CentralController::getVDCRequestPartitionedOfAPartition(partition_t* partition){
	/*
	 * Returns the VDCRequestPartitioned instance of to which the partition belongs
	 */
	VDCRequestPartitioned* result = NULL;
	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		if(this->vdcRequestsEmbedded->at(i)->originalRequest->reqID == partition->reqID){
			return this->vdcRequestsEmbedded->at(i);
		}
	}

	return result;

}

bool CentralController::admissionForRequest(VDCRequestPartitioned* request, double acceptanceRatioSoFar, double currentEmbeddingCost, double t) {

	//Have the estimation before the next reporting period

	double carbonPerVMU1 = this->getCarbonFootprintPerUnitOfVMInAllDatacenters(t);
	double carbonPerBWU1 = this->getCarbonFootprintPerUnitOfBandwidthInWholeInfrastructure(t);

	double carbonPerVMU = this->getEstiamtionCarbonFootprintPerUnitOfVMInAllDatacentersForAPeriodOfTime(t, t+this->centralControllerParameters->SLAServiceGuaranteePeriod-1);
	double carbonPerBWU = this->getEstiamtionCarbonFootprintPerUnitOfBandwidthInWholeInfrastructureForAPeriodOfTime(t, t+this->centralControllerParameters->SLAServiceGuaranteePeriod-1);

	carbonPerVMU = (carbonPerVMU1+carbonPerVMU)/2;
	carbonPerBWU = (carbonPerBWU1+carbonPerBWU)/2;

	double carbonForRequest = this->getCarbonFootrpintForARequest(request->originalRequest, carbonPerVMU, carbonPerBWU);
	double carbonLimitForRequest = request->originalRequest->limitOfCarbonEmissionPerServicePeriodGuarantee/this->centralControllerParameters->SLAServiceGuaranteePeriod;

	double estimatedFutureCost = this->getEstimationCostForARequestInTheFuture(request, t);

	//Find a way to reject based on the current available renewables
	//if(carbonForRequest < carbonLimitForRequest || acceptanceRatioSoFar == 0 || acceptanceRatioSoFar == 1){
	if(carbonForRequest < carbonLimitForRequest){
		return true;
	}else{
		//We accept if there is still profit to be made
		double revenueLeft = request->originalRequest->revenue * (1 - this->centralControllerParameters->SLAViolationProportionOfRevenueToRefund);
		if(revenueLeft > estimatedFutureCost){
			//There is still profit to be made out there
			return true;
		}else{
			//There is no profit to make out there, reject is better
			return false;
		}
	}
}



double CentralController::getEstimationCostForARequestInTheFuture(VDCRequestPartitioned* request, double t){

	/*
	 * Returns the estimation of the cost of the requests in the future if the request is accepted
	 */
	//VDCRequestPartitioned* vdc = this->getC
	double tBegin = t;
	double tEnd = request->originalRequest->arrtime + request->originalRequest->duration;
	if(tEnd > t+this->centralControllerParameters->SLAServiceGuaranteePeriod-1){
		tEnd = t+this->centralControllerParameters->SLAServiceGuaranteePeriod-1;
	}

	double costForFuture = 0;
	double sumBandwidthInBackbone = 0;
	//We compute the cost without considering the dynamic reconfiguration
	//This is a pessimistic approach
	for(int i=0; i<request->numberOfPartitions; i++){
		partition_t* partition = request->partitions->at(i);
		int DCid = partition->mapping->datacenterID;
		DataCenter* DC = this->substrateInfrastructure->getDataCenters()->at(DCid);
		double brownPower = DC->getEstimationBrownPowerNeededForPartitionForATimePeriod(partition, tBegin, tEnd);
		double costPartition = (brownPower/1000)*(DC->getEstimationOfAverageElectricityPrice(tBegin, tEnd) + DC->getCarbonFootPrintCoefficient(tBegin)*DC->getCarbonFootPrintUnitCost(tBegin));
		costForFuture += costPartition;
		//Add the cost of the bandwidth in the backbone network
		for(int j=0; j<request->numberOfPartitions; j++){
			if(i != j && request->partitions->at(j)->mapping->datacenterID != DCid){
				for(unsigned int z=0; z<request->linksBetweenPartirions[i][j]->size(); z++){
					sumBandwidthInBackbone += request->linksBetweenPartirions[i][j]->at(z)->bw;
				}
			}
		}
	}
	double costBW = sumBandwidthInBackbone*this->centralControllerParameters->costPerUnitOfBandwidthInBackboneNetwork;
	costForFuture += costBW;
	return costForFuture;
}


double CentralController::embedVDCRequestGreedyWithAdmissionControl(VDCRequestPartitioned* request, double actualAcceptanecRatio, double t) {
	/*
	 * After the decision of embeddings made, a second admission control is applied.
	 * The idea is to reject the requests with tight carbon emission constraints
	 */
	//double cost = this->embedVDCRequestGreedyUsingEstimationOfRenewables(request,t);
	double cost = this->embedVDCRequestGreedy(request,t);
	if(cost>=0){
		//The request is embeddable
		bool admission = this->admissionForRequest(request, actualAcceptanecRatio, cost, t);
		if(!admission){
			//The request is going to generate a lot of carbon, reject it
			double costDC, costBackbone;
			this->removeRequestWithLinksBetweenPartitions(request->originalRequest, t, &costDC, &costBackbone);
			return -1;
		}else{
			return cost;
		}
	}else{
		return cost;
	}
}

reoptimization_summary_t* CentralController::migrateToOptimizeGreenPowerUsage(double t, double estimationTimeWindow) {

	/*
	 * Migrate from data centers with NO available renewables to data centers with available renewables
	 * Returns the cost of the migration
	 */

	double diff[this->substrateInfrastructure->numberOfDatacenters];
	double availableRenewable[this->substrateInfrastructure->numberOfDatacenters];
	double timeEnd = t + estimationTimeWindow;
	if(this->centralControllerParameters->reoptimizationInterval < estimationTimeWindow){
		timeEnd = t + this->centralControllerParameters->reoptimizationInterval;
	}

	reoptimization_summary_t* result = new reoptimization_summary_t;
	result->migrationCost = 0;
	result->numberOfMigratedVMs = 0;
	result->numberOfDistinctRequests = 0;
	result->numberOfMigratedPartitions = 0;
	result->tBegin = t;
	vector<int> distinctRequests = vector<int>();

	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		availableRenewable[i] = this->substrateInfrastructure->getDataCenters()->at(i)->getEstimationOfAmountOfRenewables(t, timeEnd)/(timeEnd - t);//Get the rate of the available renewables
		//diff[i] = availableRenewable[i] - this->substrateInfrastructure->getDataCenters()->at(i)->actualUtilization->usedPowerIT;
		diff[i] = availableRenewable[i] - this->substrateInfrastructure->getDataCenters()->at(i)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(t, timeEnd);
	}

	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		if(diff[i] < 0) continue;
		for(int j=0; j<this->substrateInfrastructure->numberOfDatacenters; j++){
			if(i == j) continue;
			if(diff[j] > 0) continue;
			vector<partition_t*>* partitionsInJ = this->getPartitionsEmbeddedInDataCenter(j);

			unsigned int indexPartition = 0;
			while(diff[i]>0 && diff[j]<0 && indexPartition<partitionsInJ->size()){
				//There is a gain in renewable utilization if we migrate from j to i
				partition_t* partition = partitionsInJ->at(indexPartition);
				VDCRequestPartitioned* request = this->getVDCRequestPartitionedOfAPartition(partition);
				double cost = this->migratePartition(partition, request,j,i,t);
				if(cost > 0){
					//Partition successfully migrated
					//diff[i] = availableRenewable[i] - this->substrateInfrastructure->getDataCenters()->at(i)->actualUtilization->usedPowerIT;
					diff[i] = availableRenewable[i] - this->substrateInfrastructure->getDataCenters()->at(i)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(t, timeEnd)/(timeEnd - t);;

					//diff[j] = availableRenewable[j] - this->substrateInfrastructure->getDataCenters()->at(j)->actualUtilization->usedPowerIT;
					diff[j] = availableRenewable[j] - this->substrateInfrastructure->getDataCenters()->at(j)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(t, timeEnd);

					result->numberOfMigratedVMs += partition->nodes;
					result->numberOfMigratedPartitions++;
					result->migrationCost += cost;
					bool found = false;
					for(unsigned int z=0; z<distinctRequests.size(); z++){
						if(distinctRequests.at(z) == partition->reqID){
							found = true;
							break;
						}
					}
					//if(std::find(distinctRequests.begin(), distinctRequests.end(), partitionsInJ->at(indexPartition)->reqID) == distinctRequests.end()) {
					if(!found){
						result->numberOfDistinctRequests++;
						distinctRequests.push_back(partition->reqID);
					}
				}else{
					//Partition not migrated, try with the next partitions in the list
				}
				indexPartition ++;
			}
		}
	}
	return result;
}





reoptimization_summary_t* CentralController::migrateToOptimizeGreenPowerUsageConsideringBackboneNetworkCosts(double t, double estimationTimeWindow){
	/*
	 *Migrate from data centers with NO available renewables to data centers with available renewables
	 * Returns the cost of the migration
	 */

	double diff[this->substrateInfrastructure->numberOfDatacenters];
	double availableRenewable[this->substrateInfrastructure->numberOfDatacenters];
	double timeEnd = t + estimationTimeWindow;
	if(this->centralControllerParameters->reoptimizationInterval < estimationTimeWindow){
		timeEnd = t + this->centralControllerParameters->reoptimizationInterval;
	}

	reoptimization_summary_t* result = new reoptimization_summary_t;
	result->migrationCost = 0;
	result->numberOfMigratedVMs = 0;
	result->numberOfDistinctRequests = 0;
	result->numberOfMigratedPartitions = 0;
	result->tBegin = t;
	vector<int> distinctRequests = vector<int>();

	vector<int> sourceDCs = vector<int>();//Data centers to offload
	vector<int> destinationDCs = vector<int>();//Data centers to migrate partitions to

	for(int i=0; i<this->substrateInfrastructure->numberOfDatacenters; i++){
		availableRenewable[i] = this->substrateInfrastructure->getDataCenters()->at(i)->getEstimationOfAmountOfRenewables(t, timeEnd)/(timeEnd - t);//Get the rate of the available renewables
		//diff[i] = availableRenewable[i] - this->substrateInfrastructure->getDataCenters()->at(i)->actualUtilization->usedPowerIT;
		diff[i] = availableRenewable[i] - this->substrateInfrastructure->getDataCenters()->at(i)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(t, timeEnd);
		if(diff[i]>0){
			destinationDCs.push_back(i);
		}else{
			if(diff[i]<0){
				sourceDCs.push_back(i);
			}else{
				//this data center is in equilibrium, do not migrate to or from
			}
		}
	}

	while(sourceDCs.size()>0 && destinationDCs.size()>0){
		int source = sourceDCs.at(0);
		sourceDCs.erase(sourceDCs.begin());
		vector<partition_t*>* partitionsInJ = this->getPartitionsEmbeddedInDataCenter(source);
		unsigned int indexPartition = 0;
		while(indexPartition<partitionsInJ->size() && destinationDCs.size()>0){
			//Try to migrate the first partition in the list to the
			partition_t* partition = partitionsInJ->at(indexPartition);
			VDCRequestPartitioned* request = this->getVDCRequestPartitionedOfAPartition(partition);

			//Build the list of destinations for this partition
			vector<int> listDestinationsForCurrentPartition = vector<int>();
			vector<double> listCostsForDestinations = vector<double>();
			double costForMinDestination = INIFINITY;
			for(unsigned int k=0; k<destinationDCs.size(); k++){
				int dcDest = destinationDCs.at(k);
				double costForDC = this->getCostInBackboneToMigratePartition(partition, request, source, dcDest);
				bool done = false;
				for(unsigned int z=0; z<listDestinationsForCurrentPartition.size(); z++){
					if(costForDC < listCostsForDestinations.at(z)){
						//insert here
						listCostsForDestinations.insert(listCostsForDestinations.begin()+z, costForDC);
						listDestinationsForCurrentPartition.insert(listDestinationsForCurrentPartition.begin()+z, dcDest);
						done = true;
						break;
					}
				}
				if(!done){
					int z = listDestinationsForCurrentPartition.size();
					listCostsForDestinations.insert(listCostsForDestinations.begin()+z, costForDC);
					listDestinationsForCurrentPartition.insert(listDestinationsForCurrentPartition.begin()+z, dcDest);
				}
			}

			//Migrate the partition to the best data center in increasing cost
			bool done = false;
			int currentDestination = 0;
			while(!done && currentDestination<(int)listDestinationsForCurrentPartition.size()){
				int dest = listDestinationsForCurrentPartition.at(currentDestination);
				double cost = this->migratePartition(partition, request, source,dest,t);
				if(cost > 0){
					//Partition successfully migrated
					done = true;
					diff[source] = availableRenewable[source] - this->substrateInfrastructure->getDataCenters()->at(source)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(t, timeEnd)/(timeEnd - t);;
					diff[dest] = availableRenewable[dest] - this->substrateInfrastructure->getDataCenters()->at(dest)->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(t, timeEnd);
					result->numberOfMigratedVMs += partition->nodes;
					result->numberOfMigratedPartitions++;
					result->migrationCost += cost;
					bool found = false;
					for(unsigned int z=0; z<distinctRequests.size(); z++){
						if(distinctRequests.at(z) == partition->reqID){
							found = true;
							break;
						}
					}
					//if(std::find(distinctRequests.begin(), distinctRequests.end(), partitionsInJ->at(indexPartition)->reqID) == distinctRequests.end()) {
					if(!found){
						result->numberOfDistinctRequests++;
						distinctRequests.push_back(partition->reqID);
					}
				}else{
					//Partition not migrated, try with the next destination in the list of destinations
					currentDestination++;
				}
			}
			//Go to the next partition
			indexPartition ++;
		}
	}
	return result;
}

void CentralController::closeConnexionsWithRemoteDataCenters() {
	for(unsigned int i=0; i<this->substrateInfrastructure->getDataCenters()->size(); i++){
		if(this->substrateInfrastructure->getDataCenters()->at(i)->remoteDataCenterClient != NULL){
			this->substrateInfrastructure->getDataCenters()->at(i)->remoteDataCenterClient->closeConnexion();
		}
	}
}

double CentralController::PartitionAndEmbedVDCRequest(request_t* request, double t) {
	/*
	 * This Method partitions and embeds a VDC request
	 */

	//Should update the
	VDCRequestPartitioned* vdcPartitioned =this->partitionRequest(request, PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION);
	if(vdcPartitioned != NULL){
		double res = this->embedVDCRequestGreedy(vdcPartitioned, t);
		if(res<0){
			res = res + 1 - 1;
		}
		return res;
	}
	return -1;
}



VDCRequestPartitioned* CentralController::partitionRequest(request_t* initialRequest, int partitioingMethod) {
	VDCRequestPartitioned* vdcPartitioned = NULL;
	switch(partitioingMethod){
	case PARTITIONING_NODE_IS_PARTITION :{
		//Every node in the VDC is considered as a partition
		vdcPartitioned = this->partitionVDCRequestNoPartitioning(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION :{
		//Every node in the VDC is considered as a partition
		vdcPartitioned = this->partitionVDCRequestNoPartitioning(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_LOUVAIN :{
		//Use the LOUVAIN Algorithm
		vdcPartitioned = this->partitionVDCRequestLouvainAlgorithmModifiedVersion(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION :{
		//Use the LOUVAIN Algorithm
		vdcPartitioned = this->partitionVDCRequestLouvainAlgorithmModifiedVersion(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL :{
		//Use the LOUVAIN Algorithm
		vdcPartitioned = this->partitionVDCRequestLouvainAlgorithmModifiedVersion(initialRequest);
		return vdcPartitioned;
	}
	break;



	case PARTITIONING_EQUAL_PARTITIONS_SIZE :{
		//Partition into equal size partitions
		//vdcPartitioned = cn->partitionVDCRequestIntoEqualSizePartitions(initialRequest);
		vdcPartitioned = this->partitionVDCRequestIntoEqualSizePartitions(initialRequest);
		//vdcPartitioned = cn->partitionVDCRequestNoPartitioning(initialRequest);
		return vdcPartitioned;
	}
	break;

	default:{
		//There is a problem, no partitioning method is specified.
		//Should rise an exception, no partitioning specified
	}
	break;
	}

	return vdcPartitioned;

}

void CentralController::writeResultsInFiles(simulation_parameters_t simulationParameters) {

	//Write the results into files
	cout << "Writing the results into files " << endl;
	string base("Results");
	mkdir(base.c_str(), 0777);
	string baseDirectory("Results/");
	if(simulationParameters.arrivalRate > 0){
		baseDirectory.append("ResultsArrivalRate");
		stringstream ss1;
		ss1 << simulationParameters.arrivalRate;
		baseDirectory.append(ss1.str());

		baseDirectory.append("LocationProbability");
		stringstream ss2;
		ss2 << simulationParameters.locationConstraintProbability;//add number to the stream
		baseDirectory.append(ss2.str());

		baseDirectory.append("CarbonCost");
		stringstream ss3;
		ss3 << simulationParameters.carbonCost;//add number to the stream
		baseDirectory.append(ss3.str());

		baseDirectory.append("ReconfigurationInterval");
		stringstream ss4;
		ss4 << this->centralControllerParameters->reoptimizationInterval;//add number to the stream
		baseDirectory.append(ss4.str());

		baseDirectory.append("ReportingPeriod");
		stringstream ss5;
		ss5 << this->centralControllerParameters->SLAServiceGuaranteePeriod;//add number to the stream
		baseDirectory.append(ss5.str());

		mkdir(baseDirectory.c_str(), 0777);

		baseDirectory.append("/Run");

		stringstream ssss;
		ssss << simulationParameters.currentRun;
		baseDirectory.append(ssss.str());

	}else{
		time_t timeObj;
		time(&timeObj);
		tm *pTime = gmtime(&timeObj);
		char buffer[100];
		sprintf(buffer, "%d-%d-%d_%d-%d-%d", pTime->tm_mday, pTime->tm_mon, pTime->tm_year, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
		baseDirectory.append(buffer);
	}

	mkdir(baseDirectory.c_str(), 0777);
	baseDirectory.append("/");

	switch(this->centralControllerParameters->partitioningMethod){
	case PARTITIONING_NODE_IS_PARTITION :{
		//Every node in the VDC is considered as a partition
		baseDirectory.append("ResultsNoPartitioning/");
	}
	break;

	case PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION :{
		//Every node in the VDC is considered as a partition

		baseDirectory.append("ResultsOptimalSolution/");
	}
	break;

	case PARTITIONING_LOUVAIN :{
		//Use the LOUVAIN Algorithm
		baseDirectory.append("ResultsLouvain/");
	}
	break;
	case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION :{
		//Use the LOUVAIN Algorithm, dynamic migration over time
		baseDirectory.append("ResultsLouvainWithDynamicMigration/");
	}
	break;


	case PARTITIONING_EQUAL_PARTITIONS_SIZE :{
		//Partition into equal size partitions
		baseDirectory.append("ResultsLoadBalancing/");
	}
	break;
	case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL :{
		//Partition into equal size partitions
		baseDirectory.append("ResultsLouvainWithDynamicMigrationAndAdmissionControl/");
	}
	break;



	default:{
		//There is a problem, no partitioning method is specified.
		//Should rise an exception, no partitioning specified
	}
	break;
	}



	mkdir(baseDirectory.c_str(), 0777);

	//Writing the overall parameters
	ofstream outFileAll[27]; //0: Cost in DCs, 1: Cost in Provisioned Network, 2: Acceptance ratio; 3: Average Link Utilization in Provisioned Network
	//mkdir("Results");
	string s = baseDirectory + string("CostInAllDCs");
	outFileAll[0].open(s.c_str());
	//outFileAll[0].open ("Results/CostInAllDCs");

	s = baseDirectory + string("CostForUsedNodesInProvisionedNetwork");
	outFileAll[1].open(s.c_str());

	s = baseDirectory + string("AcceptanceRatio");
	outFileAll[2].open(s.c_str());;

	s = baseDirectory + string("AverageLinkUtilizationInProvisionedNetwork");
	outFileAll[3].open(s.c_str());

	s = baseDirectory + string("CDFLinkUtilizationInProvisionedNetwork");
	outFileAll[4].open(s.c_str());

	s = baseDirectory + string("SumUsedBandwidthInProvisionedNetwork");
	outFileAll[5].open(s.c_str());

	s = baseDirectory + string("CostForUsedBandwidthInProvisionedNetwork");
	outFileAll[6].open(s.c_str());

	s = baseDirectory + string("InstantaneousRevenue");
	outFileAll[7].open(s.c_str());

	s = baseDirectory + string("AmountOfCarbonAllDatacenters");
	outFileAll[8].open(s.c_str());

	s = baseDirectory + string("AmountOfGeneratedRenewablesAllDatacenters");
	outFileAll[9].open(s.c_str());

	s = baseDirectory + string("AveragePartitioningTimeComplexity");
	outFileAll[10].open(s.c_str());

	s = baseDirectory + string("AverageEmbeddingTimeComplexity");
	outFileAll[11].open(s.c_str());

	s = baseDirectory + string("InstantaneousNumberOfEmbeddedRequests");
	outFileAll[12].open(s.c_str());

	s = baseDirectory + string("SoFarNumberOfEmbeddedRequests");
	outFileAll[13].open(s.c_str());

	s = baseDirectory + string("SoFarNumberOfTotalRequests");
	outFileAll[14].open(s.c_str());

	s = baseDirectory + string("InstantaneousCarbonEmissionPerUnitOfVM");
	outFileAll[15].open(s.c_str());

	s = baseDirectory + string("InstantaneousCarbonEmissionPerUnitOfBW");
	outFileAll[16].open(s.c_str());


	s = baseDirectory + string("InstantaneousSLALimitsCarbonEmission");
	outFileAll[17].open(s.c_str());

	s = baseDirectory + string("InstantaneousSLALimitsCarbonEmissionPerUnitOfVM");
	outFileAll[18].open(s.c_str());

	s = baseDirectory + string("InstantaneousSLALimitsCarbonEmissionPerUnitOfBW");
	outFileAll[19].open(s.c_str());

	s = baseDirectory + string("SLAViolationAmountOfCarbon");
	outFileAll[20].open(s.c_str());

	s = baseDirectory + string("SLAViolationCost");
	outFileAll[21].open(s.c_str());

	s = baseDirectory + string("SLAViolationNumberOfRequests");
	outFileAll[22].open(s.c_str());

	s = baseDirectory + string("ReoptimizationSummaryNumberOfRequests");
	outFileAll[23].open(s.c_str());

	s = baseDirectory + string("ReoptimizationSummaryNumberOfMigratedVMs");
	outFileAll[24].open(s.c_str());

	s = baseDirectory + string("ReoptimizationSummaryNumberOfMigratedPartitions");
	outFileAll[25].open(s.c_str());

	s = baseDirectory + string("ReoptimizationSummaryMigrationCost");
	outFileAll[26].open(s.c_str());



	for(unsigned int i=0; i<this->wholeInfrastructureOverTime->size(); i++){
		//
		infrastructure_utilization_t* resStruct = this->wholeInfrastructureOverTime->at(i);
		outFileAll[0] <<resStruct->tBegin << "\t" << resStruct->costDatacenters << endl;
		outFileAll[1] <<resStruct->tBegin << "\t" << resStruct->costForUsedNodes*this->centralControllerParameters->costPerOnNodeInProvisionedNetwork << endl;
		outFileAll[2] <<resStruct->tBegin << "\t" << resStruct->acceptanceRatio << endl;
		outFileAll[3] <<resStruct->tBegin << "\t" << resStruct->averageLinkUtilizationInBackbone << endl;
		for(int j=0; j < NUMBER_OF_INTERVALS_CDF ; j++){
			double amount = (j*100)/CDF_STEP_INTERVAL;
			double percentage = resStruct->cdfLinkUtilization[j];
			outFileAll[4] <<resStruct->tBegin << "\t" << amount << "\t" << percentage << endl;
		}
		outFileAll[5] <<resStruct->tBegin << "\t" << resStruct->sumUsedBandwidthInBackbone << endl;

		outFileAll[6] <<resStruct->tBegin << "\t" << resStruct->sumUsedBandwidthInBackbone*this->centralControllerParameters->costPerUnitOfBandwidthInBackboneNetwork<< endl;
		outFileAll[7] <<resStruct->tBegin << "\t" << resStruct->revenue << endl;
		outFileAll[8] <<resStruct->tBegin << "\t" << resStruct->amountOfCarbon << endl;
		outFileAll[9] <<resStruct->tBegin << "\t" << resStruct->generatedRenewables << endl;

		outFileAll[10] <<resStruct->tBegin << "\t" << resStruct->partitioningTimeComplexityPerRequest << endl;

		outFileAll[11] <<resStruct->tBegin << "\t" << resStruct->embeddingTimeComplexityPerRequest << endl;

		outFileAll[12] <<resStruct->tBegin << "\t" << resStruct->instantaneousNumberOfEmbeddedRequests << endl;

		outFileAll[13] <<resStruct->tBegin << "\t" << resStruct->numberOfEmbeddedRequestsSoFar << endl;

		outFileAll[14] <<resStruct->tBegin << "\t" << resStruct->numberOfTotalRequestsSoFar << endl;

		outFileAll[15] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionPerUnitOfVM << endl;
		outFileAll[16] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionPerUnitOfBandwidth << endl;

		outFileAll[17] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionLimitByAllRequests << endl;
		outFileAll[18] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionLimitPerUnitOfVM << endl;
		outFileAll[19] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionLimitPerUnitOfBandwidth << endl;

	}


	for(unsigned int i=0; i<this->SLAviolatioOverTime->size(); i++){
		sla_violation_summary_t* resStruct = this->SLAviolatioOverTime->at(i);
		outFileAll[20] <<resStruct->tEnd << "\t" << resStruct->totalCarbonViolation << endl;
		outFileAll[21] <<resStruct->tEnd << "\t" << resStruct->totalPenaltyCostToPay << endl;
		outFileAll[22] <<resStruct->tEnd << "\t" << resStruct->numberOfRequestViolated << endl;
	}

	for(unsigned int i=0; i<this->reoptimazationSummariesOverTime->size(); i++){
		reoptimization_summary_t* resStruct = this->reoptimazationSummariesOverTime->at(i);
		outFileAll[23] <<resStruct->tBegin << "\t" << resStruct->numberOfDistinctRequests << endl;
		outFileAll[24] <<resStruct->tBegin << "\t" << resStruct->numberOfMigratedVMs << endl;
		outFileAll[25] <<resStruct->tBegin << "\t" << resStruct->numberOfMigratedPartitions << endl;
		outFileAll[26] <<resStruct->tBegin << "\t" << resStruct->migrationCost << endl;
	}


	for(int i=0; i<27; i++){
		outFileAll[i].close();
	}


	//For the different datacenters;
	ofstream outFileDCs[this->centralControllerParameters->numberOfDatacenters][11]; //[i][] for datacenter i, 0:InstantaneousCost, 1: PUE, 2: ElectricityPrice: 3:CarbonFootprintPerUnitOfPower, 4:CarbonCostPerTon, 5:availableRenawables, numberOfVMs

	for(int i=1; i <= this->centralControllerParameters->numberOfDatacenters; i++){


		string s0 = baseDirectory + string("InstantaneousCostDataCenter");
		stringstream ss;//create a stringstream
		ss << i;//add number to the stream
		s0.append(ss.str());
		outFileDCs[i-1][0].open(s0.c_str());

		string s1 = baseDirectory + string("PUEDataCenter");
		s1.append(ss.str());
		outFileDCs[i-1][1].open(s1.c_str());

		string s2 = baseDirectory + string("ElectricityPriceDataCenter");
		s2.append(ss.str());
		outFileDCs[i-1][2].open(s2.c_str());

		string s3 = baseDirectory + string("CarbonFootprintPerUnitOfPowerDataCenter");
		s3.append(ss.str());
		outFileDCs[i-1][3].open(s3.c_str());

		string s4 = baseDirectory + string("CarbonCostPerTonDataCenter");
		s4.append(ss.str());
		outFileDCs[i-1][4].open(s4.c_str());

		string s5 = baseDirectory + string("AvailableRenawablesDataCenter");
		s5.append(ss.str());
		outFileDCs[i-1][5].open(s5.c_str());

		string s6 = baseDirectory + string("NumberOfVMsDataCenter");
		s6.append(ss.str());
		outFileDCs[i-1][6].open(s6.c_str());

		string s7 = baseDirectory + string("CostPerUnitOfPowerDataCenter");
		s7.append(ss.str());
		outFileDCs[i-1][7].open(s7.c_str());

		string s8 = baseDirectory + string("UsedPowerDataCenter");
		s8.append(ss.str());
		outFileDCs[i-1][8].open(s8.c_str());

		string s9 = baseDirectory + string("UsedPowerFromTheGridDataCenter");
		s9.append(ss.str());
		outFileDCs[i-1][9].open(s9.c_str());

		string s10 = baseDirectory + string("AmountOfCarbonDataCenter");
		s10.append(ss.str());
		outFileDCs[i-1][10].open(s10.c_str());


	}


	for(unsigned int i=0; i<this->datacenterParameterOverTime->size(); i++){
		vector<datacenter_utilization_t*>* DCsForThisInsatant = this->datacenterParameterOverTime->at(i);
		//For every datcenter
		for(unsigned int j=0; j<DCsForThisInsatant->size(); j++){

			datacenter_utilization_t* ut = DCsForThisInsatant->at(j);
			outFileDCs[j][0] <<ut->tBegin << "\t" << ut->instantaneousCost << endl;
			outFileDCs[j][1] <<ut->tBegin << "\t" << ut->PUE << endl;
			outFileDCs[j][2] <<ut->tBegin << "\t" << ut->electricityPrice << endl;
			outFileDCs[j][3] <<ut->tBegin << "\t" << ut->carbonCoefficient << endl;
			outFileDCs[j][4] <<ut->tBegin << "\t" << ut->carbonCost << endl;
			outFileDCs[j][5] <<ut->tBegin << "\t" << ut->availableRenwables << endl;
			outFileDCs[j][6] <<ut->tBegin << "\t" << ut->numberOfVMs << endl;
			outFileDCs[j][7] <<ut->tBegin << "\t" << ut->costPerUnitOfPower << endl;
			outFileDCs[j][8] <<ut->tBegin << "\t" << ut->usedPowerIT*ut->PUE << endl;

			double usedFromGrid = ut->usedPowerIT*ut->PUE;
			if(ut->availableRenwables > usedFromGrid){
				usedFromGrid = 0;
			}else{
				usedFromGrid -= ut->availableRenwables;
			}
			outFileDCs[j][9] <<ut->tBegin << "\t" << usedFromGrid << endl;
			outFileDCs[j][10] <<ut->tBegin << "\t" << ut->amountOfCarbon << endl;
		}
	}


	for(int i=0; i<this->centralControllerParameters->numberOfDatacenters; i++){
		for(int j=0; j<11; j++){
			outFileDCs[i][j].close();
		}
	}

	//End of the test
	cout << "Results were written into files, end of the simulation " << endl;
	//delete simulator;
}

void CentralController::daemonCollectInfrastructureUtilization(double timeStart) {
	//Collecting the statistics about the utlization in the infrastructure

	double intervDouble = (this->centralControllerParameters->performanceEvaluationReportingInterval)*3600; // Conversion from hours to seconds
	int intervalInSeconds = (int)intervDouble;
	double currentTime = timeStart;

	while(!this->stopUtilizationReporting){
		vector<datacenter_utilization_t*>* actualUtilizationInDCs = this->getInstaneousUtilizationInDatacenters(currentTime);
		this->datacenterParameterOverTime->push_back(actualUtilizationInDCs);

		//Backbone network
		infrastructure_utilization_t* par;
		par = this->getInstanteousUtilizationWholeInfrastructure(currentTime/3600);
		par->acceptanceRatio = 0;

		if(nbTotalRequests>0){
			par->acceptanceRatio = (double)nbEmbeddedRequests / (double)nbTotalRequests;
			par->numberOfTotalRequestsSoFar = nbTotalRequests;
			par->numberOfEmbeddedRequestsSoFar = nbEmbeddedRequests;
			//if(nbEmbeddedRequests > 0){
			//	par->partitioningTimeComplexityPerRequest = timeComlexityPartitioningSoFar/nbEmbeddedRequests;
			//	par->embeddingTimeComplexityPerRequest = timeComplexityEmbeddingSoFar/nbEmbeddedRequests;
			//}else{
				par->partitioningTimeComplexityPerRequest = 0;
				par->embeddingTimeComplexityPerRequest = 0;
			//}
		}

		this->wholeInfrastructureOverTime->push_back(par);

		//Go to sleep
		sleep(intervalInSeconds);
		currentTime += intervalInSeconds;
	}
}

void CentralController::daemonCollectSLAViolations(double timeStart) {

	//Collect the summaries of the SLA violations
	double intervDouble = (this->centralControllerParameters->SLAServiceGuaranteePeriod)*3600; // Conversion from hours to seconds
	int intervalInSeconds = (int)intervDouble;
	double currentTime = timeStart;

	while(!this->stopSLAViolationReporting){
		sla_violation_summary_t* par = this->getViolatedSLAsSummaryForAServiceGuaranteePeriod(currentTime/3600);
		this->SLAviolatioOverTime->push_back(par);
		sleep(intervalInSeconds);
		currentTime += intervalInSeconds;
	}
}


void CentralController::daemonReoptimization(double timeStart) {
	//Collect the summaries of the SLA violations
	double intervDouble = (this->centralControllerParameters->reoptimizationInterval)*3600; // Conversion from hours to seconds
	int intervalInSeconds = (int)intervDouble;
	double currentTime = timeStart;

	while(!this->stopSLAViolationReporting){
		currentTime += intervalInSeconds;
		sleep(intervalInSeconds);
		sla_violation_summary_t* par = this->getViolatedSLAsSummaryForAServiceGuaranteePeriod(currentTime/3600);
		this->SLAviolatioOverTime->push_back(par);
	}
}


void* CentralController::staticDaemonCollectInfrastructureUtilization(void* cn) {
	CentralController* cntrl = (CentralController*)cn;
	cntrl->daemonCollectInfrastructureUtilization(0);
}

void* CentralController::staticDaemonCollectSLAViolations(void* cn) {
	CentralController* cntrl = (CentralController*)cn;
	cntrl->daemonCollectSLAViolations(0);
}

void* CentralController::staticDaemonReoptimization(void* cn) {
	CentralController* cntrl = (CentralController*)cn;
	cntrl->daemonReoptimization(0);
}


void CentralController::startThreadsToCollectStatistics(double timeStart) {
	//Start the threads

	pthread_t threadCollectUtilization, threadCollectSLAViolation, threadReoptimization;
	int ret1 = pthread_create( &threadCollectUtilization, NULL, staticDaemonCollectInfrastructureUtilization, (void*)this);
	int ret2 = pthread_create( &threadCollectSLAViolation, NULL, staticDaemonCollectSLAViolations, (void*)this);
	int ret3 = pthread_create( &threadReoptimization, NULL, staticDaemonReoptimization, (void*)this);

	//thread threadCollectUtilization(&CentralController::daemonCollectInfrastructureUtilization, this, timeStart);

}

double CentralController::removeRequest(int idRequest, double t) {
	int index=-1;
	for(unsigned int i = 0; i<this->vdcRequestsEmbedded->size();i++){
		if(this->vdcRequestsEmbedded->at(i)->originalRequest->reqID == idRequest){
			index = i;
			break;
		}
	}
	if(index > -1){
		double costDCs, costBackbone;
		double res = this->removeRequestWithLinksBetweenPartitions(this->vdcRequestsEmbedded->at(index)->originalRequest,t,&costDCs, &costBackbone);
		return res;
	}
	return -1;
}



void CentralController::loadFromDataBaseAndEmbedRequstes(double t) {
	//Loads all the VDC requests from the database and embeds them
	vector<VDCRequestPartitioned*>* listVDCs = this->dataBaseConnector->readAllVDCRequestsPartitionedFromDataBase();
	for(unsigned int i=0; i<listVDCs->size(); i++){

		/*
		 * Should we do this or not? Should we re-embed the or just use the fact that every data center will recover from himelf and jus continue
		 * Maybe we should look at how the inter-data center communication is implemented
		 */

		VDCRequestPartitioned* request = listVDCs->at(i);
		double cost = this->embedVDCRequestGreedy(request, t);
		if(cost<0){
			//The request was not embedded
			cout << "OOUUUUUPPSSSSS: There is a serious problem as request "<<request->idRequest<<" was previously embedded but can no longer be embedded, it will be removed from the database "<<endl;
			this->dataBaseConnector->removeVDCRequestFromDataBase(request->originalRequest, t);
		}else{
			//The request was successfully embedded
			//Nothing to do
		}

	}
}

int CentralController::getHighestRequestID() {
	int max = 0;
	for(unsigned int i=0; i<this->vdcRequestsEmbedded->size(); i++){
		if(this->vdcRequestsEmbedded->at(i)->idRequest > max)
			max = this->vdcRequestsEmbedded->at(i)->idRequest;
	}
	return max;
}

void CentralController::stopThreadsCollectingStatistics() {
	this->stopReoptimization = true;
	this->stopUtilizationReporting = true;
	this->stopSLAViolationReporting = true;
}
