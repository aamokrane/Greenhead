#ifndef EMBED_H
#define EMBED_H

#define CDF_NodePerVDC
#define CDF_HopsPerVlink

#define MAX_SNODE_PER_PATH 20

#include "def.h"
#include <time.h>
#include <vector>
#include <stdint.h>

using namespace std;

struct link_t;

struct location_t{
	int id;
	int xLocation;//-1 if no location is specified
	int yLocation;
	int maxDistance; //This is for delay constraint between a VM and a datacenter
	char* name;
};

struct result_embed_t{
	double cost;
	double partitioingTime; //Time in seconds
	double embeddingTime;
};

struct datacenter_parameter_t{
	double tBegin;
	double tEnd;
	double value; //PUE, Electricity Price
};


struct path_t {
  int len; //# of nodes on the path
  int link[MAX_SNODE_PER_PATH];
  //vector<int> nodes;
  double bw;
  double delay;
};
//typedef struct path path_t;

struct mapping_virtual_link_to_substrate_path{
	int requestID;
	link_t* virtualLink;
	//vector<link_t*> substrateLinks;
	path_t* substratePath;
	int DCId; //The mapping to a DC or a path in the provisioned network
	double mapBegin;
	double mapEnd;
	double costPerUnitOfTime;
};

struct link_t {
  int idRequest;
  int nodeFrom;
  int nodeTo;
  int partitionFrom;
  int partitionTo;
  double bw;
  double delay;
  //int numberOfMappings;
  //mapping_virtual_link_to_substrate_path* mappings[MAX_MAPPING_HISORY];
  //The mapping changes over time, a list of all the mappings (when dynamically reallocated)
  mapping_virtual_link_to_substrate_path* mapping; //A single mapping which corresponds to the actual mapping
};


struct virtual_machine_t {
  int32_t idRequest;
  int32_t nodeID;
  int32_t type;
  int32_t flavor;
  double memory;
  double disk;
  double cpu;
  double migrationCost;
  location_t* location;
};

struct mapping_parition_to_datacenter{
	int partitioID;
	int requestID;
	int datacenterID;
	double mapBegin;
	double mapEnd;
	double costPerUnitOfTime;
};



struct provisioned_network_node_t{
	int idNode;
	double totalInBandwidth; //The actual bandwidth allocated in this node
	double inBandwidthCapacity; //The traffic that this node can handle
};


#define MAX_SLINK_PER_VLINK 200

struct req2sub {
  int map;
  double maptime;
  int snode[MAX_REQ_NODES];
  path_t spath[MAX_REQ_LINKS];
};
//typedef struct req2sub req2sub_t;



//typedef struct struct_link link_t;

struct request_t {
  int reqID;
  char VDCName[MAX_VDC_NAME_LENGTH];
  char userName[MAX_USER_NAME_LENGTH];
  char sessionName[MAX_SESSION_NAME_LENGTH];
  char applicationName[MAX_APPLICATION_NAME_LENGTH];
  int split;
  double arrtime;
  int topo;
  double duration;
  int nodes;
  int links;
  double revenue;
  double availability;
  //virtual_machine_t* virtualMachines[MAX_REQ_NODES];

  int nodesID[MAX_REQ_NODES];
  double migrationCostOfVM[MAX_REQ_NODES];
  int flavor[MAX_REQ_NODES]; //The type of the VM
  location_t* nodeLocation[MAX_REQ_NODES];
  double cpu[MAX_REQ_NODES];
  double mem[MAX_REQ_NODES];
  double disk[MAX_REQ_NODES];
  int type[MAX_REQ_NODES];
  int bwt[MAX_REQ_NODES]; //total bandwidth of all links connected to this vnode

  link_t* link[MAX_REQ_LINKS];
  double limitOfCarbonEmissionPerServicePeriodGuarantee; //The carbon emission per allowed in the SLA by the VDC request (tons/reporting period or Service guarantee time period)
};
//typedef struct request request_t;

struct partition_t {
  int reqID;
  request_t* request;
  char VDCName[MAX_VDC_NAME_LENGTH];
  char userName[MAX_USER_NAME_LENGTH];
  char sessionName[MAX_SESSION_NAME_LENGTH];
  char applicationName[MAX_APPLICATION_NAME_LENGTH];
  int partitionID;
  int nodes;
  int links;
  double revenue;
  double availability;

  //virtual_machine_t* virtualMachines[MAX_REQ_NODES];
  int nodesID[MAX_REQ_NODES];
  int flavor[MAX_REQ_NODES]; //The type of the VM
  location_t* nodeLocation[MAX_REQ_NODES];
  double migrationCost; //Cost of migration of the whole partition from one data center to another
  double cpu[MAX_REQ_NODES];
  double mem[MAX_REQ_NODES];
  double disk[MAX_REQ_NODES];
  int type[MAX_REQ_NODES];
  int bwt[MAX_REQ_NODES]; //total bandwidth of all links connected to this vnode

  link_t* link[MAX_REQ_LINKS];
  //int numberOfMappings;
  //mapping_parition_to_datacenter* mappings[MAX_MAPPING_HISORY];
  mapping_parition_to_datacenter* mapping;
  int numberOfMigrations;
  //mapping_parition_to_datacenter* mappings[MAX_MAPPING_HISORY];
  //struct req2sub v2sMigrate;
};
//typedef struct partition partition_t;


/*
 * These are structures used for the SAVI implemetation, We remove teh pointer to links in the structures
 */

struct link_savi_remote_call_t {
	int32_t idRequest;
	int32_t nodeFrom;
	int32_t nodeTo;
	int32_t partitionFrom;
	int32_t partitionTo;
	double bw;
	double delay;
};


struct location_savi_remote_call_t{
	int32_t id;
	int32_t xLocation;//-1 if no location is specified
	int32_t yLocation;
	int32_t maxDistance; //This is for delay constraint between a VM and a datacenter
};

struct virtual_machine_savi_remote_call_t {
  int32_t idRequest;
  int32_t nodeID;
  int32_t type;
  int32_t flavor;
  double memory;
  double disk;
  double cpu;
  double migrationCost;
  location_t* location;
};

struct request_savi_remote_call_t {
  int32_t reqID;
  char VDCName[MAX_VDC_NAME_LENGTH];
  char userName[MAX_USER_NAME_LENGTH];
  char sessionName[MAX_SESSION_NAME_LENGTH];
  char applicationName[MAX_APPLICATION_NAME_LENGTH];
  double arrtime;
  double duration;
  int32_t nodes;
  int32_t links;
  double revenue;
  double availability;

  //virtual_machine_savi_remote_call_t virtualMachines[MAX_REQ_NODES];

  int32_t nodesID[MAX_REQ_NODES];
  int32_t flavor[MAX_REQ_NODES]; //The type of the VM
  double migrationCostOfVM[MAX_REQ_NODES];
  location_savi_remote_call_t nodeLocation[MAX_REQ_NODES];
  double cpu[MAX_REQ_NODES];
  double mem[MAX_REQ_NODES];
  double disk[MAX_REQ_NODES];
  int32_t type[MAX_REQ_NODES];
  int32_t bwt[MAX_REQ_NODES]; //total bandwidth of all links connected to this vnode

  link_savi_remote_call_t link[MAX_REQ_LINKS];
  double limitOfCarbonEmissionPerServicePeriodGuarantee; //The carbon emission per allowed in the SLA by the VDC request (tons/reporting period or Service guarantee time period)
};

struct partition_savi_remote_call_t {
  int32_t reqID;
  char VDCName[MAX_VDC_NAME_LENGTH];
  char userName[MAX_USER_NAME_LENGTH];
  char sessionName[MAX_SESSION_NAME_LENGTH];
  char applicationName[MAX_APPLICATION_NAME_LENGTH];
  int32_t partitionID;
  int32_t nodes;
  int32_t links;
  double revenue;
  double availability;

  //virtual_machine_savi_remote_call_t virtualMachines[MAX_REQ_NODES];
  int32_t nodesID[MAX_REQ_NODES];
  int32_t flavor[MAX_REQ_NODES]; //The type of the VM
  location_savi_remote_call_t nodeLocation[MAX_REQ_NODES];
  double cpu[MAX_REQ_NODES];
  double mem[MAX_REQ_NODES];
  double disk[MAX_REQ_NODES];
  int32_t type[MAX_REQ_NODES];
  int32_t bwt[MAX_REQ_NODES]; //total bandwidth of all links connected to this vnode

  link_savi_remote_call_t link[MAX_REQ_LINKS];
  int32_t numberOfMigrations;
};




/*
 *
 */
struct event_t {
  request_t * reqid;
  int etype;
  double time;
  event_t *next;
};


struct substrate_network {
  int nodes;
  int links;
  double cpu[MAX_SUB_NODES];
  double mem[MAX_SUB_NODES];
  int type[MAX_SUB_NODES];
  double bw[MAX_SUB_NODES];
  link_t link[MAX_SUB_LINKS];
};

struct substrate_infrastructure{
	//The substrate infrastructure

};

struct substrate_network_state {
  int Active_count;
  int active[MAX_SUB_NODES];
};


struct s2v_node {
  int req_count;

  request_t * req[MAX_REQ_PER_NODE];
  int vnode[MAX_REQ_PER_NODE];
  double cpu[MAX_REQ_PER_NODE];
  double mem[MAX_REQ_PER_NODE];
  double rest_cpu;
  double rest_mem;
  double rest_bwt;// available outgoing bandwidth from this pnode
};

struct s2v_link {
  int count;
  request_t * req[MAX_REQ_PER_LINK];
  int vlink[MAX_REQ_PER_LINK];
  double bw[MAX_REQ_PER_LINK];
  double rest_bw;
};


struct shortest_path {
  int length;
  int next;
};

struct bneck {
  int slink;
  int req;
  int vlink;
};




//These data structures are used for the graph partitioning

struct edge_graph_t{
	int idSource;
	int idDestionation;
	int partitionSource;
	int partitionDestination;
	double weight;
};


//For performance Evaluation
struct infrastructure_utilization_t{
	double costForUsedNodes;
	double costDatacenters;
	double acceptanceRatio;
	double averageLinkUtilizationInBackbone;
	double sumUsedBandwidthInBackbone;
	double cdfLinkUtilization[NUMBER_OF_INTERVALS_CDF];
	double revenue;
	double amountOfCarbon;
	double generatedRenewables;
	double remainingRenewables;
	double tBegin;
	double partitioningTimeComplexityPerRequest;
	double embeddingTimeComplexityPerRequest;
	double numberOfTotalRequestsSoFar;
	double numberOfEmbeddedRequestsSoFar;
	double instantaneousNumberOfEmbeddedRequests;
	double carbonEmissionPerUnitOfVM;
	double carbonEmissionPerUnitOfBandwidth;
	double carbonEmissionLimitByAllRequests;
	double carbonEmissionLimitPerUnitOfVM;
	double carbonEmissionLimitPerUnitOfBandwidth;
	double totalUsedBandwidth;
	double totalUsedCPU;
};


struct datacenter_utilization_t{ //Instantaneous values
	double tBegin;
	int numberOfVMs;
	double totalUsedCPU;
	double totalUsedBandwidth;
	double PUE;
	double electricityPrice;
	double costPerUnitOfPower;
	double availableRenwables;
	double carbonCoefficient;
	double carbonCost;
	double instantaneousCost;//The actual cost in this datacenter
	double remainingRenewables;
	double usedPowerIT;
	double amountOfCarbon;
	double carbonEmissionPerUnitOfVM;
	double carbonEmissionPerUnitOfBandwidth;
};


struct sla_violation_per_request_t{
	request_t* request;
	double tEnd;
	double amountOfAdditionalCarbonGenerated;
	double cost;
};


struct reoptimization_summary_t{
	double tBegin; //The time of the end of this Service Guarantee Period
	double migrationCost;
	int numberOfMigratedVMs;
	int numberOfMigratedPartitions;
	int numberOfDistinctRequests;
};

struct sla_violation_summary_t{
	double tEnd; //The time of the end of this Service Guarantee Period
	double totalCarbonViolation;
	double totalPenaltyCostToPay;
	int numberOfRequestViolated;
};

struct central_controller_parameters_t{
	int provisionedNetworkTopology;
	int numberOfNodesProvisionedNetwork;
	int numberOfDatacenters;
	double physicalLinkProbability;
	int numberOfAlternativePaths;
	double accessBandwidthForDC;
	double provisionedNetworkLinkCapacity;
	double costPerTonOfCarbon;
	double parameterUpdateInterval;
	double timeSlotDuration;
	double carbonPerUnitOfBandwidthInBackbone;
	int partitioningMethod;
	double performanceEvaluationReportingInterval; //The interval for reporting the utilization in the infrastructure
	double SLAServiceGuaranteePeriod; //The period after which the Carbon emission limits per VDC are checked
	double SLAViolationProportionOfRevenueToRefund; //The proportion of the bill to be given back by the InP in case of violation of the SLA
	double lookAheadTimeForForcasting;
	double reoptimizationInterval;
	double costPerUnitOfBandwidthInBackboneNetwork;
	double costPerOnNodeInProvisionedNetwork;
	char* dataBaseServerAddress;
	int dataBaseServerPort;
	char* dataBaseUsername;
	char* dataBasePassword;
	char* dataBaseName;
};

struct simulation_parameters_t{
	int currentRun;
	double locationConstraintProbability;
	double carbonCost;
	//double reconfigurationInterval;
	//double reportingPeriod;
	double simDuration;
	int requestGeneration; //GOOGLE or POISSON

	//int topologyGeneration; //How the VMs are interconnected
	int minNumberOfVirtualNodes;
	int maxNumberOfVirtualNodes;
	double virtualLinkProbability;//The probability of having a link between two
	int numberOfVirtualNodesDistribution; //The distribution of the number of nodes in the VDCs (CONSTANT, EXPONENTIAL)

	double arrivalRate;	 //Arrival rate of the requests
	int arrivaleRateDistribution; //POISSON, CONSTANT
	double meanLifetime; //Mean duration of a request
	int lifeTimeDistribution; //The lifetime distribution of requests

	int carbonSLAConstraintDistribution; //The distribution of the carbon limitation per reporting period
	double carbonSLAConstraintLimitInf;
	double carbonSLAConstraintLimitSup;

	//The revenue of the different requests
	double revenuePerUnitOfCPU;
	double revenuPerUnitOfBandwidth;
	double revenuPerUnitOfMemory;
	double bandwidthInf;
	double bandwidthSup;
};


#endif//EMBED_H
