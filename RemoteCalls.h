


#ifndef REMOTECALLS_H_
#define REMOTECALLS_H_


#include "Embed.h"

//Parameter for the remote calls

#define NOPE -1
#define EMBED_LINK 1
#define EMBED_PARTITION 2
#define REMOVE_LINK 3
#define REMOVE_PARTITION 4

#define GET_COST_FOR_PARTITION 15
#define GET_POWER_FOR_PARTITION 16
#define GET_COST_FOR_LINK 17
#define GET_PUE 18
#define GET_ELECTRICITY_PRICE 19
#define GET_CARBON_PER_UNIT_OF_POWER 20
#define GET_CARBON_COST 21
#define GET_AVAILABLE_RENEWABLES 22
#define GET_INSTANTANEOUS_UTILIZATION 23
#define GET_TOTAL_CARBON_EMISSION_RATE 24
#define GET_CARBON_EMISSION_RATE_SERVERS 25
#define GET_CARBON_EMISSION_RATE_NETWORK 26

#define GET_ESTIMATION_AVAILABLE_RENEWABLE_POWER_FOR_PERIOD 30
#define GET_ESTIMATION_AVERAGE_ELECTRICITY_PRICE_FOR_PERIOD 31


#define EMBED_REQUEST 55
#define REMOVE_REQUEST 56
#define EXPORT_RESULTS 57
#define STOP_SERVER 60
#define CLIENT_IS_LEAVING 61



//The ports for the remote calls
#define CENTRAL_CONTROLLER_PORT 5000000
#define DATA_CENTER_PORT_1 6000001
#define DATA_CENTER_PORT_2 6000002
#define DATA_CENTER_PORT_3 6000003
#define DATA_CENTER_PORT_4 6000004


// Data Structures for the calls

union parameter_to_remote_call_t{
	request_savi_remote_call_t request;
	partition_savi_remote_call_t partition;
	link_savi_remote_call_t link;
	datacenter_utilization_t dataCenterUtilization;
	infrastructure_utilization_t infrastructureUtilization;
	simulation_parameters_t simulation_parameters;
	double tEnd;
};

struct remote_call_t{
	int32_t method;
	double result;
	double tBegin;
	parameter_to_remote_call_t parameter;
	//partition_savi_remote_call_t partition;
};


#endif
