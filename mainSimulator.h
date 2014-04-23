/*
 * mainSimulator.h
 *
 *  Created on: Dec 28, 2012
 *      Author: ahmedamokrane
 */

#ifndef MAINSIMULATOR_H_
#define MAINSIMULATOR_H_


#include "Embed.h"
#include "DataCenter.h"
#include "CentralController.h"
#include "SubstrateInfrastructure.h"
#include "VDCRequestPartitioned.h"
#include "TopologyGenerator.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>

using namespace std;

using std::vector;


/*
 *
 */
class mainSimulator {
public:

	// For the VDc Requests
	simulation_parameters_t* simulationParameters;
	central_controller_parameters_t* centralControllerParmaeters;
	CentralController* cn;

	//Variables to help setting the things
	int idRqsts;
	double r;//For the random numbers

	unsigned int seedInitialValue;


	mainSimulator();
	virtual ~mainSimulator();

	//static int main (int argc, char *argv[]);

	request_t* generateNextRequest(double t);

	request_t* generateRandomRequest(int numberOfNodes, double linkProbability, double t);
	request_t* generateRandomRequestWithStars(int nbNodes, double linkProbability, double t);

	VDCRequestPartitioned* partitionRequest(request_t* initialRequest, CentralController* cn, int partitioingMethod);


	void insertEventInQueue(event_t** eventQ, event_t* evnt);

	void removeLeaveEventOfRequestInQueue(event_t** eventQ, request_t* rqst);

	void updateInstantaneousCostOfEmbededRequests(double t);

	request_t* generateTestRequest();

	infrastructure_utilization_t* startSimulation();

	infrastructure_utilization_t* startSimulationFromListOfEvents(event_t* listEvents);

	event_t** generateListEventsForSimulation();


};


#endif /* MAINSIMULATOR_H_ */
