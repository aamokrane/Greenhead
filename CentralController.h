/*
 * CentralController.h
 *
 *  Created on: Dec 27, 2012
 *      Author: ahmedamokrane
 */

#ifndef CENTRALCONTROLLER_H_
#define CENTRALCONTROLLER_H_

#include <vector>
#include <limits>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include "Embed.h"
#include "EmbeddingSolution.h"
#include "BranchAndBound.h"
#include "SubstrateInfrastructure.h"
#include "VDCRequestPartitioned.h"
#include "RawGraph.h"
#include "GreenheadCentralControllerMysqlConnector.h"

#include <pthread.h>
//#include <thread>


#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

using std::vector;
using namespace std;

/*
 *
 */
class CentralController {


	/*
	 * This class is responsible for running the assignment algorithm and manage the physical infrastructure
	 *
	 */


public:
	//The physical topology
	SubstrateInfrastructure* substrateInfrastructure;

	//The VDC requests in the system at the desired time
	vector<VDCRequestPartitioned*>* vdcRequestsEmbedded;
	//vector<request_t*>* originalRequests;

	vector<VDCRequestPartitioned*>* vdcRequestsPreviouslyEmbedded; //The requests that have been embedded and that left the system


	//Trace of the evolution of the utilization over time
	vector<infrastructure_utilization_t*>* wholeInfrastructureOverTime; //It is the same as the one used in the Simulator
	vector<vector<datacenter_utilization_t*>*>* datacenterParameterOverTime;
	vector<reoptimization_summary_t*>* reoptimazationSummariesOverTime;
	vector<sla_violation_summary_t*>* SLAviolatioOverTime;


	central_controller_parameters_t* centralControllerParameters;

	GreenheadCentralControllerMysqlConnector* dataBaseConnector;

	//Variables for the threads to stop
	bool stopUtilizationReporting;
	bool stopSLAViolationReporting;
	bool stopReoptimization;

	//Some variable for the performance evaluation
	int nbTotalRequests, nbEmbeddedRequests;


	CentralController(central_controller_parameters_t* parameters);
	virtual ~CentralController();

	const SubstrateInfrastructure* getSubstrateInfrastructure() const {
		return substrateInfrastructure;
	}

	void setSubstrateInfrastructure( SubstrateInfrastructure* substrateInfrastructure) {
		this->substrateInfrastructure = substrateInfrastructure;
	}

	const vector<VDCRequestPartitioned*>* getVdcRequestsEmbedded() const {
		return vdcRequestsEmbedded;
	}

	void setVdcRequestsEmbedded(vector<VDCRequestPartitioned*>* vdcRequestsEmbedded) {
		this->vdcRequestsEmbedded = vdcRequestsEmbedded;
	}

	double getElectricityPrice(int idDC, double t);

	//
	double PartitionAndEmbedVDCRequest(request_t* request,double t);

	double embedVDCRequestGreedy(VDCRequestPartitioned* request, double t);
	double embedVDCRequestGreedyRecoveryFerbuary12(VDCRequestPartitioned* request, double t);
	double embedVDCRequestGreedyUsingEstimationOfRenewables(VDCRequestPartitioned* request, double t);

	double embedVDCRequestGreedyWithAdmissionControl(VDCRequestPartitioned* request, double actualAcceptanecRatio, double t);

	double embedVDCRequestToGetOptimalSolution(VDCRequestPartitioned* request, double t);

	double embedVDCRequestForLoadBalancingWithoutPartitioning(VDCRequestPartitioned* request, double t);

	double embedVDCRequestGreedyLoadBalancing(VDCRequestPartitioned* request, double t);

	double embedFromGivenEmbeddingSolution(EmbeddingSolution* embeddingSolution ,double t);

	double embedFromGivenEmbeddingSolutionWithoutEmbeddingLinksInBackboneNetwork(EmbeddingSolution* embeddingSolution ,double t);

	//Partition the VDC request
	VDCRequestPartitioned* partitionVDCRequestNoPartitioning(request_t* initialRequest);

	//Partition the VDC request using the Louvain algorithm, Authors of Louvain proposal
	VDCRequestPartitioned* partitionVDCRequestLouvainOriginalImplementation(request_t* initialRequest);

	//Partition the VDC request using the Louvain algorithm, Our modified version
	VDCRequestPartitioned* partitionVDCRequestLouvainAlgorithmModifiedVersion(request_t* initialRequest);

	VDCRequestPartitioned* partitionVDCRequestIntoEqualSizePartitions(request_t* initialRequest);

	VDCRequestPartitioned* partitionVDCRequestToGetOneSinglePartition(request_t* initialRequest);


	VDCRequestPartitioned* partitionRequest(request_t* initialRequest, int partitioingMethod);


	//Remove the partition that have been embedded
	double removeRequestWithLinksBetweenPartitions(request_t* request, double t, double* costDCs, double* costProvisionedNetwork);
	double removeRequestWithoutRemovingLinks(request_t* request, double t, double* costDCs, double* costProvisionedNetwork, bool addToPreviouslyEmbeddedRequests);
	double removeRequest(int idRequest, double t);

	//Get the instantaneous cost in the DCs and the provisioned network
	infrastructure_utilization_t* getInstanteousUtilizationWholeInfrastructure(double t);

	//Get instantaneous Revenue
	double getInstanteousRevenue(double t);

	double getInstanteousNumberOfEmbeddedRequests(double t);

	//Get the instantaneous cost in the DC DCid
	infrastructure_utilization_t* getInstanteousOperatingCostInDataCenter(double t, int DCid);

	//Get the instantaneous cost in the provisioned network
	infrastructure_utilization_t* getInstanteousOperatingCostInProvisionedNetwork(double t);

	//Update the instantaneous costs in the embedded request
	void updateInstantaneousCostOfEmbeddedRequests(double t);

	//Get the costs of the remaining requests in the
	double getCostRemainingRequests(double t, double* costDCs, double* costProvisionedNetwork);

	double getElectricityPrice(double t, location_t* location);

	vector<datacenter_utilization_t*>* getInstaneousUtilizationInDatacenters(double t);

	//Gives the instantaneous carbon emission per unit of VM and unit of bandwidth in data centers
	double getCarbonFootprintPerUnitOfVMInAllDatacenters(double t);
	//	double getCarbonFootprintPerUnitOfBandwidthInAllDatacenters(double t);
	double getCarbonFootprintPerUnitOfBandwidthInWholeInfrastructure(double t);

	//get the estimation of carbon emissions
	double getEstiamtionCarbonFootprintPerUnitOfVMInAllDatacentersForAPeriodOfTime(double tBegin, double tEnd);
	double getEstiamtionCarbonFootprintPerUnitOfBandwidthInWholeInfrastructureForAPeriodOfTime(double tBegin, double tEnd);

	//Gives the carbon emission per unit of VM in data centers for the reporting period that ends at t
	//t is the last time (last time slot, last second, last minute...) of the reporting period to which t belongs
	double getCarbonFootprintPerUnitOfVMInAllDatacentersForAServiceGuaranteePeriod(double t);
	double getCarbonFootprintPerUnitOfBandwidthInAllDatacentersForAServiceGuaranteePeriod(double t);

	double getInstantaneousCarbonFootrpintForARequest(request_t* request, double t); //Instantaneous Carbon emissions due to a VDC request

	//double getCarbonFootrpintForARequestForAServiceGuaranteePeriod(request_t* request, double t); //Carbon emissions due to a VDC request incured during a service time guarantee

	double getCarbonFootrpintForARequest(request_t* request, double CarbonPerVMUnit, double CarbonPerBWUnit); //Instantaneous Carbon emissions due to a VDC request

	bool admissionForRequest(VDCRequestPartitioned* request, double acceptanceRatioSoFar, double currentEmbeddingCost, double t); //YES or NO to accept the request or reject it because of the carbon limitations

	double getEstimationCostForARequestInTheFuture(VDCRequestPartitioned* request, double t);

	vector<sla_violation_per_request_t*>* getListViolatedSLAsForAServiceGuaranteePeriod(double t); //At the end of the time period T

	sla_violation_summary_t* getViolatedSLAsSummaryForAServiceGuaranteePeriod(double t);


	int getPeriodFromTime(double t);
	int getPeriodFromTimeSlot(int slot);
	int getTimeSlotFromTime(double t);


	//Migration

	//Migrate a partition from a source data center to a destination data center. The method returns the cost, or a FAILURE_MIGRATE if migration was not successful
	double migratePartition(partition_t* partition, VDCRequestPartitioned* requestPartitioned, int dataCenterSource, int dataCenterDestination, double tn);
	double getCostInBackboneToMigratePartition(partition_t* partition, VDCRequestPartitioned* requestPartitioned, int dataCenterSource, int dataCenterDestination);

	vector<partition_t*>* getPartitionsEmbeddedInDataCenter(int dataCenterID);
	VDCRequestPartitioned* getVDCRequestPartitionedOfAPartition(partition_t* partition);
	reoptimization_summary_t* migrateToOptimizeGreenPowerUsage(double t, double estimationTimeWindow); //Migrate partitions from datacenter with no renewables to data centers with renewables
	reoptimization_summary_t* migrateToOptimizeGreenPowerUsageConsideringBackboneNetworkCosts(double t, double estimationTimeWindow); //Migrate partitions from datacenter with no renewables to data centers with renewables

	//double getITPowerConsumedByRequest(int idRequest,double t);


	//Closing the connextions with data centers
	void closeConnexionsWithRemoteDataCenters();


	//Load the VDC request from the database and embed them
	void loadFromDataBaseAndEmbedRequstes(double t);

	int getHighestRequestID(); //Get the highest request ID of the Requests that are embedded


	//At the end, the statistic are written into files
	void writeResultsInFiles(simulation_parameters_t simulationParameters);



	//This is the daemon function to collect data from the data centers and the backbone network
	void daemonCollectInfrastructureUtilization(double timeStart);
	void daemonCollectSLAViolations(double timeStart);
	void daemonReoptimization(double timeStart);


	static void* staticDaemonCollectInfrastructureUtilization(void* cn);
	static void* staticDaemonCollectSLAViolations(void* cn);
	static void* staticDaemonReoptimization(void* cn);


	void startThreadsToCollectStatistics(double timeStart);
	void stopThreadsCollectingStatistics();

};


#endif /* CENTRALCONTROLLER_H_ */
