/*
 * RawGraph.h
 *
 *  Created on: Jan 11, 2013
 *      Author: ahmed
 */

#ifndef RAWGRAPH_H_
#define RAWGRAPH_H_

#include<vector>
#include<string.h>
#include<stdio.h>
#include "Embed.h"
#include "VDCRequestPartitioned.h"


using std::vector;

class RawGraph {
public:

	vector<int>* nodes;
	vector<location_t*> locationConstraint;
	vector<double>* weightsIn;
	vector<int>* node2partitions;
	int nbNodes;
	int nbPartitions;
	vector<vector<vector<double>* >* >* links;
	request_t* VDCRequest;

	double sumAllWeights;
	double modularity;

	RawGraph(request_t* request);
	virtual ~RawGraph();

	VDCRequestPartitioned* partitionUsingLouvainAlgprithm();

	VDCRequestPartitioned* partitionIntoEqualPartitionsAlgorithmWithoutLocationConstraints(int nbPartitions);

	VDCRequestPartitioned* partitionIntoEqualPartitionsAlgorithmConsideringLocationConstraints(int nbPartitions);

	VDCRequestPartitioned* partitionToGetOneSinglePartition();


	double computeModularity();
	double computeModularityWhenMovingNode(int node, int partitionSource, int partitionDestination);

	double getWeight(int nodei, int nodej);

	VDCRequestPartitioned* getVDCRequestPartitioned();

};

#endif /* RAWGRAPH_H_ */
