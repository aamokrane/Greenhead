/*
 * SubstrateInfrastructure.cpp
 *
 *  Created on: Dec 27, 2012
 *      Author: ahmedamokrane
 */

#include "SubstrateInfrastructure.h"
#include<queue>
#include "def.h"
#include<iostream>

using std::queue;
using namespace std;


SubstrateInfrastructure::SubstrateInfrastructure(int nbNodes, int nbDataCenters){
	numberOfNodesInProvisionedNetwork = nbNodes;
	numberOfDatacenters = nbDataCenters;
	onNodes = new vector<int>();
	provisionedNetworkNodes = new vector<provisioned_network_node_t*>();

	availableBandwidthBetweenNodes = new vector<vector<double>* >();
	for(int i=0; i<numberOfDatacenters + numberOfNodesInProvisionedNetwork; i++){
		vector<double>* columnNodei = new vector<double>();
		for(int j=0; j<numberOfDatacenters + numberOfNodesInProvisionedNetwork; j++){
			columnNodei->push_back(0);
		}
		availableBandwidthBetweenNodes->push_back(columnNodei);
	}

	bandwidthBetweenNodes = new vector<vector<double>* >();
	for(int i=0; i<numberOfDatacenters + numberOfNodesInProvisionedNetwork; i++){
		vector<double>* columnNodei = new vector<double>();
		for(int j=0; j<numberOfDatacenters + numberOfNodesInProvisionedNetwork; j++){
			columnNodei->push_back(0);
		}
		bandwidthBetweenNodes->push_back(columnNodei);
	}

	delayBetweenNodes = new vector<vector<double>* >();
	for(int i=0; i<numberOfDatacenters + numberOfNodesInProvisionedNetwork; i++){
		vector<double>* columnNodei = new vector<double>();
		for(int j=0; j<numberOfDatacenters + numberOfNodesInProvisionedNetwork; j++){
			columnNodei->push_back(0);
		}
		delayBetweenNodes->push_back(columnNodei);
	}

	dataCenters = new vector<DataCenter*>();
	links = new vector<link_t*>();

	for(int i=0; i<numberOfNodesInProvisionedNetwork; i++){
		provisioned_network_node_t* node = new provisioned_network_node_t;
		node->idNode = i+numberOfDatacenters;
		node->inBandwidthCapacity = BANDWIDTH_CAPACITY_PROVIIONED_NETWORK_NODE;
		node->totalInBandwidth = 0;
		provisionedNetworkNodes ->push_back(node);
	}

	this->alternativePaths = NULL;

}


SubstrateInfrastructure::~SubstrateInfrastructure() {
	// TODO Auto-generated destructor stub
	this->provisionedNetworkNodes->clear();
	for(int i=0; i<numberOfDatacenters + numberOfNodesInProvisionedNetwork; i++){
		availableBandwidthBetweenNodes->at(i)->clear();
		bandwidthBetweenNodes->at(i)->clear();
		delayBetweenNodes->at(i)->clear();
	}
	availableBandwidthBetweenNodes->clear();
	bandwidthBetweenNodes->clear();
	delayBetweenNodes->clear();
	dataCenters->clear();
	for(unsigned int i=0; i<this->alternativePaths->size(); i++){
		for(unsigned int j=0; j<this->alternativePaths->at(i)->size(); j++){
			if(i != j){
				for(unsigned int k=0; k<this->alternativePaths->at(i)->at(j)->size(); k++){
					delete this->alternativePaths->at(i)->at(j)->at(k);
				}
				this->alternativePaths->at(i)->at(j)->clear();
			}
		}
		this->alternativePaths->at(i)->clear();
	}
	//this->alternativePaths->clear();
}


/*
SubstrateInfrastructure::SubstrateInfrastructure(){
	numberOfNodesInProvisionedNetwork = 0;
	numberOfDatacenters = 0;
	availableBandwidthBetweenNodes = new vector<vector<double>* >();
	bandwidthBetweenNodes = new vector<vector<double>* >();
	onNodes = new vector<int>();

}
*/

vector<DataCenter*>* SubstrateInfrastructure::getDataCenters()  {
	return this->dataCenters;
}

int SubstrateInfrastructure::getNumberOfDatacenters()  {
	return numberOfDatacenters;
}

void SubstrateInfrastructure::setNumberOfDatacenters(int numberOfDatacenters) {
	this->numberOfDatacenters = numberOfDatacenters;
}

int SubstrateInfrastructure::getNumberOfNodesInProvisionedNetwork() {
	return numberOfNodesInProvisionedNetwork;
}

void SubstrateInfrastructure::setNumberOfNodesInProvisionedNetwork(
		int numberOfNodesInProvisionedNetwork) {
	this->numberOfNodesInProvisionedNetwork = numberOfNodesInProvisionedNetwork;
}

vector<provisioned_network_node_t*>* SubstrateInfrastructure::getProvisionedNetworkNodes() {
	return provisionedNetworkNodes;
}



void SubstrateInfrastructure::buildAlternativePathsBetweenDataCenters(int nbPaths){
	//Build the alternative paths between the different datacenters

	this->alternativePaths = new vector<vector<vector<path_t*>* >* >(numberOfDatacenters);
	for(int i=0; i<numberOfDatacenters; i++){
		this->alternativePaths->at(i) = new vector<vector<path_t*>* >(numberOfDatacenters);
	}

	for(int i=0; i<numberOfDatacenters; i++){

		//this->alternativePaths->push_back(new vector<vector<path_t*>* >());

		for(int j=0; j<numberOfDatacenters; j++){
			//Find nbPaths paths between the datacenter i and the datacenter j
			if(i == j) continue;

			//int currentPath = 0;
			vector<path_t*>* currentPaths = new vector<path_t*>();
			queue<path_t*>* queueBFS = new queue<path_t*>();
			path_t* p = new path_t;
			p->len = 1;
			p->link[0] = i;
			queueBFS->push(p);
			while(!queueBFS->empty() && currentPaths->size() < (unsigned int)nbPaths){
				path_t* q = queueBFS->front();
				queueBFS->pop();

				if(j == q->link[q->len-1]){
					//We are at the end of the path (We reached the desired datacenter)
					currentPaths->push_back(q);
				}else{
					//We didn't reach the datacenter yet
					vector<int>* neighbors = getNeighbors(q->link[q->len-1]);

					for(unsigned int k=0; k<neighbors->size(); k++){
						if(q->len < MAX_SNODE_PER_PATH){ //Avoid long paths and loops
							path_t* z = new path_t;
							z->len = q->len;

							for(int h=0; h<q->len; h++){
								z->link[h] = q->link[h];
								//Avoid the loops
								if(z->link[h] == neighbors->at(k)){
									//This a loop
									delete z;
									z = NULL;
									break;
								}
							}

							if(z == NULL){
								continue;
							}

							z->bw = q->bw;
							z->delay = q->delay;

							z->link[z->len] = neighbors->at(k);
							z->len++;
							if(z->bw > this->availableBandwidthBetweenNodes->at(q->len-1)->at(neighbors->at(k))){
								z->bw = availableBandwidthBetweenNodes->at(q->len-1)->at(neighbors->at(k));
							}
							if(z->delay < this->delayBetweenNodes->at(q->len-1)->at(neighbors->at(k))){
								z->delay = this->delayBetweenNodes->at(q->len-1)->at(neighbors->at(k));
							}
							if(j == z->link[z->len-1]){
								//We are at the end of the path (We reached the desired datacenter)
								currentPaths->push_back(z);
								if(currentPaths->size() == (unsigned int)nbPaths){
									break;
								}
							}else{
								//We didn't reach the datacenter yet
								queueBFS->push(z);
							}
						}

					}//END FOR NEIGHBORS

				}//END IF j == q->link[q->len-1] (IF WE REACHED THE DATACENTER OF NOT)

			}//END WHILE NBPATHS (while(!queueBFS->empty() && currentPaths->size() < nbPaths))

			//Here we put the paths between the datacenters i and j in the vector
			this->alternativePaths->at(i)->at(j) = currentPaths;
			currentPaths = NULL;

		}//END FOR J IN DATACENTERS

	}//END FOR I IN DATACENTERS
}

vector<int>* SubstrateInfrastructure::getNeighbors(int nodeIndex){
	vector<int>* neighbors = new vector<int>();
	for(int i=0; i<numberOfDatacenters + numberOfNodesInProvisionedNetwork; i++){
		if(i!=nodeIndex && bandwidthBetweenNodes->at(nodeIndex)->at(i) > 0){
			//This node i is a neighbor of nodeID
			neighbors->push_back(i);
		}
	}
	return neighbors;
}

double SubstrateInfrastructure::getCostEmbedLink(int DCSource, int DCDestination, double bw, double delay) {
	/*
	 * Returns the cost of embedding the link between DCs source and destination in the substrate network
	 */
	//We assume that the cost is given by the number of nodes in the provisioned network that have to be turned on
	vector<path_t*>* paths = this->alternativePaths->at(DCSource)->at(DCDestination);
	int minPath = -1;
	int costMinPath = numberOfNodesInProvisionedNetwork + numberOfDatacenters;
	for(unsigned int i=0; i<paths->size(); i++){
		path_t* p = paths->at(i);
		//Check whether this path satisfies the bandwidth and delay requirements
		if(this->getBandwidthForPath(p) < bw || this->getDelayForPath(p) > delay) continue; //The path violates the requirements of the virtual link
		int nb = 0;
		for(int j=0; j<paths->at(i)->len; j++){
			bool found = false;
			for(unsigned int k=0; k<this->onNodes->size(); k++){
				if(this->onNodes->at(k) == paths->at(i)->link[j]){
					found = true;
					break;
				}
			}
			if(!found){
				nb++;
			}
		}

		if(minPath == -1 || costMinPath > nb){
			//We have a better path to use
			minPath = i;
			costMinPath = nb;
		}
	}
	return costMinPath * POWER_CONSUMPTION_INTERMEDIARY_NODE;
}










path_t* SubstrateInfrastructure::embedVirtualLinkBetweenDataCenters(int DCSource, int DCDestination, double bw, double delay, time_t t, double* cost) {
	/*
	 * Embeds and returns the cost after embedding the virtual link between the two datacenters
	 */

	if(DCSource == DCDestination){
		cout << "There is a problem, embed a link between a DC and itself" << endl;
		return NULL;
	}

	vector<path_t*>* paths = this->alternativePaths->at(DCSource)->at(DCDestination);


	int minPath = -1;

	//double costMinPathPower = (double)((numberOfNodesInProvisionedNetwork + numberOfDatacenters)*POWER_CONSUMPTION_INTERMEDIARY_NODE);
	int costMinPathNodes = numberOfNodesInProvisionedNetwork + numberOfDatacenters;
	for(unsigned int i=0; i<paths->size(); i++){
		path_t* p = paths->at(i);
		//Check whether this path satisfies the bandwidth and delay requirements
		if(this->getBandwidthForPath(p) < bw || this->getDelayForPath(p) > delay) continue; //The path violates the requirements of the virtual link

		//This path is eligible to embed the virtual link
		int nb = 0;
		for(int j=1; j<paths->at(i)->len-1; j++){
			bool found = false;
			for(unsigned int k=0; k<this->onNodes->size(); k++){
				if(this->onNodes->at(k) == paths->at(i)->link[j]){
					found = true;
					break;
				}
			}
			if(!found){
				nb++;
			}
		}

		if(minPath == -1 || costMinPathNodes > nb){
			//We have a better path to use
			minPath = i;
			costMinPathNodes = nb;
		}
	}

	if(minPath >-1){
		//We found a possible path between the datacenters
		//At the end, update the available bandwidth between the nodes in the provisioned network
		for(int j=0; j<paths->at(minPath)->len-1; j++){
			this->availableBandwidthBetweenNodes->at(paths->at(minPath)->link[j])->at(paths->at(minPath)->link[j+1]) -= bw;
			this->availableBandwidthBetweenNodes->at(paths->at(minPath)->link[j+1])->at(paths->at(minPath)->link[j]) -= bw;
			//Update the InBnadwidth for all the intermediary nodes
			if(j < paths->at(minPath)->len-2){
				int nodePos = paths->at(minPath)->link[j+1] - numberOfDatacenters;
				provisionedNetworkNodes->at(nodePos)->totalInBandwidth += bw;
			}
		}

		//

		//Add the new turned on nodes to the list of the onNodes
		for(int j=1; j<paths->at(minPath)->len-1; j++){
			bool found = false;
			for(unsigned int k=0; k<this->onNodes->size(); k++){
				if(this->onNodes->at(k) == paths->at(minPath)->link[j]){
					found = true;
					break;
				}
			}
			if(!found){
				this->onNodes->push_back(paths->at(minPath)->link[j]);
			}
		}

		//Add the mapping to the list of the mapped paths in the VDCRequest

		//Return the cost of embedding and the corresponding path
		double costMinPathPower = (double)(costMinPathNodes * POWER_CONSUMPTION_INTERMEDIARY_NODE);
		*cost = costMinPathPower;

		path_t* res = new path_t;
		res->bw = bw;
		res->delay = delay;
		res->len = paths->at(minPath)->len;
		for(int j=0; j<paths->at(minPath)->len; j++){
			res->link[j] = paths->at(minPath)->link[j];
		}
		return res;

	}else{
		//We didn't find a path between the two datacenters
		//Failed to embed the link
		*cost = -1;
		return NULL;
	}

}

double SubstrateInfrastructure::getBandwidthForPath(path_t* path) {
	/*
	 * Returns the amount of available bandwidth on the path,
	 * Used to determine if a path can be used for a virtual link
	 */
	double minBW = -1;
	for(int i=0; i<path->len-1; i++){
		if(minBW == -1 || minBW > this->availableBandwidthBetweenNodes->at(path->link[i])->at(path->link[i+1])){
			minBW = this->availableBandwidthBetweenNodes->at(path->link[i])->at(path->link[i+1]);
		}
	}
	return minBW;
}

int SubstrateInfrastructure::removeVirtualLinkGivenPath(path_t* path, time_t t) {
	/*
	 * This method removes the virtual link embedded in the provisioned network by the path path
	 * return 1 if successful
	 */

	//free the allocated bandwidth in the provisioned network
	for(int i=0; i<path->len-1; i++){
		this->availableBandwidthBetweenNodes->at(path->link[i])->at(path->link[i+1]) += path->bw;
		this->availableBandwidthBetweenNodes->at(path->link[i+1])->at(path->link[i]) += path->bw;
	}

	for(int i=1; i<path->len-1; i++){
		//Update the InBandwidth per node in the provisioned network
		int nodePos = path->link[i] - numberOfDatacenters;
		provisionedNetworkNodes->at(nodePos)->totalInBandwidth -= path->bw;
		if(provisionedNetworkNodes->at(nodePos)->totalInBandwidth == 0){
			//This node doesn't carry traffic any more, turn it off
			for(unsigned int k=0; k<this->onNodes->size(); k++){
				if(this->onNodes->at(k) == path->link[i+1]){
					//remove this node from the list of on nodes
					onNodes->erase(onNodes->begin() + k);
					break;
				}
			}
		}
	}

	return SUCCESS_OPERATION;
}



double SubstrateInfrastructure::getDelayForPath(path_t* path) {
	/*
	 * Returns the delay on the path,
	 * Used to determine if a path can be used for a virtual link
	 */
	double maxDL = -1;
	for(int i=0; i<path->len-1; i++){
		if(maxDL == -1 || maxDL < this->delayBetweenNodes->at(path->link[i])->at(path->link[i+1])){
			maxDL = this->delayBetweenNodes->at(path->link[i])->at(path->link[i+1]);
		}
	}
	return maxDL;
}



double SubstrateInfrastructure::getAverageLinkUtilization(){
	/*
	 * Returns the average link utilization of the physical links between the datacenters
	 */
	double sumUtilization = 0.0;
	int nbUsedLinks = 0;
	for(int i=0; i<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; i++){
		for(int j=i+1; j<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; j++){
			if(this->bandwidthBetweenNodes->at(i)->at(j) > 0){
				sumUtilization += (this->bandwidthBetweenNodes->at(i)->at(j) - this->availableBandwidthBetweenNodes->at(i)->at(j)) / this->bandwidthBetweenNodes->at(i)->at(j);
				nbUsedLinks ++;
			}
		}
	}
	if(nbUsedLinks == 0){
		return 0;
	}else{
		return sumUtilization/nbUsedLinks;
	}
}


double SubstrateInfrastructure::getSumUsedBandwidth(){
	/*
	 * Returns the average link utilization of the physical links between the daytacenters
	 */
	double sumBW = 0.0;
	for(int i=0; i<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; i++){
		for(int j=i+1; j<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; j++){
			if(this->bandwidthBetweenNodes->at(i)->at(j) > 0){
				sumBW += this->bandwidthBetweenNodes->at(i)->at(j) - this->availableBandwidthBetweenNodes->at(i)->at(j);
			}
		}
	}
	return sumBW;
}

double SubstrateInfrastructure::getCarbonFootprintOfBackboneNetwork(double carbonPerUnitOfBandwidth){
	/*
	 * Returns the average link utilization of the physical links between the daytacenters
	 */
	double sumBW = 0.0;
	for(int i=0; i<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; i++){
		for(int j=i+1; j<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; j++){
			if(this->bandwidthBetweenNodes->at(i)->at(j) > 0){
				sumBW += this->bandwidthBetweenNodes->at(i)->at(j) - this->availableBandwidthBetweenNodes->at(i)->at(j);
			}
		}
	}
	double carbonEmission = sumBW*carbonPerUnitOfBandwidth;
	return carbonEmission;
}





infrastructure_utilization_t* SubstrateInfrastructure::getLinkUtilizationDistribution(){
	/*
	 * Returns the average link utilization of the physical links between the daytacenters
	 */
	double sumUtilization = 0.0;
	double sumUsedBw = 0.0;
	int nbUsedLinks = 0;
	infrastructure_utilization_t* result = new infrastructure_utilization_t;
	for(int i=0; i<NUMBER_OF_INTERVALS_CDF; i++){
		result->cdfLinkUtilization[i] = 0.0;
	}
	for(int i=0; i<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; i++){
		for(int j=i+1; j<this->numberOfDatacenters+this->numberOfNodesInProvisionedNetwork; j++){
			if(this->bandwidthBetweenNodes->at(i)->at(j) > 0){
				double currentUtilization = (this->bandwidthBetweenNodes->at(i)->at(j) - this->availableBandwidthBetweenNodes->at(i)->at(j)) / this->bandwidthBetweenNodes->at(i)->at(j);
				sumUtilization += currentUtilization;
				nbUsedLinks ++;
				sumUsedBw += this->bandwidthBetweenNodes->at(i)->at(j) - this->availableBandwidthBetweenNodes->at(i)->at(j);

				int index = 0;
				if(currentUtilization == 0){
					index = 0;
				}else{
					if(currentUtilization == 1){
						index = CDF_STEP_INTERVAL;
					}else{
						index = (int)(currentUtilization*CDF_STEP_INTERVAL)+1;
					}
				}
				result->cdfLinkUtilization[index]++;
			}
		}
	}
	if(nbUsedLinks == 0){
		//This case happens only when we have a problem (No physical links)
		result->averageLinkUtilizationInBackbone = 0;
		result->sumUsedBandwidthInBackbone = 0;
	}else{

		//Turn the distribution into a CDF
		for(int i=0; i<NUMBER_OF_INTERVALS_CDF-1; i++){
			result->cdfLinkUtilization[i+1] += result->cdfLinkUtilization[i];
			result->cdfLinkUtilization[i] = result->cdfLinkUtilization[i]/nbUsedLinks;
		}
		result->cdfLinkUtilization[NUMBER_OF_INTERVALS_CDF-1] =  result->cdfLinkUtilization[NUMBER_OF_INTERVALS_CDF-1]/nbUsedLinks;;

		result->averageLinkUtilizationInBackbone = sumUtilization/nbUsedLinks;
		result->sumUsedBandwidthInBackbone = sumUsedBw;
	}
	return result;
}



double SubstrateInfrastructure::getProvisionedNetworkInstanteousCostPerUSedNodes(){

	return ((double)this->onNodes->size())*((double)POWER_CONSUMPTION_INTERMEDIARY_NODE);
}


double SubstrateInfrastructure::getProvisionedNetworkInstanteousCostPerUSedBandwidth() {

	return -10000;
}


bool SubstrateInfrastructure::isPossibleToEmbedPartitionInDataCenter(
		partition_t* partition, int datacenter) {

	for(int k=0; k<partition->nodes; k++){
		if(partition->nodeLocation[k]->xLocation > -1){
			//This node has a location constraint
			double euclidianDist = (partition->nodeLocation[k]->xLocation - this->dataCenters->at(datacenter)->location->xLocation)*(partition->nodeLocation[k]->xLocation - this->dataCenters->at(datacenter)->location->xLocation)
								+ (partition->nodeLocation[k]->yLocation - this->dataCenters->at(datacenter)->location->yLocation)*(partition->nodeLocation[k]->yLocation - this->dataCenters->at(datacenter)->location->yLocation);
			//Not possible
			if(euclidianDist > (partition->nodeLocation[k]->maxDistance * partition->nodeLocation[k]->maxDistance)){
				return false;
			}
		}
	}
	return true;
}
