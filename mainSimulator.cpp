/*
 * mainSimulator.cpp
 *
 *  Created on: Dec 28, 2012
 *      Author: ahmedamokrane
 */

#include "mainSimulator.h"
#include<stdio.h>
#include<vector>
#include <cstdlib>
#include "Embed.h"
#include "def.h"



mainSimulator::mainSimulator() {
	idRqsts = 1;
	centralControllerParmaeters = new central_controller_parameters_t;
	simulationParameters = new simulation_parameters_t;
	this->seedInitialValue = 1;
}

mainSimulator::~mainSimulator() {
	// TODO Auto-generated destructor stub

	//vector<vector<datacenter_utilization_t*>* >* datacenterParameterOverTime; //One element (vector<datacenter_utilization_t*>*) per instant of observation, Every element contains the list of the datacenters
	//vector<provisioned_network_utilization_t*>* wholeInfrastructureOverTime;


}

VDCRequestPartitioned* mainSimulator::partitionRequest(request_t* initialRequest, CentralController* cn, int partitioingMethod) {
	VDCRequestPartitioned* vdcPartitioned = NULL;
	switch(partitioingMethod){
	case PARTITIONING_NODE_IS_PARTITION :{
		//Every node in the VDC is considered as a partition
		vdcPartitioned = cn->partitionVDCRequestNoPartitioning(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION :{
		//Every node in the VDC is considered as a partition
		vdcPartitioned = cn->partitionVDCRequestNoPartitioning(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_LOUVAIN :{
		//Use the LOUVAIN Algorithm
		vdcPartitioned = cn->partitionVDCRequestLouvainAlgorithmModifiedVersion(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION :{
		//Use the LOUVAIN Algorithm
		vdcPartitioned = cn->partitionVDCRequestLouvainAlgorithmModifiedVersion(initialRequest);
		return vdcPartitioned;
	}
	break;

	case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL :{
		//Use the LOUVAIN Algorithm
		vdcPartitioned = cn->partitionVDCRequestLouvainAlgorithmModifiedVersion(initialRequest);
		return vdcPartitioned;
	}
	break;



	case PARTITIONING_EQUAL_PARTITIONS_SIZE :{
		//Partition into equal size partitions
		//vdcPartitioned = cn->partitionVDCRequestIntoEqualSizePartitions(initialRequest);
		vdcPartitioned = cn->partitionVDCRequestIntoEqualSizePartitions(initialRequest);
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

void mainSimulator::updateInstantaneousCostOfEmbededRequests(double t) {
	/*
	 * Update the cost of the previously embedded requests
	 */

	//Modify the cost of already embedded requests
	this->cn->updateInstantaneousCostOfEmbeddedRequests(t);

}


infrastructure_utilization_t* mainSimulator::startSimulation(){

	//Initialize the seed

	srand(this->seedInitialValue);

	//The parameters of the simulation are already updated in the main method that instantiates this

	r = ((double) rand() / (RAND_MAX));

	//Generate the physical infrastructure

	cout << "Building the infrastructure network" << endl;
	//Building the physical infrastructure

	//SubstrateInfrastructure* substrateInf = this->generateTopology(numberOfNodesProvisionedNetwork, numberOfDatacenters, physicalLinkProbability);
	//SubstrateInfrastructure* substrateInf = this->generateTopology(this->provisionedNetworkTopology);
	SubstrateInfrastructure* substrateInf = TopologyGenerator::generateTopology(this->centralControllerParmaeters);

	substrateInf->buildAlternativePathsBetweenDataCenters(this->centralControllerParmaeters->numberOfAlternativePaths);

	//Create the central controller
	cn = new CentralController(this->centralControllerParmaeters);
	cn->substrateInfrastructure = substrateInf;

	//Initialize the variables of performance evaluation
	cn->datacenterParameterOverTime = new vector< vector<datacenter_utilization_t*>* >();
	cn->wholeInfrastructureOverTime = new vector<infrastructure_utilization_t*>();
	cn->reoptimazationSummariesOverTime = new vector<reoptimization_summary_t*>();

	cn->SLAviolatioOverTime = new vector<sla_violation_summary_t*>();

	cn->centralControllerParameters = this->centralControllerParmaeters;

	//These variables will be used for the performance evaluation

	double sumCostProvisionedNetwork = 0.0;
	double sumCostInDatacenter = 0.0;
	double sumAllCost = 0.0;

	int nbEmbeddedRequests = 0;
	int nbTotalRequests = 0, nbRejectedRequests = 0;

	double timeComlexityPartitioningSoFar = 0;
	double timeComplexityEmbeddingSoFar = 0;

	//Create the priority queue that will hold the events
	event_t* eventQueue = NULL;


	double currentTime = 0;
	//Create the first Event
	event_t* ev = new event_t;
	ev->reqid = NULL;
	ev->time = currentTime;
	ev->etype = REPORT_INSTANTNEOUS_PARAMETERS;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);


	ev = new event_t;
	ev->reqid = NULL;
	ev->time = currentTime + (this->centralControllerParmaeters->SLAServiceGuaranteePeriod*0.999);
	ev->etype = SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);


	//Create the last event
	ev = new event_t;
	ev->reqid = NULL;
	ev->time = currentTime + this->simulationParameters->simDuration;
	ev->etype = REPORT_INSTANTNEOUS_PARAMETERS;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);


	//The parameter changing Event

	//ev = new event_t;
	//ev->reqid = NULL;
	//ev->time = currentTime + this->parameterUpdateInterval;
	//ev->etype = PUE_CHANGE_EVENT;
	//ev->next = NULL;
	//eventQueue = ev;
	//this->insertEventInQueue(&eventQueue, ev);



	//The Next request
	request_t* rqst = this->generateNextRequest(currentTime);
	ev = new event_t;
	ev->reqid = rqst;
	ev->time = rqst->arrtime;
	ev->etype = ARRIVAL_EVENT;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);

	//The migration to optimize the renewables utilization
	if(this->centralControllerParmaeters->partitioningMethod == PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION || this->centralControllerParmaeters->partitioningMethod == PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL){
		ev = new event_t;
		ev->reqid = NULL;
		ev->time = currentTime + this->centralControllerParmaeters->reoptimizationInterval;
		ev->etype = MIGRATION_EVENT;
		ev->next = NULL;
		//eventQueue = ev;
		this->insertEventInQueue(&eventQueue, ev);
	}



	while(eventQueue!=NULL && eventQueue->time <= this->simulationParameters->simDuration){
		//dequeue the event from the queue
		event_t* currEvent = eventQueue;
		eventQueue = eventQueue->next;
		currentTime = currEvent->time;

		//Which kind of event do we have right now?
		switch(currEvent->etype){
		case ARRIVAL_EVENT :{
			//Arrival of a request
			//Embed this request, then generate the next request

			//Partition and embed the current request

			clock_t startPartitioning=clock();
			VDCRequestPartitioned* rsqtPartitioned = cn->partitionRequest(currEvent->reqid, this->centralControllerParmaeters->partitioningMethod);
			clock_t endPartitioning=clock();
			double diffPartitioning = (endPartitioning - startPartitioning)/(double)CLOCKS_PER_SEC;
			double cost;
			clock_t startEmbedding=clock();

			switch(this->centralControllerParmaeters->partitioningMethod){
			case PARTITIONING_NODE_IS_PARTITION :{
				//Every node in the VDC is considered as a partition
				cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION :{
				//Every node in the VDC is considered as a partition
				//cost = cn->embedVDCRequestToGetOptimalSolution(rsqtPartitioned, currentTime);
				cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_LOUVAIN :{
				//Use the LOUVAIN Algorithm
				cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION :{
				//Use the LOUVAIN Algorithm
				//cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
				cost = cn->embedVDCRequestGreedyUsingEstimationOfRenewables(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL :{
				//Use the LOUVAIN Algorithm

				double acceptanceRatio = 0;
				if(nbTotalRequests>0){
					acceptanceRatio = (double)nbEmbeddedRequests / (double)nbTotalRequests;
				}

				cost = cn->embedVDCRequestGreedyWithAdmissionControl(rsqtPartitioned, acceptanceRatio, currentTime);

			}
			break;

			case PARTITIONING_EQUAL_PARTITIONS_SIZE :{
				//Partition into equal size partitions
				cost = cn->embedVDCRequestGreedyLoadBalancing(rsqtPartitioned, currentTime);
			}
			break;

			default:{
				//There is a problem, no partitioning method is specified.
				//Should rise an exception, no partitioning specified

			}
			break;
			}



			clock_t endEmbedding=clock();
			double diffEmbedding = (endEmbedding - startEmbedding)/(double)CLOCKS_PER_SEC;

			nbTotalRequests++;
			if(cost >= 0){
				nbEmbeddedRequests++;
				timeComlexityPartitioningSoFar += diffPartitioning;
				timeComplexityEmbeddingSoFar += diffEmbedding;

				//Create the end of the request event;
				ev = new event_t;
				ev->reqid = currEvent->reqid;
				ev->time = currEvent->reqid->arrtime + currEvent->reqid->duration;
				ev->etype = LEAVE_EVENT;
				ev->next = NULL;
				this->insertEventInQueue(&eventQueue, ev);
			}else{
				nbRejectedRequests++;
				cout << "Request "<< currEvent->reqid->reqID <<" is rejected" << endl;
				//Remove the leave event
				//this->removeLeaveEventOfRequestInQueue(&eventQueue, currEvent->reqid);

			}


			//Create the next arrival event
			rqst = this->generateNextRequest(currentTime);
			ev = new event_t;
			ev->reqid = rqst;
			rqst->arrtime += currentTime;
			ev->time = rqst->arrtime;
			ev->etype = ARRIVAL_EVENT;
			ev->next = NULL;
			this->insertEventInQueue(&eventQueue, ev);


		}
		break;

		case LEAVE_EVENT :{
			//Departure of a request
			//Free the resources in the Infrastructure
			double costDCs = 0, costProvis = 0;
			double cost = 0;
			/*
			if(this->partitioningMethod == PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION){
				cost = cn->removeRequestWithoutRemovingLinks(currEvent->reqid, currentTime, &costDCs, &costProvis, true);
			}else{
				cost = cn->removeRequestWithLinksBetweenPartitions(currEvent->reqid, currentTime, &costDCs, &costProvis);
			}
			 */

			cost = cn->removeRequestWithLinksBetweenPartitions(currEvent->reqid, currentTime, &costDCs, &costProvis);

			if(cost<0){
				cout << "OUUUUUCH, Request "<< currEvent->reqid->reqID <<" is not in the list" << endl;
			}

			sumCostProvisionedNetwork += costProvis;
			sumCostInDatacenter = costDCs;
			sumAllCost = cost;

		}
		break;

		case RECONFIGURATION_EVENT:{
			//Reconfiguration in the embedding of the different requests
			//Migration....


		}
		break;


		case MIGRATION_EVENT:{
			//Migration from data centers with small ammounts of renewables to data centers with high amount of renewables
			reoptimization_summary_t* migrationSummary = this->cn->migrateToOptimizeGreenPowerUsage(currentTime, this->centralControllerParmaeters->lookAheadTimeForForcasting);
			cn->reoptimazationSummariesOverTime->push_back(migrationSummary);
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + this->centralControllerParmaeters->reoptimizationInterval;
			ev->etype = MIGRATION_EVENT;
			ev->next = NULL;
			//eventQueue = ev;
			this->insertEventInQueue(&eventQueue, ev);
		}
		break;



		case PUE_CHANGE_EVENT:{
			//The PUE of datacenters changes
			//Change all the instantaneous costs in the different datacenters

			/*
			//Modify the cost for the different partitions that have been embedded so far
			this->updateInstantaneousCostOfEmbededRequests(currentTime);
			//generate the next parameter changing event
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + this->parameterUpdateInterval;
			ev->etype = PUE_CHANGE_EVENT;
			ev->next = NULL;
			this->insertEventInQueue(&eventQueue, ev);
			 */

		}
		break;
		case REPORT_INSTANTNEOUS_PARAMETERS:{
			//This Event is added for performance Evaluation
			//Report the Evolution of the datacenter costs and the utilization
			vector<datacenter_utilization_t*>* actualUtilizationInDCs = cn->getInstaneousUtilizationInDatacenters(currentTime);
			cn->datacenterParameterOverTime->push_back(actualUtilizationInDCs);

			//Backbone network
			infrastructure_utilization_t* par;
			par = cn->getInstanteousUtilizationWholeInfrastructure(currentTime);
			par->acceptanceRatio = 0;

			if(nbTotalRequests>0){
				par->acceptanceRatio = (double)nbEmbeddedRequests / (double)nbTotalRequests;
				par->numberOfTotalRequestsSoFar = nbTotalRequests;
				par->numberOfEmbeddedRequestsSoFar = nbEmbeddedRequests;
				if(nbEmbeddedRequests > 0){
					par->partitioningTimeComplexityPerRequest = timeComlexityPartitioningSoFar/nbEmbeddedRequests;
					par->embeddingTimeComplexityPerRequest = timeComplexityEmbeddingSoFar/nbEmbeddedRequests;
				}else{
					par->partitioningTimeComplexityPerRequest = 0;
					par->embeddingTimeComplexityPerRequest = 0;
				}
			}
			cn->wholeInfrastructureOverTime->push_back(par);
			//Generate the next reporting Event
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + this->centralControllerParmaeters->performanceEvaluationReportingInterval;
			ev->etype = REPORT_INSTANTNEOUS_PARAMETERS;
			ev->next = NULL;
			//eventQueue = ev;
			this->insertEventInQueue(&eventQueue, ev);
		}
		break;


		case SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED:{
			//The InO should check if the SLA in Carbon Emissions are guaranteed

			sla_violation_summary_t* par = cn->getViolatedSLAsSummaryForAServiceGuaranteePeriod(currentTime);
			cn->SLAviolatioOverTime->push_back(par);

			//Generate the next reporting Event
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + cn->centralControllerParameters->SLAServiceGuaranteePeriod;
			ev->etype = SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED;
			ev->next = NULL;
			//eventQueue = ev;
			this->insertEventInQueue(&eventQueue, ev);
		}
		break;



		}

		delete currEvent;

	}//END WHILE ON SIMULATION TIME


	//Here We should add the costs of the request that are still there
	double remainingCostDC, remainingCostProvisioned;
	cn->getCostRemainingRequests(this->simulationParameters->simDuration, &remainingCostDC, &remainingCostProvisioned);

	sumCostInDatacenter += remainingCostDC;
	sumCostProvisionedNetwork += remainingCostProvisioned;

	infrastructure_utilization_t* res = new infrastructure_utilization_t;
	res->costDatacenters = sumCostInDatacenter;
	res->costForUsedNodes = sumCostProvisionedNetwork;
	res->acceptanceRatio = nbEmbeddedRequests/nbTotalRequests;

	cout << "End of simulation" << endl;

	return res;

	//return SUCCESS_OPERATION;
}



infrastructure_utilization_t* mainSimulator::startSimulationFromListOfEvents(event_t* listEvents){

	cout << "Building the infrastructure network" << endl;
	//Building the physical infrastructure

	SubstrateInfrastructure* substrateInf = TopologyGenerator::generateTopology(this->centralControllerParmaeters);

	substrateInf->buildAlternativePathsBetweenDataCenters(this->centralControllerParmaeters->numberOfAlternativePaths);

	//Create the central controller
	cn = new CentralController(this->centralControllerParmaeters);
	cn->substrateInfrastructure = substrateInf;

	//Initialize the variables of performance evaluation
	cn->datacenterParameterOverTime = new vector< vector<datacenter_utilization_t*>* >();
	cn->wholeInfrastructureOverTime = new vector<infrastructure_utilization_t*>();
	cn->reoptimazationSummariesOverTime = new vector<reoptimization_summary_t*>();
	cn->SLAviolatioOverTime = new vector<sla_violation_summary_t*>();

	cn->centralControllerParameters = this->centralControllerParmaeters;


	//These variables will be used for the performance evaluation

	double sumCostProvisionedNetwork = 0.0;
	double sumCostInDatacenter = 0.0;
	double sumAllCost = 0.0;

	int nbEmbeddedRequests = 0;
	int nbTotalRequests = 0, nbRejectedRequests = 0;

	double timeComlexityPartitioningSoFar = 0;
	double timeComplexityEmbeddingSoFar = 0;

	//Create the priority queue that will hold the events
	event_t* eventQueueToRun = &(*listEvents);

	double currentTime = 0;


	while(eventQueueToRun!=NULL && eventQueueToRun->time <= this->simulationParameters->simDuration){
		//dequeue the event from the queue

		event_t* currEvent = eventQueueToRun;
		eventQueueToRun = eventQueueToRun->next;
		currentTime = currEvent->time;

		//Which kind of event do we have right now?
		switch(currEvent->etype){
		case ARRIVAL_EVENT :{
			//Arrival of a request
			//Embed this request, then generate the next request
			cout <<"Request Arrival Event "<<endl;

			//Partition and embed the current request

			clock_t startPartitioning=clock();
			VDCRequestPartitioned* rsqtPartitioned = this->partitionRequest(currEvent->reqid, cn, this->centralControllerParmaeters->partitioningMethod);
			clock_t endPartitioning=clock();
			double diffPartitioning = (endPartitioning - startPartitioning)/(double)CLOCKS_PER_SEC;
			double cost;
			clock_t startEmbedding=clock();

			switch(this->centralControllerParmaeters->partitioningMethod){
			case PARTITIONING_NODE_IS_PARTITION :{
				//Every node in the VDC is considered as a partition
				cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION :{
				//Every node in the VDC is considered as a partition
				//cost = cn->embedVDCRequestToGetOptimalSolution(rsqtPartitioned, currentTime);
				cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_LOUVAIN :{
				//Use the LOUVAIN Algorithm
				cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION :{
				//Use the LOUVAIN Algorithm
				cost = cn->embedVDCRequestGreedy(rsqtPartitioned, currentTime);
				//cost = cn->embedVDCRequestGreedyUsingEstimationOfRenewables(rsqtPartitioned, currentTime);
			}
			break;

			case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL :{
				//Use the LOUVAIN Algorithm

				double acceptanceRatio = 0;
				if(nbTotalRequests>0){
					acceptanceRatio = (double)nbEmbeddedRequests / (double)nbTotalRequests;
				}

				cost = cn->embedVDCRequestGreedyWithAdmissionControl(rsqtPartitioned, acceptanceRatio, currentTime);

			}
			break;

			case PARTITIONING_EQUAL_PARTITIONS_SIZE :{
				//Partition into equal size partitions
				cost = cn->embedVDCRequestGreedyLoadBalancing(rsqtPartitioned, currentTime);
			}
			break;

			default:{
				//There is a problem, no partitioning method is specified.
				//Should rise an exception, no partitioning specified

			}
			break;
			}



			clock_t endEmbedding=clock();
			double diffEmbedding = (endEmbedding - startEmbedding)/(double)CLOCKS_PER_SEC;

			nbTotalRequests++;
			if(cost >= 0){
				nbEmbeddedRequests++;
				timeComlexityPartitioningSoFar += diffPartitioning;
				timeComplexityEmbeddingSoFar += diffEmbedding;

			}else{
				nbRejectedRequests++;
				cout << "Request "<< currEvent->reqid->reqID <<" is rejected" << endl;
				//Remove the leave event
				//this->removeLeaveEventOfRequestInQueue(&eventQueueToRun, currEvent->reqid);
			}

		}
		break;

		case LEAVE_EVENT :{
			//Departure of a request
			//Free the resources in the Infrastructure
			cout <<"Leave Event "<<endl;
			double costDCs = 0, costProvis = 0;
			double cost = 0;
			/*
			if(this->partitioningMethod == PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION){
				cost = cn->removeRequestWithoutRemovingLinks(currEvent->reqid, currentTime, &costDCs, &costProvis, true);
			}else{
				cost = cn->removeRequestWithLinksBetweenPartitions(currEvent->reqid, currentTime, &costDCs, &costProvis);
			}
			 */

			cost = cn->removeRequestWithLinksBetweenPartitions(currEvent->reqid, currentTime, &costDCs, &costProvis);

			if(cost<0){
				cout << "OUUUUUCH, Request "<< currEvent->reqid->reqID <<" is not in the list" << endl;
			}

			sumCostProvisionedNetwork += costProvis;
			sumCostInDatacenter = costDCs;
			sumAllCost = cost;

		}
		break;

		case RECONFIGURATION_EVENT:{
			//Reconfiguration in the embedding of the different requests
			//Migration....


		}
		break;


		case MIGRATION_EVENT:{
			cout <<"Reconfiguration Event "<<endl;
			//Migration from data centers with small ammounts of renewables to data centers with high amount of renewables
			if(this->centralControllerParmaeters->partitioningMethod == PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION || this->centralControllerParmaeters->partitioningMethod == PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL){
				//reoptimization_summary_t* migrationSummary = this->cn->migrateToOptimizeGreenPowerUsage(currentTime, this->lookAheadTimeForForcasting);
				reoptimization_summary_t* migrationSummary = this->cn->migrateToOptimizeGreenPowerUsageConsideringBackboneNetworkCosts(currentTime, this->centralControllerParmaeters->lookAheadTimeForForcasting);
				cn->reoptimazationSummariesOverTime->push_back(migrationSummary);
			}
		}
		break;



		case PUE_CHANGE_EVENT:{
			//The PUE of datacenters changes
			//Change all the instantaneous costs in the different datacenters

		}
		break;
		case REPORT_INSTANTNEOUS_PARAMETERS:{
			//This Event is added for performance Evaluation
			//Report the Evolution of the datacenter costs and the utilization

			cout <<"Reporting Instantaneous utilization in the Infrastructure Event"<<endl;

			vector<datacenter_utilization_t*>* actualUtilizationInDCs = cn->getInstaneousUtilizationInDatacenters(currentTime);
			cn->datacenterParameterOverTime->push_back(actualUtilizationInDCs);

			//Backbone network
			infrastructure_utilization_t* par;
			par = cn->getInstanteousUtilizationWholeInfrastructure(currentTime);
			par->acceptanceRatio = 0;

			if(nbTotalRequests>0){
				par->acceptanceRatio = (double)nbEmbeddedRequests / (double)nbTotalRequests;
				par->numberOfTotalRequestsSoFar = nbTotalRequests;
				par->numberOfEmbeddedRequestsSoFar = nbEmbeddedRequests;
				if(nbEmbeddedRequests > 0){
					par->partitioningTimeComplexityPerRequest = timeComlexityPartitioningSoFar/nbEmbeddedRequests;
					par->embeddingTimeComplexityPerRequest = timeComplexityEmbeddingSoFar/nbEmbeddedRequests;
				}else{
					par->partitioningTimeComplexityPerRequest = 0;
					par->embeddingTimeComplexityPerRequest = 0;
				}
			}
			cn->wholeInfrastructureOverTime->push_back(par);
			//Generate the next reporting Event
		}
		break;


		case SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED:{
			//The InO should check if the SLA in Carbon Emissions are guaranteed

			sla_violation_summary_t* par = cn->getViolatedSLAsSummaryForAServiceGuaranteePeriod(currentTime);
			cn->SLAviolatioOverTime->push_back(par);

		}
		break;


		}

		//delete currEvent;

	}//END WHILE ON SIMULATION TIME


	//Here We should add the costs of the request that are still there
	//double remainingCostDC, remainingCostProvisioned;
	//cn->getCostRemainingRequests(this->simDuration, &remainingCostDC, &remainingCostProvisioned);

	//sumCostInDatacenter += remainingCostDC;
	//sumCostProvisionedNetwork += remainingCostProvisioned;

	infrastructure_utilization_t* res = new infrastructure_utilization_t;
	res->costDatacenters = sumCostInDatacenter;
	res->costForUsedNodes = sumCostProvisionedNetwork;
	res->acceptanceRatio = nbEmbeddedRequests/nbTotalRequests;

	//We should go through the list of events and modify the mapping of all the links and nodes
	eventQueueToRun = listEvents;
	while(eventQueueToRun != NULL){
		event_t* currEvent = eventQueueToRun;
		eventQueueToRun = eventQueueToRun->next;
		if(currEvent->etype == ARRIVAL_EVENT){
			request_t* rqst = currEvent->reqid;
			int nbLinks = rqst->links;
			for(int i=0; i<nbLinks; i++){
				link_t* l = rqst->link[i];
				//delete l->mapping->substratePath;
				//delete l->mapping;
				l->mapping = NULL;
			}
		}
	}

	//Close the connexions with remote data centers

	this->cn->closeConnexionsWithRemoteDataCenters();


	cout << "End of simulation" << endl;

	return res;

	//return SUCCESS_OPERATION;
}




event_t** mainSimulator::generateListEventsForSimulation(){

	//Initialize the seed

	srand(this->seedInitialValue);

	//The parameters of the simulation are already updated in the main method that instantiates this

	r = ((double) rand() / (RAND_MAX));

	//Generate the physical infrastructure

	cout << "Building the infrastructure network" << endl;
	//Building the physical infrastructure

	//SubstrateInfrastructure* substrateInf = this->generateTopology(numberOfNodesProvisionedNetwork, numberOfDatacenters, physicalLinkProbability);
	//SubstrateInfrastructure* substrateInf = this->generateTopology();

	//substrateInf->buildAlternativePathsBetweenDataCenters(this->numberOfAlternativePaths);

	//Create the central controller
	cn = new CentralController(this->centralControllerParmaeters);
	//cn->substrateInfrastructure = substrateInf;

	//Initialize the variables of performance evaluation
	cn->datacenterParameterOverTime = new vector< vector<datacenter_utilization_t*>* >();
	cn->wholeInfrastructureOverTime = new vector<infrastructure_utilization_t*>();
	cn->reoptimazationSummariesOverTime = new vector<reoptimization_summary_t*>();

	cn->SLAviolatioOverTime = new vector<sla_violation_summary_t*>();

	cn->centralControllerParameters = this->centralControllerParmaeters;

	//These variables will be used for the performance evaluation

	//Create the priority queue that will hold the events
	event_t** listEventResult = new event_t*;
	event_t* eventQueue = NULL;

	double currentTime = 0;
	//Create the first Event
	event_t* ev = new event_t;
	ev->reqid = NULL;
	ev->time = currentTime;
	ev->etype = REPORT_INSTANTNEOUS_PARAMETERS;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);

	*listEventResult = &(*eventQueue);

	ev = new event_t;
	ev->reqid = NULL;
	ev->time = currentTime + (this->centralControllerParmaeters->SLAServiceGuaranteePeriod*0.999);
	ev->etype = SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);


	//Create the last event
	ev = new event_t;
	ev->reqid = NULL;
	ev->time = currentTime + this->simulationParameters->simDuration;
	ev->etype = REPORT_INSTANTNEOUS_PARAMETERS;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);


	//The parameter changing Event

	//ev = new event_t;
	//ev->reqid = NULL;
	//ev->time = currentTime + this->parameterUpdateInterval;
	//ev->etype = PUE_CHANGE_EVENT;
	//ev->next = NULL;
	//eventQueue = ev;
	//this->insertEventInQueue(&eventQueue, ev);



	//The Next request
	request_t* rqst = this->generateNextRequest(currentTime);
	ev = new event_t;
	ev->reqid = rqst;
	ev->time = rqst->arrtime;
	ev->etype = ARRIVAL_EVENT;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);

	//The migration to optimize the renewables utilization
	//if(this->partitioningMethod == PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION || this->partitioningMethod == PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL){
	ev = new event_t;
	ev->reqid = NULL;
	ev->time = currentTime + this->centralControllerParmaeters->reoptimizationInterval;
	ev->etype = MIGRATION_EVENT;
	ev->next = NULL;
	//eventQueue = ev;
	this->insertEventInQueue(&eventQueue, ev);
	//}



	while(eventQueue!=NULL && eventQueue->time <= this->simulationParameters->simDuration){
		//dequeue the event from the queue
		event_t* currEvent = eventQueue;
		currentTime = currEvent->time;

		//Which kind of event do we have right now?
		switch(currEvent->etype){
		case ARRIVAL_EVENT :{
			//Arrival of a request

			//Create the end of the request event;
			ev = new event_t;
			ev->reqid = currEvent->reqid;
			ev->time = currEvent->reqid->arrtime + currEvent->reqid->duration;
			ev->etype = LEAVE_EVENT;
			ev->next = NULL;
			this->insertEventInQueue(&eventQueue, ev);


			//Create the next arrival event
			rqst = this->generateNextRequest(currentTime);
			ev = new event_t;
			ev->reqid = rqst;
			rqst->arrtime += currentTime;
			ev->time = rqst->arrtime;
			ev->etype = ARRIVAL_EVENT;
			ev->next = NULL;
			this->insertEventInQueue(&eventQueue, ev);


		}
		break;

		case LEAVE_EVENT :{

		}
		break;

		case RECONFIGURATION_EVENT:{
			//Reconfiguration in the embedding of the different requests
			//Migration....


		}
		break;


		case MIGRATION_EVENT:{
			//Migration from data centers with small ammounts of renewables to data centers with high amount of renewables
			//reoptimization_summary_t* migrationSummary = this->cn->migrateToOptimizeGreenPowerUsage(currentTime, this->lookAheadTimeForForcasting);
			//this->reoptimazationSummariesOverTime->push_back(migrationSummary);
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + this->centralControllerParmaeters->reoptimizationInterval;
			ev->etype = MIGRATION_EVENT;
			ev->next = NULL;
			//eventQueue = ev;
			this->insertEventInQueue(&eventQueue, ev);
		}
		break;



		case PUE_CHANGE_EVENT:{
			//The PUE of datacenters changes
			//Change all the instantaneous costs in the different datacenters

			/*
			//Modify the cost for the different partitions that have been embedded so far
			this->updateInstantaneousCostOfEmbededRequests(currentTime);
			//generate the next parameter changing event
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + this->parameterUpdateInterval;
			ev->etype = PUE_CHANGE_EVENT;
			ev->next = NULL;
			this->insertEventInQueue(&eventQueue, ev);
			 */

		}
		break;
		case REPORT_INSTANTNEOUS_PARAMETERS:{
			//This Event is added for performance Evaluation
			//Report the Evolution of the datacenter costs and the utilization
			//Generate the next reporting Event
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + this->centralControllerParmaeters->performanceEvaluationReportingInterval;
			ev->etype = REPORT_INSTANTNEOUS_PARAMETERS;
			ev->next = NULL;
			//eventQueue = ev;
			this->insertEventInQueue(&eventQueue, ev);
		}
		break;


		case SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED:{
			//The InO should check if the SLA in Carbon Emissions are guaranteed

			sla_violation_summary_t* par = cn->getViolatedSLAsSummaryForAServiceGuaranteePeriod(currentTime);
			cn->SLAviolatioOverTime->push_back(par);

			//Generate the next reporting Event
			ev = new event_t;
			ev->reqid = NULL;
			ev->time = currentTime + this->centralControllerParmaeters->SLAServiceGuaranteePeriod;
			ev->etype = SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED;
			ev->next = NULL;
			//eventQueue = ev;
			this->insertEventInQueue(&eventQueue, ev);
		}
		break;



		}

		//Go to next event
		eventQueue = eventQueue->next;
		//delete currEvent;

	}//END WHILE ON SIMULATION TIME

	return listEventResult;

	//return SUCCESS_OPERATION;
}



request_t* mainSimulator::generateNextRequest(double t){
	request_t* rqst;
	double r;
	switch(this->simulationParameters->requestGeneration){

	case RANDOM:{
		//Create the request according to random generation
		cout << "Building a request" << endl;
		int numberOfNodes = 1;
		switch(simulationParameters->numberOfVirtualNodesDistribution){
		case POISSON :
		{
			r = ((double) rand() / (RAND_MAX));
			double nbNodesExpo = - this->simulationParameters->minNumberOfVirtualNodes * log(1-r) ;
			numberOfNodes = (int) nbNodesExpo;
		}
		break;

		case CONSTANT:
		{
			numberOfNodes = simulationParameters->minNumberOfVirtualNodes;
		}
		break;

		case UNIFORM:
		{
			//Uniform number of nodes
			r = ((double)rand())/RAND_MAX;
			numberOfNodes = this->simulationParameters->minNumberOfVirtualNodes + (int)(r*(double)(this->simulationParameters->maxNumberOfVirtualNodes - this->simulationParameters->minNumberOfVirtualNodes));
		}
		break;
		}

		double arriv;
		double lifetime;

		//Generate the time of arrival and leave of the request
		switch(simulationParameters->arrivaleRateDistribution){
		case POISSON :{
			r = ((double) rand() / (RAND_MAX));
			arriv = - (1/this->simulationParameters->arrivalRate)*(log(1-r));
		}
		break;
		case CONSTANT:{
			arriv =  1/this->simulationParameters->arrivalRate;
		}
		break;
		default:{
			arriv =  1/this->simulationParameters->arrivalRate;
		}
		break;
		}


		switch(simulationParameters->lifeTimeDistribution){
		case EXPONENTIAL :{
			r = ((double) rand() / (RAND_MAX));
			lifetime = - this->simulationParameters->meanLifetime*(log(1-r));
		}
		break;
		case CONSTANT:{
			lifetime =  this->simulationParameters->meanLifetime;
		}
		break;
		default:{
			lifetime =  this->simulationParameters->meanLifetime;
		}
		break;
		}


		//Generate the carbon emission limits of the SLA of the request
		double carbonLimit;
		switch(this->simulationParameters->carbonSLAConstraintDistribution){
		case EXPONENTIAL :{
			r = ((double) rand() / (RAND_MAX));
			carbonLimit = - this->simulationParameters->carbonSLAConstraintLimitInf*(log(1-r));
		}
		break;
		case CONSTANT:{
			carbonLimit =  this->simulationParameters->carbonSLAConstraintLimitInf;
		}
		break;
		case UNIFORM:{
			r = ((double) rand() / (RAND_MAX));
			carbonLimit =  this->simulationParameters->carbonSLAConstraintLimitInf + r*(this->simulationParameters->carbonSLAConstraintLimitSup - this->simulationParameters->carbonSLAConstraintLimitInf);
		}
		break;

		default:{
			carbonLimit =  this->simulationParameters->carbonSLAConstraintLimitInf;
		}
		break;
		}


		//Generate start requests with some probability

		r = ((double) rand() / (RAND_MAX));
		if(r < 0.25){
			rqst = this->generateRandomRequest(numberOfNodes, simulationParameters->virtualLinkProbability,t);
		}else{
			rqst = this->generateRandomRequestWithStars(numberOfNodes, simulationParameters->virtualLinkProbability, t);
		}

		//rqst = this->generateRandomRequest(numberOfNodes, virtualLinkProbability);

		//To Change
		//Make the request a long lived one
		r = ((double) rand() / (RAND_MAX));
		if(r < 0.05){
			//Some background requests
			lifetime = lifetime*4;
		}

		rqst->arrtime = arriv;
		rqst->duration = lifetime;
		rqst->limitOfCarbonEmissionPerServicePeriodGuarantee = carbonLimit;

	}
	break;

	case GOOGLE:{
		//Read the request from the google traces

	}
	break;

	case TEST_TEST:{
		//Generate one single totpology for tests
		double arriv;
		double lifetime;

		//Generate the time of arrival and leave of the request
		switch(simulationParameters->arrivaleRateDistribution){
		case POISSON :{
			r = ((double) rand() / (RAND_MAX));
			arriv = - (1/this->simulationParameters->arrivalRate)*(log(1-r));
		}
		break;
		case CONSTANT:{
			arriv =  1/this->simulationParameters->arrivalRate;
		}
		break;
		default:{
			arriv =  1/this->simulationParameters->arrivalRate;
		}
		break;
		}

		switch(simulationParameters->lifeTimeDistribution){
		case EXPONENTIAL :{
			r = ((double) rand() / (RAND_MAX));
			lifetime = - this->simulationParameters->meanLifetime*(log(1-r));
		}
		break;
		case CONSTANT:{
			lifetime =  this->simulationParameters->meanLifetime;
		}
		break;
		default:{
			lifetime =  this->simulationParameters->meanLifetime;
		}
		break;
		}

		rqst = this->generateTestRequest();
		rqst->arrtime = arriv;
		rqst->duration = lifetime;
	}
	break;


	default:
		cout << "No virtual request generation method" << endl;
		break;
	}//END SWITCH numberOfVirtualNodesDistribution
	return rqst;
}



void mainSimulator::insertEventInQueue(event_t** eventQ, event_t* evnt){

	if(*eventQ == NULL){
		//The event list is empty, insert at the beginning of the list
		*eventQ = evnt;
	}else{
		//bool done = false;
		event_t *p,*q;
		q = NULL;
		p = *eventQ;
		while(p!=NULL && p->time <= evnt->time){
			q = p;
			p = p->next;
		}
		evnt->next = p;
		if(q!=NULL){
			q->next = evnt;
		}else{
			*eventQ = evnt;
		}
	}
}


void mainSimulator::removeLeaveEventOfRequestInQueue(event_t** eventQ, request_t* rqst) {
	event_t *p,*q;
	q = NULL;
	p = *eventQ;
	while(p!=NULL && ( p->reqid == NULL || p->reqid->reqID != rqst->reqID)){
		q = p;
		p = p->next;
	}
	if(p != NULL){
		if(q == NULL){
			*eventQ = (*eventQ)->next;
			delete q;
		}else{
			q->next = p->next;
			delete p;
		}
	}

}



request_t* mainSimulator::generateTestRequest(){

	request_t* p = new request_t;
	p->arrtime = 1.0;
	p->nodes = 8;
	p->duration = 100.0;
	p->revenue = 0;
	p->reqID = this->idRqsts;
	this->idRqsts++;
	for(int i=0; i<p->nodes; i++){
		r = ((double)rand())/RAND_MAX;
		int sizeVM = (int)(r*(double)3) + 1;
		p->cpu[i] = sizeVM;
		p->mem[i] = 0;
		p->revenue += p->cpu[i]*this->simulationParameters->revenuePerUnitOfCPU + p->mem[i]*this->simulationParameters->revenuPerUnitOfMemory;
		p->nodesID[i] = i;
		location_t* locat = new location_t;
		locat->maxDistance = -1;
		locat->xLocation = -1;
		locat->yLocation = -1;
		p->nodeLocation[i] = locat;
	}

	//Generate some lcoation constrained nodes
	double r= ((double)rand())/RAND_MAX;
	if(r <= 0.5){
		p->nodeLocation[0]->maxDistance = 80;
		p->nodeLocation[0]->xLocation = 50;
		p->nodeLocation[0]->yLocation = 0;

		p->nodeLocation[4]->maxDistance = 80;
		p->nodeLocation[4]->xLocation = 50;
		p->nodeLocation[4]->yLocation = 100;
	}



	p->links = 0;
	link_t* l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 1;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 2;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 3;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 20;
	p->revenue += l->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 4;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 4;
	l->nodeTo = 6;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 4;
	l->nodeTo = 7;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 4;
	l->nodeTo = 5;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	return p;
}



request_t* mainSimulator::generateRandomRequest(int nbNodes, double linkProbability, double t){

	int numberOfNodes = nbNodes;

	switch(this->simulationParameters->numberOfVirtualNodesDistribution){

	case CONSTANT:
	{
		//Constant number of nodes
		numberOfNodes = this->simulationParameters->minNumberOfVirtualNodes;
	}
	break;

	case POISSON:
	{
		//Poisson number of nodes
		r = ((double)rand())/RAND_MAX;
		numberOfNodes = - this->simulationParameters->minNumberOfVirtualNodes*(log(1-r));
	}
	break;

	case UNIFORM:
	{
		//Uniform number of nodes
		r = ((double)rand())/RAND_MAX;
		numberOfNodes = this->simulationParameters->minNumberOfVirtualNodes + (int)(r*(double)(this->simulationParameters->maxNumberOfVirtualNodes - this->simulationParameters->minNumberOfVirtualNodes));
	}
	break;
	}

	request_t* p = new request_t;
	p->arrtime = 1.0;
	p->nodes = numberOfNodes;
	p->duration = 100.0;
	p->reqID = this->idRqsts;
	this->idRqsts++;
	p->links = 0;
	p->revenue = 0;


	for(int i=0; i<numberOfNodes; i++){


		//Uniform CPU Demand
		r = ((double)rand())/RAND_MAX;
		int sizeVM = (int)(r*(double)3) + 1;
		p->cpu[i] = sizeVM;
		p->mem[i] = 0;
		p->nodesID[i] = i;
		p->migrationCostOfVM[i] = 0.0001;
		p->revenue += p->cpu[i]*this->simulationParameters->revenuePerUnitOfCPU + p->mem[i]*this->simulationParameters->revenuPerUnitOfMemory;

		location_t* locat = new location_t;

		//Generate the location constraint for two nodes on average
		r = ((double)rand())/RAND_MAX;
		locat->id = i;
		if(r < this->simulationParameters->locationConstraintProbability){
			// Location genration
			location_t locationForThisRequest;
			locationForThisRequest.maxDistance = 5;
			r = ((double)rand())/RAND_MAX;
			if(r < 0.05){
				//Random location
				locat->maxDistance = 75;
				r = ((double)rand())/RAND_MAX;
				int x = (int)(r*100);
				locat->xLocation = x;
				r = ((double)rand())/RAND_MAX;
				int y = (int)(r*100);
				locat->yLocation = y;
			}else{
				r = ((double)rand())/RAND_MAX;
				int dc = (int)(r*4);
				locationForThisRequest.maxDistance = 50;
				switch(dc){
				case 0:
					locationForThisRequest.xLocation = 90;
					locationForThisRequest.yLocation = 0;
					break;
				case 1:
					locationForThisRequest.xLocation = 0;
					locationForThisRequest.yLocation = 10;
					break;
				case 2:
					locationForThisRequest.xLocation = 90;
					locationForThisRequest.yLocation = 100;
					break;
				case 3:
					locationForThisRequest.xLocation = 0;
					locationForThisRequest.yLocation = 90;
					break;
				}

				locat->xLocation = locationForThisRequest.xLocation;
				locat->yLocation = locationForThisRequest.yLocation;
				locat->maxDistance = locationForThisRequest.maxDistance;
			}

		}else{
			//No location Consraint for this node
			locat->maxDistance = -1;
			locat->xLocation = -1;
			locat->yLocation = -1;
		}
		p->nodeLocation[i] = locat;

		//Gererate the links with the neighboring nodes
		for(int j=i+1; j<numberOfNodes; j++){
			//generate the virtual links
			r = ((double)rand())/RAND_MAX;
			if(r <= linkProbability){
				link_t* l1 = new link_t;

				r = ((double)rand())/RAND_MAX;

				//Uniform bandwidth between bandwidthInf and bandwidthSup
				double bwdth = r*(this->simulationParameters->bandwidthSup - this->simulationParameters->bandwidthInf) + this->simulationParameters->bandwidthInf;

				l1->bw = bwdth;
				p->revenue += l1->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
				l1->delay = 0.5;
				l1->idRequest = p->reqID;
				l1->nodeFrom = i;
				l1->nodeTo = j;
				l1->partitionFrom = -1;
				l1->partitionTo = -1;
				//l1->numberOfMappings = 0;
				l1->mapping = NULL;
				p->link[p->links] = l1;
				p->links = p->links + 1;
			}
		}
	}

	return p;
}




request_t* mainSimulator::generateRandomRequestWithStars(int nbNodes, double linkProbability, double t){

	int numberOfNodes = nbNodes;

	switch(this->simulationParameters->numberOfVirtualNodesDistribution){

	case CONSTANT:
	{
		//Constant number of nodes
		numberOfNodes = this->simulationParameters->minNumberOfVirtualNodes;
	}
	break;

	case POISSON:
	{
		//Poisson number of nodes
		r = ((double)rand())/RAND_MAX;
		numberOfNodes = - this->simulationParameters->minNumberOfVirtualNodes*(log(1-r));
	}
	break;

	case UNIFORM:
	{
		//Uniform number of nodes
		r = ((double)rand())/RAND_MAX;
		numberOfNodes = this->simulationParameters->minNumberOfVirtualNodes + (int)(r*(double)(this->simulationParameters->maxNumberOfVirtualNodes - this->simulationParameters->minNumberOfVirtualNodes));
	}
	break;
	}

	request_t* p = new request_t;
	p->arrtime = 1.0;
	p->nodes = numberOfNodes;
	p->duration = 100.0;
	p->reqID = this->idRqsts;
	this->idRqsts++;
	p->links = 0;
	p->revenue = 0;




	///////////////////////////
	//generate the links between the nodes

	int nb_stars = numberOfNodes/6 + 1;
	int nb_nodePerStar = numberOfNodes/nb_stars;

	//Generate the stars
	int currentNode = 0;
	int lastStar = 0;
	vector<int> starNodes = vector<int>();
	for(int i = 0; i<nb_stars; i++){
		int starCenter = currentNode;
		lastStar = currentNode;
		starNodes.push_back(currentNode);
		currentNode++;
		for(int j=1; j<nb_nodePerStar; j++){
			//Generate the links between this node and the star center
			link_t* l1 = new link_t;

			r = ((double)rand())/RAND_MAX;

			//Uniform bandwidth between bandwidthInf and bandwidthSup
			double bwdth = r*(this->simulationParameters->bandwidthSup - this->simulationParameters->bandwidthInf) + this->simulationParameters->bandwidthInf;

			l1->bw = bwdth;
			p->revenue += l1->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
			l1->delay = 0.5;
			l1->idRequest = p->reqID;
			l1->nodeFrom = starCenter;
			l1->nodeTo = currentNode;
			l1->partitionFrom = -1;
			l1->partitionTo = -1;
			//l1->numberOfMappings = 0;
			l1->mapping = NULL;
			p->link[p->links] = l1;
			p->links = p->links + 1;

			currentNode++;
		}
	}
	//Add the remaining nodes to the topology
	while(currentNode < numberOfNodes){

		//Generate the links between this node and the star center
		link_t* l1 = new link_t;

		r = ((double)rand())/RAND_MAX;

		//Uniform bandwidth between bandwidthInf and bandwidthSup
		double bwdth = r*(this->simulationParameters->bandwidthSup - this->simulationParameters->bandwidthInf) + this->simulationParameters->bandwidthInf;

		l1->bw = bwdth;
		p->revenue += l1->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
		l1->delay = 0.5;
		l1->idRequest = p->reqID;
		l1->nodeFrom = lastStar;
		l1->nodeTo = currentNode;
		l1->partitionFrom = -1;
		l1->partitionTo = -1;
		//l1->numberOfMappings = 0;
		l1->mapping = NULL;
		p->link[p->links] = l1;
		p->links = p->links + 1;

		currentNode++;
	}

	//Generate the links betwene the stars
	for(unsigned int i =0; i<starNodes.size(); i++){
		for(unsigned int j = i+1; j<starNodes.size(); j++){
			if(i != j){
				r = ((double)rand())/RAND_MAX;
				if(r <= linkProbability){
					link_t* l1 = new link_t;
					r = ((double)rand())/RAND_MAX;
					//Uniform bandwidth between bandwidthInf and bandwidthSup
					double bwdth = r*(this->simulationParameters->bandwidthSup - this->simulationParameters->bandwidthInf) + this->simulationParameters->bandwidthInf;
					l1->bw = bwdth;
					p->revenue += l1->bw*this->simulationParameters->revenuPerUnitOfBandwidth;
					l1->delay = 0.5;
					l1->idRequest = p->reqID;
					l1->nodeFrom = starNodes.at(i);
					l1->nodeTo = starNodes.at(j);
					l1->partitionFrom = -1;
					l1->partitionTo = -1;
					//l1->numberOfMappings = 0;
					l1->mapping = NULL;
					p->link[p->links] = l1;
					p->links = p->links + 1;
				}
			}
		}
	}



	///////////////////////////////
	// Generate the nodes


	for(int i=0; i<numberOfNodes; i++){

		//Fixed CPU Demand

		r = ((double)rand())/RAND_MAX;
		int sizeVM = (int)(r*(double)3) + 1;
		p->cpu[i] = sizeVM;
		p->mem[i] = 0;
		p->nodesID[i] = i;
		p->revenue += p->cpu[i]*this->simulationParameters->revenuePerUnitOfCPU + p->mem[i]*this->simulationParameters->revenuPerUnitOfMemory;

		location_t* locat = new location_t;
		//No location Consraint for this node
		locat->maxDistance = -1;
		locat->xLocation = -1;
		locat->yLocation = -1;
		locat->id = i;
		p->nodeLocation[i] = locat;
		p->migrationCostOfVM[i] = 0.0001;

	}

	int nbUsedStarNodes = 0;
	vector<int> constranedSoFar = vector<int>();

	for(int i=0; i<numberOfNodes; i++){
		//Generate the location constraint
		r = ((double)rand())/RAND_MAX;

		if(r < this->simulationParameters->locationConstraintProbability){
			int node = -1;

			if(nbUsedStarNodes < (int)starNodes.size()){
				//Choose the center of the star as a constrained node
				int indStarNode = (((double)rand())/RAND_MAX)*((int)starNodes.size());
				bool done = false;

				while(!done){
					bool found = false;
					done = true;
					for(unsigned int z=0; z<constranedSoFar.size(); z++){
						if(constranedSoFar.at(z) == starNodes.at(indStarNode)){
							found = true;
							break;
						}
						if(found){
							done = false;
							indStarNode = (((double)rand())/RAND_MAX)*((int)starNodes.size());
						}else{
							done = true;
						}
					}
				}

				node = starNodes.at(indStarNode);
				constranedSoFar.push_back(node);
				nbUsedStarNodes++;
			}else{
				//Choose another node as a center
				node = ((double)rand())/RAND_MAX*numberOfNodes;
				bool done = false;

				while(!done){
					bool found = false;
					done = true;
					for(unsigned int z=0; z<constranedSoFar.size(); z++){
						if(constranedSoFar.at(z) == node){
							found = true;
							break;
						}
					}
					if(found){
						done = false;
						node = ((double)rand())/RAND_MAX*numberOfNodes;
					}else{
						done = true;
					}
				}

				constranedSoFar.push_back(node);
			}

			location_t* locat = p->nodeLocation[node];
			r = ((double)rand())/RAND_MAX;
			if(r < 0.0){
				//Random location
				locat->maxDistance = 75;
				r = ((double)rand())/RAND_MAX;
				int x = (int)(r*100);
				locat->xLocation = x;
				r = ((double)rand())/RAND_MAX;
				int y = (int)(r*100);
				locat->yLocation = y;
			}else{
				r = ((double)rand())/RAND_MAX;
				int dc = (int)(r*4);
				locat->maxDistance = 50;
				switch(dc){
				case 0:
					locat->xLocation = 90;
					locat->yLocation = 0;
					break;
				case 1:
					locat->xLocation = 0;
					locat->yLocation = 10;
					break;
				case 2:
					locat->xLocation = 90;
					locat->yLocation = 100;
					break;
				case 3:
					locat->xLocation = 0;
					locat->yLocation = 90;
					break;
				}

			}//if(r < 0.3){

		}//End If Location probability
	}
	return p;
}






