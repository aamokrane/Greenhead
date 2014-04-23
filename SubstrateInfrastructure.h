/*
 * SubstrateInfrastructure.h
 *
 *  Created on: Dec 27, 2012
 *      Author: ahmedamokrane
 */

#ifndef SUBSTRATEINFRASTRUCTURE_H_
#define SUBSTRATEINFRASTRUCTURE_H_

#include "DataCenter.h"
#include "Embed.h"

using std::vector;

class SubstrateInfrastructure {

public:

	//The physical infrastructure
	vector<provisioned_network_node_t*>* provisionedNetworkNodes; //The nodes of the provisioned network (provisionedNetworkNodes[i] contains the id of the node in the provisioned network)
	int numberOfNodesInProvisionedNetwork; //Number of nodes in the provisioned network

	vector<DataCenter*>* dataCenters; //The list of datacenters
	int numberOfDatacenters; //The number of datacenters

	vector<link_t*>* links;
	vector<vector<double> *>* bandwidthBetweenNodes;
	vector<vector<double> *>* availableBandwidthBetweenNodes;
	vector<vector<double> *>* delayBetweenNodes;

	//The alternative paths between the datacenters
	vector<vector<vector<path_t*>* >* >* alternativePaths;

	//The on nodes in the provisioned network
	vector<int>* onNodes;
	//vector<double>* totalInBandwidth;


	SubstrateInfrastructure(int nbNodes, int nbDataCenters);
	//SubstrateInfrastructure();
	virtual ~SubstrateInfrastructure();
	vector<DataCenter*>* getDataCenters();
	int getNumberOfDatacenters();
	void setNumberOfDatacenters(int numberOfDatacenters);
	int getNumberOfNodesInProvisionedNetwork();
	void setNumberOfNodesInProvisionedNetwork(int numberOfNodesInProvisionedNetwork);
	vector<provisioned_network_node_t*>* getProvisionedNetworkNodes();
	void buildAlternativePathsBetweenDataCenters(int nbPaths);
	vector<int>* getNeighbors(int nodeID);

	//Getting Statistics
	double getAverageLinkUtilization();//Returns the average link utilization of the physical links between the datacenters
	double getSumUsedBandwidth();
	double getCarbonFootprintOfBackboneNetwork(double carbonPerUnitOfBandwidth);
	infrastructure_utilization_t* getLinkUtilizationDistribution();
	double getProvisionedNetworkInstanteousCostPerUSedNodes(); //Returns the instantanous cost in the provisioned network
	double getProvisionedNetworkInstanteousCostPerUSedBandwidth(); //Returns the instantanous cost in the provisioned network

	//Embedding links
	double getCostEmbedLink(int DCSource, int DCDestination, double bw, double delay); //returns the cost of embedding the virtual link link
	path_t* embedVirtualLinkBetweenDataCenters(int DCSource, int DCDestination, double bw, double delay, time_t t, double* cost);
	int removeVirtualLinkGivenPath(path_t* path, time_t t);
	double getBandwidthForPath(path_t* path);
	double getDelayForPath(path_t* path);
	bool isPossibleToEmbedPartitionInDataCenter(partition_t* partition, int datacenter);

};

#endif /* SUBSTRATEINFRASTRUCTURE_H_ */
