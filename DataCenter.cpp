/*
 * DataCenter.cpp
 *
 *  Created on: Dec 22, 2012
 *      Author: ahmedamokrane
 */

#include "DataCenter.h"


DataCenter::DataCenter() {
	embeddedPartitions = new vector<partition_t*>();
	embeddedLinks = new vector<link_t*>();
	actualUtilization = new datacenter_utilization_t;
	actualUtilization->tBegin = 0.0;
	actualUtilization->totalUsedBandwidth = 0;
	actualUtilization->numberOfVMs = 0;
	actualUtilization->totalUsedCPU = 0;
	actualUtilization->usedPowerIT = 0;
	actualUtilization->costPerUnitOfPower = 0.0;

	amountOfRenewablesOverTime = NULL;
	carbonFootPrintUnitCostOverTime = NULL;
	carbonFootPrintCoefficientOverTime = NULL;
	electrictyPriceOverTime = NULL;
	pueOverTime = NULL;

	this->nbIntervalsPerDay = 3;
	this->maxPowerCapacity = 20000000;
	this->squareMeterSolarPanels = 0;
	id = rand();
	location = NULL;
	this->rejectProbability = 0.15;

	this->fanMaxPower = 2;

	this->desiredOperatingTemperature = 80; //In Fehrenheit

	this->fractionOfPowerForServers = 0.75; //According to [Greenberg2009, The Cost of a Cloud:Research Problems in Data Center Networks], 3/4 goes to the servers

	this->dataCenterImplementation = SIMULATED; //By default we use simulated data centers

	this->remoteDataCenterClient = NULL;

}

DataCenter::~DataCenter() {
	this->embeddedPartitions->clear();
}


double DataCenter::embedPartition(partition_t* partition, double t){
	/*
	 * This methods embeds a VDC request in the datacenter and returns the corresponding cost of embedding (mainly energy cost)
	 * The returned result is the same as the getCostEnergyForPartition method
	 * The t parameters is the instant at which the partition to embed is sent
	 */

	double cost = -1;

	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		partition_savi_remote_call_t partitionToRemote;
		staticUsefulMethods::getSerializablePartition(partition, &partitionToRemote);
		remoteCallParam->method = EMBED_PARTITION;
		remoteCallParam->parameter.partition = partitionToRemote;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		if(resultCall.result >= 0){
			*(this->actualUtilization) = resultCall.parameter.dataCenterUtilization;
		}
		cost = resultCall.result;
		//delete remoteCallParam1;
	}break;

	case SIMULATED:{
		//The cost is given in watts
		cost = partition->nodes * POWER_PER_SERVER_IN_DATACENTERS; //Assume that we have to turn on always a new server
		double newUse = (actualUtilization->usedPowerIT + cost)*this->getPUE(t);

		if(newUse > this->maxPowerCapacity){
			//We are exceeding the ca�city of the datacenter, reject
			return FAILURE_PARTITION_EMBED;
		}
		//Update the utilization in this datacenter
		actualUtilization->numberOfVMs += partition->nodes;
		actualUtilization->tBegin = t;

		for(int i=0; i<partition->nodes; i++){
			actualUtilization->totalUsedCPU += partition->cpu[i];
		}

		for(int i=0; i<partition->links; i++){
			actualUtilization->totalUsedBandwidth += partition->link[i]->bw;
		}

		double available = this->getAmountOfRenewables(t) - (actualUtilization->usedPowerIT*this->getPUE(t));
		actualUtilization->usedPowerIT += cost;
		//Compute the cost
		if(available > cost){
			return 0;
		}else{//Not enough renewables
			if(available > 0){
				return cost - available;
			}else{
				return cost;
			}
		}
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center


	}break;

	default:{

	}

	}//End Switch

	//Sort the partitions according to their lifetime
	//The long lived partitions will be first
	//Sort the partitions to migrate, migrate long lived VDCs first

	unsigned int z=0;
	bool found = false;
	while(z<embeddedPartitions->size() && !found){

		if(embeddedPartitions->at(z)->request->arrtime + embeddedPartitions->at(z)->request->duration > partition->request->arrtime+partition->request->duration){
			found = true;
		}else{
			z++;
		}
	}
	//Insert the partition at position z
	embeddedPartitions->insert(embeddedPartitions->begin()+z,partition);
	if(partition->mapping != NULL){
		partition->mapping->datacenterID = this->id;
	}
	return cost;
}



double DataCenter::getRemainingRenewables(double t) {
	double rem = this->getAmountOfRenewables(t) - (actualUtilization->usedPowerIT*this->getPUE(t));
	if(rem < 0){
		return 0;
	}else{
		return rem;
	}
}

double DataCenter::getCostPerUnitOfPowerAndTime(double t) {
	/*
	 * //Returns the cost per unit of power (assuming every server requires units of power)
	 */
	if(this->getRemainingRenewables(t) == 0){
		//Everything is in KWatt
		double cost = this->getPUE(t)*(this->getElectrictyPrice(t) + (this->getCarbonFootPrintCoefficient(t)*this->getCarbonFootPrintUnitCost(t)));
		return cost;
	}else{
		//The renewables are assumed to be free
		return 0;
	}
}

double DataCenter::removePartition(partition_t* partition, double t){
	/*
	 * Remove the partition from this datacenter
	 * Return the total cost from the beginning of the embedding in this DC (Optional, we can retreive it differently)
	 */

	double cost = -1;


	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		partition_savi_remote_call_t partitionToRemote;
		staticUsefulMethods::getSerializablePartition(partition, &partitionToRemote);
		remoteCallParam->method = REMOVE_PARTITION;
		remoteCallParam->parameter.partition = partitionToRemote;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		if(resultCall.result >= 0){
			*(this->actualUtilization) = resultCall.parameter.dataCenterUtilization;
		}
		cost = resultCall.result;
		//delete remoteCallParam1;

	}break;

	case SIMULATED:{
		//Update the utilization in this datacenter
		actualUtilization->numberOfVMs -= partition->nodes;
		actualUtilization->tBegin = t;
		//Remove the CPU utilization to the servers
		for(int i=0; i<partition->nodes; i++){
			actualUtilization->totalUsedCPU -= partition->cpu[i];
		}

		for(int i=0; i<partition->links; i++){
			actualUtilization->totalUsedBandwidth -= partition->link[i]->bw;
		}

		//
		cost = partition->nodes * POWER_PER_SERVER_IN_DATACENTERS; //Assume that we have to turn on always a new server
		actualUtilization->usedPowerIT -= cost;
		actualUtilization->remainingRenewables = this->getRemainingRenewables(t);
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}


	}//End Switch

	//The cost is given in watts
	for(unsigned int i=0; i<embeddedPartitions->size(); i++){
		if(this->embeddedPartitions->at(i)->partitionID == partition->partitionID && this->embeddedPartitions->at(i)->reqID == partition->reqID){
			this->embeddedPartitions->erase(this->embeddedPartitions->begin()+i);
			break;
		}
	}

	if(partition->mapping != NULL){
		partition->mapping->datacenterID = -1;
	}

	return cost;
}


double DataCenter::getCostEnergyForPartition(partition_t* partition, double t){
	/*
	 * This method returns the cost of embedding a request without doing the embedding
	 * It takes as a parameter the VDC request (VMs + Links)
	 * The result in terms of energy is given for an on/off model of servers as the number of additional servers
	 * It is also difference between the power consumer if we embed this VDC - the energy consumed if we don't embed
	 * It is the additional amount of energy needed in this datacenter to accommodate the request
	 */

	//The cost is given in watt

	double cost = -1;
	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		partition_savi_remote_call_t partitionToRemote;
		staticUsefulMethods::getSerializablePartition(partition, &partitionToRemote);
		remoteCallParam->method = GET_COST_FOR_PARTITION;
		remoteCallParam->parameter.partition = partitionToRemote;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		if(resultCall.result >= 0){
			*(this->actualUtilization) = resultCall.parameter.dataCenterUtilization;
		}
		cost = resultCall.result;
		//delete remoteCallParam1;

	}break;

	case SIMULATED:{
		//This is a simulated data center
		double cost = partition->nodes * POWER_PER_SERVER_IN_DATACENTERS; //Assume that we have to turn on always a new server
		double newUse = (actualUtilization->usedPowerIT + cost)*this->getPUE(t);

		if(newUse*this->getPUE(t) > this->maxPowerCapacity){
			//We are exceeding the capacity of the datacenter, reject
			return FAILURE_PARTITION_EMBED;
		}


		//Reject with a probability

		double available = this->getAmountOfRenewables(t) - (actualUtilization->usedPowerIT*this->getPUE(t));
		//Compute the cost
		if(available > cost){
			return 0;
		}else{//Not enough renewables
			if(available > 0){
				return cost - available;
			}else{
				return cost;
			}
		}

	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch

	return cost;
}



double DataCenter::getEstimationBrownPowerNeededForPartitionForATimePeriod(partition_t* partition, double tBegin, double tEnd){
	/*
	 * This method returns the brown power (power from the grid) estimation for this partition for the upcoming perdion of time
	 * The perdio starts at tBegin and ends at tEnd
	 */

	//The cost is given in watt
	//double powerForPartition = partition->nodes * POWER_PER_SERVER_IN_DATACENTERS; //Assume that we have to turn on always a new server
	double powerForPartition = this->getPowerForPartition(partition);

	//Reject with a probability
	//double available = this->getEstimationOfAmountOfRenewables(tBegin, tEnd)/(tEnd - tBegin) - (actualUtilization->usedPowerIT*this->getPUE(tBegin));
	double available = this->getEstimationOfAmountOfRenewables(tBegin, tEnd)/(tEnd - tBegin) - this->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(tBegin, tEnd)*this->getPUE(tBegin);
	//Compute the cost
	if(available < 0){
		//Amount of brown power, we pick the part of the partition in the overall power consumption of the data center
		double result = (powerForPartition/this->getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(tBegin, tEnd)*this->getPUE(tBegin))*(-available);
		return result;
	}else{//Not enough renewables
		return 0;
	}
}



double DataCenter::embedLink(link_t* link, double t){

	/*
	 * Remove the link as opposed to embedLink
	 */

	double cost = -1;

	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = EMBED_LINK;
		link_savi_remote_call_t l;
		staticUsefulMethods::getSerializableLink(link, &l);
		remoteCallParam->parameter.link = l;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		if(resultCall.result >= 0){
			*(this->actualUtilization) = resultCall.parameter.dataCenterUtilization;
		}
		cost = resultCall.result;
		//delete remoteCallParam1;

	}break;

	case SIMULATED:{
		//This is a simulated data center
		this->actualUtilization->totalUsedBandwidth += link->bw;
		if(link->mapping != NULL){
			link->mapping->DCId = this->id;
		}
		cost = 0.0;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch

	this->embeddedLinks->push_back(link);

	return 0.0;
}


double DataCenter::removeLink(link_t* link, double t){

	/*
	 * Remove the link as opposed to embedLink
	 */

	double cost = 0.0;


	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = REMOVE_LINK;
		link_savi_remote_call_t l;
		staticUsefulMethods::getSerializableLink(link, &l);
		remoteCallParam->parameter.link = l;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		if(resultCall.result >= 0){
			*(this->actualUtilization) = resultCall.parameter.dataCenterUtilization;
		}
		cost = resultCall.result;
		//delete remoteCallParam1;


	}break;

	case SIMULATED:{
		this->actualUtilization->totalUsedBandwidth -= link->bw;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch

	if(link->mapping != NULL){
		link->mapping->DCId = -1;
	}
	//Remove the link from the list of embedded links
	for(unsigned int i=0; i<this->embeddedLinks->size(); i++){
		link_t* linkInList = this->embeddedLinks->at(i);
		if(linkInList->idRequest == link->idRequest && linkInList->nodeFrom == link->nodeTo){
			this->actualUtilization->totalUsedBandwidth -= link->bw;
			this->embeddedLinks->erase(this->embeddedLinks->begin()+i);
			break;
		}
	}

	return 0.0;
}

double DataCenter::getPUE(double t){
	//Return the PUE at time t
	double res;
	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_PUE;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		res = resultCall.result;

	}break;

	case SIMULATED:{
		double COP = 0.0068*(this->desiredOperatingTemperature*this->desiredOperatingTemperature) + 0.0008*this->desiredOperatingTemperature + 0.458;
		//double coolingPower = this->fanMaxPower + this->actualUtilization->usedPowerIT;
		double totalPower = this->actualUtilization->usedPowerIT + this->fanMaxPower+ this->actualUtilization->usedPowerIT/COP;

		if(this->actualUtilization->usedPowerIT > 0){
			res = totalPower/this->actualUtilization->usedPowerIT;
		}else{
			res = 1;
		}
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch

	return res;
}

double DataCenter::getElectrictyPrice(double t) {

	double res;

	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_ELECTRICITY_PRICE;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		res = resultCall.result;

	}break;

	case SIMULATED:{
		div_t divresult = div(t, 24); //24 hours per day
		double tInDay = divresult.rem;
		int interval = 24/this->nbIntervalsPerDay;
		int index = (int) div(tInDay,interval).quot; //get the index of the day;

		res = this->electrictyPriceOverTime->at(index)->value;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch




	return res;
}

double DataCenter::getCarbonFootPrintCoefficient(double t) {

	double res;
	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_CARBON_PER_UNIT_OF_POWER;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		res = resultCall.result;

	}break;

	case SIMULATED:{
		div_t divresult = div(t, 24); //24 hours per day
		double tInDay = divresult.rem;
		int interval = 24/this->nbIntervalsPerDay;
		int index = (int) div(tInDay,interval).quot; //get the index of the day;
		res = this->carbonFootPrintCoefficientOverTime->at(index)->value;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch

	return res;
}

double DataCenter::getCarbonFootPrintUnitCost(double t) {
	double res;
	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{

		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_CARBON_COST;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		res = resultCall.result;

	}break;

		case SIMULATED:{
			div_t divresult = div(t, 24); //24 hours per day
				double tInDay = divresult.rem;
				int interval = 24/this->nbIntervalsPerDay;
				int index = (int) div(tInDay,interval).quot; //get the index of the day;
				res = this->carbonFootPrintUnitCostOverTime->at(index)->value;
		}break;

		case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
			//TODO: AIMAL: Code goes here
			//This is a simulated data center

		}break;

		default:{

		}

	}//End Switch





	return res;
}

double DataCenter::getAmountOfRenewables(double t) {
	double res;
	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{

		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_AVAILABLE_RENEWABLES;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		res = resultCall.result;



	}break;

	case SIMULATED:{
		div_t divresult = div(t, 24); //24 hours per day
		double tInDay = divresult.rem;
		int interval = 24/this->nbIntervalsPerDay;
		int index = (int) div(tInDay,interval).quot; //get the index of the day;

		res = this->amountOfRenewablesOverTime->at(index)->value;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch


	return res;
}

double DataCenter::getAmountOfCarbonTotal(double t){
	double totalCarbon;
	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_TOTAL_CARBON_EMISSION_RATE;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		totalCarbon = resultCall.result;

	}break;

	case SIMULATED:{
		double available = this->getAmountOfRenewables(t) - (this->actualUtilization->usedPowerIT*this->getPUE(t));
		double totalCarbon = 0;
		if(available <= 0){
			totalCarbon = (this->actualUtilization->usedPowerIT*this->getPUE(t) -this->getAmountOfRenewables(t))*this->getCarbonFootPrintCoefficient(t)/1000; //The powerIT is given in Watts and the carbon coefficient is in Tons/KWh
		}

	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch

	return totalCarbon;

}

double DataCenter::getAmountOfCarbonOfServers(double t){
	double carbonForVMs;
	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_CARBON_EMISSION_RATE_SERVERS;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		carbonForVMs = resultCall.result;

	}break;

	case SIMULATED:{
		double available = this->getAmountOfRenewables(t) - (this->actualUtilization->usedPowerIT*this->getPUE(t));
		double totalCarbon = 0;
		if(available <= 0){
			totalCarbon = (this->actualUtilization->usedPowerIT*this->actualUtilization->PUE -this->getAmountOfRenewables(t))*this->getCarbonFootPrintCoefficient(t)/1000; //The powerIT is given in Watts and the carbon coefficient is in Tons/KWh
		}

		carbonForVMs = totalCarbon*this->fractionOfPowerForServers;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch

	return carbonForVMs;
}

double DataCenter::getAmountOfCarbonOfNetwork(double t){
	double carbonForNetwork;

	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_CARBON_EMISSION_RATE_NETWORK;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		carbonForNetwork = resultCall.result;

	}break;

	case SIMULATED:{
		double available = this->getAmountOfRenewables(t) - (this->actualUtilization->usedPowerIT*this->getPUE(t));
		double totalCarbon = 0;
		if(available <= 0){
			totalCarbon = (this->actualUtilization->usedPowerIT*this->getPUE(t) - this->getAmountOfRenewables(t))*this->getCarbonFootPrintCoefficient(t)/1000; //The powerIT is given in Watts and the carbon coefficient is in Tons/KWh
		}

		double carbonForNetwork =  totalCarbon*(1 - this->fractionOfPowerForServers);
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch


	return carbonForNetwork;
}

datacenter_utilization_t* DataCenter::getInstantneousUtilization(double t) {


	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_INSTANTANEOUS_UTILIZATION;
		remoteCallParam->tBegin = t;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		*(this->actualUtilization) =  resultCall.parameter.dataCenterUtilization;

	}break;

	case SIMULATED:{
		//This is a simulated Data Center
		this->actualUtilization->PUE = this->getPUE(t);
		this->actualUtilization->electricityPrice = this->getElectrictyPrice(t);
		this->actualUtilization->availableRenwables = this->getAmountOfRenewables(t);
		this->actualUtilization->carbonCoefficient = this->getCarbonFootPrintCoefficient(t);
		this->actualUtilization->carbonCost = this->getCarbonFootPrintUnitCost(t);
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch


	//Build the structure to return
	datacenter_utilization_t* strct = new datacenter_utilization_t;
	strct->numberOfVMs = this->actualUtilization->numberOfVMs;
	strct->totalUsedBandwidth = this->actualUtilization->totalUsedBandwidth;
	strct->totalUsedCPU = this->actualUtilization->totalUsedCPU;
	strct->availableRenwables = this->actualUtilization->availableRenwables;
	strct->PUE = this->actualUtilization->PUE;
	strct->carbonCoefficient = this->actualUtilization->carbonCoefficient;
	strct->carbonCost = this->actualUtilization->carbonCost;
	strct->usedPowerIT = this->actualUtilization->usedPowerIT;
	strct->electricityPrice = this->actualUtilization->electricityPrice;
	double available = strct->availableRenwables - (this->actualUtilization->usedPowerIT*this->actualUtilization->PUE);
	if(available <= 0){
		strct->costPerUnitOfPower = this->actualUtilization->PUE*(this->actualUtilization->electricityPrice + this->actualUtilization->carbonCoefficient*this->actualUtilization->carbonCost); //Cost per kWh

		if(strct->PUE>0){
			strct->instantaneousCost = ((strct->usedPowerIT*this->actualUtilization->PUE - strct->availableRenwables)/this->actualUtilization->PUE)*(strct->costPerUnitOfPower/1000); //Switch from Wh to kWh
		}else{
			strct->instantaneousCost = 0;
		}
		strct->amountOfCarbon = (strct->usedPowerIT*this->actualUtilization->PUE - strct->availableRenwables)*strct->carbonCoefficient / 1000; //The powerIT is given in Watts and the carbon coefficient is in Tons/KWh
		this->actualUtilization->remainingRenewables = 0;
	}else{
		strct->costPerUnitOfPower = 0;
		strct->instantaneousCost = 0;
		strct->amountOfCarbon = 0;
		this->actualUtilization->remainingRenewables = available;
	}

	strct->tBegin = t;

	strct->carbonEmissionPerUnitOfVM = 0;
	if(strct->totalUsedCPU > 0){
		strct->carbonEmissionPerUnitOfVM = (strct->amountOfCarbon * this->fractionOfPowerForServers)/strct->totalUsedCPU;
	}

	strct->carbonEmissionPerUnitOfBandwidth = 0;
	if(strct->totalUsedBandwidth > 0){
		strct->carbonEmissionPerUnitOfBandwidth = (strct->amountOfCarbon * (1 - this->fractionOfPowerForServers))/strct->totalUsedCPU;
	}

	return strct;
}


double DataCenter::getEstimationOfAmountOfRenewables(double tBegin, double tEnd) {

	double result = 0;


	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		remoteCallParam->method = GET_ESTIMATION_AVAILABLE_RENEWABLE_POWER_FOR_PERIOD;
		remoteCallParam->tBegin = tBegin;
		remoteCallParam->parameter.tEnd;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		result = resultCall.result;

	}break;

	case SIMULATED:{
		tEnd = tEnd - 0.001;
		if(tBegin>tEnd){
			return 0;
		}
		div_t divresult = div(tBegin, 24); //24 hours per day
		double tBeginInDay = divresult.rem;
		//double timeFirstInterval = tBegin - (tBeginInDay + 24*divresult.quot);
		int interval = 24/this->nbIntervalsPerDay;
		int indexBegin = (int) div(tBeginInDay,interval).quot;

		divresult = div(tEnd, 24); //24 hours per day
		double tEndInDay = divresult.rem;
		//double timeLastInterval = (tEnd-1) - (tEndInDay + 24*divresult.quot);
		int indexEnd = (int) div(tEndInDay,interval).quot;

		int i=indexBegin;
		while(i != indexEnd){
			result += this->amountOfRenewablesOverTime->at(i)->value;
			i++;
			if(i == 24){
				i=0;
			}
		}
		result += this->amountOfRenewablesOverTime->at(indexEnd)->value;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch


	return result;

}

double DataCenter::getPowerForPartition(partition_t* partition) {

	//Returns the power needed to embed the partition partition

	double cost;

	switch(this->dataCenterImplementation){

	case STUB_FOR_REMOTE:{
		//Call the remote method to embed the parition
		remote_call_t* remoteCallParam = new remote_call_t;
		partition_savi_remote_call_t partitionToRemote;
		staticUsefulMethods::getSerializablePartition(partition, &partitionToRemote);
		remoteCallParam->method = GET_POWER_FOR_PARTITION;
		remoteCallParam->parameter.partition = partitionToRemote;
		remote_call_t resultCall;
		this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
		cost = resultCall.result;
		//delete remoteCallParam1;

	}break;

	case SIMULATED:{
		//This a simulated data center
		cost = partition->nodes * POWER_PER_SERVER_IN_DATACENTERS;
	}break;

	case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
		//TODO: AIMAL: Code goes here
		//This is a simulated data center

	}break;

	default:{

	}

	}//End Switch


	return cost;
}


bool DataCenter::isPartitionCurrentlyEmbeddedInDataceter(partition_t* partition){
	for(unsigned int i=0; i<embeddedPartitions->size(); i++){
		if(this->embeddedPartitions->at(i)->partitionID == partition->partitionID && this->embeddedPartitions->at(i)->reqID == partition->reqID){
			return true;
		}
	}
	return false;
}

double DataCenter::getEstimationOfAverageElectricityPrice(double tBegin, double tEnd) {
	double result = 0;



	switch(this->dataCenterImplementation){

		case STUB_FOR_REMOTE:{
			//Call the remote method to embed the parition
					remote_call_t* remoteCallParam = new remote_call_t;
					remoteCallParam->method = GET_ESTIMATION_AVERAGE_ELECTRICITY_PRICE_FOR_PERIOD;
					remoteCallParam->tBegin = tBegin;
					remoteCallParam->parameter.tEnd;
					remote_call_t resultCall;
					this->remoteDataCenterClient->callRemoteMethod(remoteCallParam, &resultCall);
					result = resultCall.result;

		}break;

		case SIMULATED:{
			tEnd = tEnd - 0.001;
				if(tBegin>tEnd){
					return 0;
				}
				div_t divresult = div(tBegin, 24); //24 hours per day
				double tBeginInDay = divresult.rem;
				//double timeFirstInterval = tBegin - (tBeginInDay + 24*divresult.quot);
				int interval = 24/this->nbIntervalsPerDay;
				int indexBegin = (int) div(tBeginInDay,interval).quot;

				divresult = div(tEnd, 24); //24 hours per day
				double tEndInDay = divresult.rem;
				//double timeLastInterval = (tEnd-1) - (tEndInDay + 24*divresult.quot);
				int indexEnd = (int) div(tEndInDay,interval).quot;
				int i=indexBegin;
				int counter=0;
				while(i != indexEnd){
					result += this->electrictyPriceOverTime->at(i)->value;
					counter ++;
					i++;
					if(i == 24){
						i=0;
					}
				}
				result += this->electrictyPriceOverTime->at(indexEnd)->value;
				counter ++;
				result = result/((double)(counter));
		}break;

		case DEPLOYED_WITH_VDCPLANNER_IN_SAVI:{
			//TODO: AIMAL: Code goes here
			//This is a simulated data center

		}break;

		default:{

		}

	}//End Switch

	return result;
}




double DataCenter::getEstimationOfAverageCPUUsedDuringUpcomingTimePeriod(double currentTime, double tEnd){
	//Returns the total Energy consumption of this data center given that some request are going to leave from the present time to tEnd
	double CPUToCome = 0;
	for(unsigned int i=0; i<this->embeddedPartitions->size(); i++){
		//get the total CPU demand of this partition
		double cpuOfPartition = 0;
		for(int k=0; k<this->embeddedPartitions->at(i)->nodes; k++){
			cpuOfPartition += this->embeddedPartitions->at(i)->cpu[k];
		}
		double endTime = this->embeddedPartitions->at(i)->request->arrtime + this->embeddedPartitions->at(i)->request->duration;
		if(endTime > tEnd){
			endTime = tEnd;
		}
		double timeVDCIsThere = endTime-currentTime;
		cpuOfPartition = cpuOfPartition*timeVDCIsThere;
		CPUToCome += cpuOfPartition;
	}

	CPUToCome = CPUToCome/(tEnd - currentTime);
	return CPUToCome;
}



double DataCenter::getEstimationOfAverageITConsumedPowerDuringUpcomingTimePeriod(double currentTime, double tEnd){
	//Returns the total Energy consumption of this data center given that some request are going to leave from the present time to tEnd
	double powerITToCome = 0;
	for(unsigned int i=0; i<this->embeddedPartitions->size(); i++){
		double powerForArtition = this->getPowerForPartition(this->embeddedPartitions->at(i));
		double endTime = this->embeddedPartitions->at(i)->request->arrtime + this->embeddedPartitions->at(i)->request->duration;
		if(endTime > tEnd){
			endTime = tEnd;
		}
		double timeVDCIsThere = endTime-currentTime;
		double energyForPartition = powerForArtition*timeVDCIsThere;
		powerITToCome += energyForPartition;
	}

	powerITToCome = powerITToCome/(tEnd - currentTime);
	return powerITToCome;
}





