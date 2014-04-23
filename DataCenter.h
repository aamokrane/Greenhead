/*
 * DataCenter.h
 *
 *  Created on: Dec 22, 2012
 *      Author: ahmedamokrane
 */

#ifndef DATACENTER_H_
#define DATACENTER_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Embed.h"
#include "DataCenterClient.h"
#include "RemoteCalls.h"
#include "staticUsefulMethods.h"
//#include "CentralController.h"

using std::vector;

class DataCenter {

public:
	//The datacenter propoerties
	//double PUE;
	int id;

	location_t* location;

	vector<datacenter_parameter_t*>* pueOverTime;
	vector<datacenter_parameter_t*>* electrictyPriceOverTime;	//The electricity Price ($ per Kwh)
	vector<datacenter_parameter_t*>* carbonFootPrintCoefficientOverTime; //The coefficient that gives the amount of Carbon per unit of power (ton per KWh)
	vector<datacenter_parameter_t*>* carbonFootPrintUnitCostOverTime;	//The cost per ton of Carbon ($ per ton)
	vector<datacenter_parameter_t*>* amountOfRenewablesOverTime;


	//The embedded partitions and links at this time
	vector<partition_t*>* embeddedPartitions;
	vector<link_t*>* embeddedLinks;
	datacenter_utilization_t* actualUtilization;


	//Corresponding remote Data Center
	DataCenterClient* remoteDataCenterClient;
	int dataCenterImplementation; //SIMULATION or REAL_DEPLOYMENT

	//double usedPower;

	//Put the physical topology at this level

	int nbIntervalsPerDay;

	double squareMeterSolarPanels; //The total suare pabels in possession of this datacenter

	double maxPowerCapacity;

	double rejectProbability;

	double desiredOperatingTemperature; //In Fehrenheit (78F for 25 C, 90F for 30)

	double fanMaxPower;

	double fractionOfPowerForServers; //The fraction of power that goes to the servers (compared to the power for the network)

	//The methods

	DataCenter();
	virtual ~DataCenter();


	double embedPartition(partition_t* partition, double t);
	/* Embeds the the partition "partition" of the original VDC request "rqst" to embed, see the format a VDC request
	 * Returns the cost of embedding
	 * The t parameters is the instant at which the partition to embed is sent
	 * */

	double getPowerForPartition(partition_t* partition);

	bool isPartitionCurrentlyEmbeddedInDataceter(partition_t* partition);


	double getCostEnergyForPartition(partition_t* partition, double t);
	/* Returns the cost of embedding the partition "partition" of the original VDC request "rqst" to embed, see the format a VDC request */

	double getEstimationBrownPowerNeededForPartitionForATimePeriod(partition_t* partition, double tBegin, double tEnd);


	//double embedLink(request_t* rqst, partition_t* sourcePartition, partition_t* destinationPartition, int nodeIDSource, int nodeIDDestination, double bandwidth, double delay, double t);
	double embedLink(link_t* link, double t);

	/*
	 * This method embeds a link between two nodes, the source node belong to sourcePartition, and the destination node belongs to destinationPartition
	 * Both partitions belong to the same VDC request rqst
	 * Returns the energy cost if success and -1 if the link was not embedded
	 * t is the instant at which the request for link embedding is sent
	 */

	//double removeLink(request_t* rqst, partition_t* sourcePartition, partition_t* destinationPartition, int nodeIDSource, int nodeIDDestination, double bw, double t);
	double removeLink(link_t* link, double t);


	double getPUE(double t); //Return the PUE at time t

	double getElectrictyPrice(double t); //Return the PUE at time t

	double getCarbonFootPrintCoefficient(double t); //Return the Carbon foot print per unit of power at time t

	double getCarbonFootPrintUnitCost(double t); //Return the Cost per carbon unit at time t

	double getAmountOfRenewables(double t); //Return the amount of renewables at time t

	double getRemainingRenewables(double t); //Return the amount of renewables at time t

	double getAmountOfCarbonTotal(double t); //Return the amount of carbon footprint at time t

	double getAmountOfCarbonOfServers(double t);  //Return the amount of carbon footprint at time t related to the servers only
	double getAmountOfCarbonOfNetwork(double t);  //Return the amount of carbon footprint at time t related to the network

	double getCostPerUnitOfPowerAndTime(double t); //Returns the cost per unit of power (assuming every server requires units of power)

	datacenter_utilization_t* getInstantneousUtilization(double t);

	double removePartition(partition_t* partition, double t);
	/*
	 * Remove the partition from this data center
	 * Return the total cost from the beginning of the embedding in this DC (Optional, we can retrieve it differently)
	 */

	//double getCarbonFootprintPerUnitOfVM2(double t); //Returns the carbon emission per unit of VM at time t (which corresponds to the time slot of t)

	//double getCarbonFootprintPerUnitOfBandwidth2(double t); //Returns the carbon emission per unit of Bandwidth in the data center at time t (which corresponds to the time slot of t)

	double getEstimationOfAmountOfRenewables(double tBegin, double tEnd); //Return the estimation of the amount of available renewables between tBegin and tEnd

	double getEstimationOfAverageElectricityPrice(double tBegin, double tEnd); //Return the estimation of the eletricity price between tBegin and tEnd

	double getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(double currentTime, double tEnd);//Returns the total Energy consumption of this data center given that some request are going to leave from the present time to tEnd

	double getEstimationOfAverageCPUUsedDuringUpcomingTimePeriod(double currentTime, double tEnd);

	//double getEstimationOfAverageAmountOfCarbonDuringUpcomingTimePeriod(double currentTime, double tEnd);//Returns the total Carbon emission of this data center given that some request are going to leave from the present time to tEnd

};



#endif /* DATACENTER_H_ */
