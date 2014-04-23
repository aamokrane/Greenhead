/*
 * VDCRequestPartitioned.h
 *
 *  Created on: Dec 27, 2012
 *      Author: ahmedamokrane
 */

#ifndef VDCREQUESTPARTITIONED_H_
#define VDCREQUESTPARTITIONED_H_

#define MAX_PARTITIONS
#define MAX_LINKS_BETWEEN_PAETITIONS

#include <vector>

#include "Embed.h"
#include "SubstrateInfrastructure.h"

using std::vector;

class VDCRequestPartitioned {

public:
	int idRequest; //The id of the request
	request_t* originalRequest;

	//The list of partitions
	int numberOfPartitions;
	vector<partition_t*>* partitions;

	//The set of links between partitions
	//int numberOfLinksBetweenPartirions[MAX_REQ_NODES][MAX_REQ_NODES];
	vector<link_t*>* linksBetweenPartirions[MAX_REQ_NODES][MAX_REQ_NODES];
	//vector<double>* bandwidthDemand[MAX_REQ_NODES][MAX_REQ_NODES];
	//vector<double>* delayDemand[MAX_REQ_NODES][MAX_REQ_NODES];

	//vector<link_t*>* linksBetweenPartitions; //The set of links in another fashion

	//The mapping of the different partitions and links

	//vector< vector<mapping_parition_to_datacenter*>* >* partitionMapping; //Partition Mapping to the set of datacenters (We keep trace of the mapping at different times)

	//vector< vector<mapping_virtual_link_to_substrate_path*>* >* linkMapping;


	//VDCRequestPartitioned(int id, int nbPartitions);
	VDCRequestPartitioned(request_t* rqst);
	virtual ~VDCRequestPartitioned();

	//bool partitionRequestNoPartitioning(SubstrateInfrastructure* substrateInf);
	bool partitionRequestNoPartitioning(SubstrateInfrastructure substrateInf);
};

#endif /* VDCREQUESTPARTITIONED_H_ */
