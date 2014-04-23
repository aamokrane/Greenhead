#include "CentralControllerServer.h"
#include <iostream>

#include <iostream>
#include "Embed.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "RemoteCalls.h"
#include "staticUsefulMethods.h"
#include "CentralController.h"
#include "TopologyGenerator.h"
#include "GreenheadCentralControllerMysqlConnector.h"
#include "CentralControllerServerSAVIFrontEnd.h"



using namespace std;

int main(int argc, char *argv[]){

	//int serverPort = CENTRAL_CONTROLLER_PORT;
	int serverPort = 8080;

	central_controller_parameters_t* cnParameters = new central_controller_parameters_t;
	cnParameters->numberOfDatacenters = 4;
	cnParameters->numberOfNodesProvisionedNetwork = 10;
	cnParameters->physicalLinkProbability = 0.6;
	cnParameters->accessBandwidthForDC = 100000;
	cnParameters->provisionedNetworkLinkCapacity = 100000;
	cnParameters->provisionedNetworkTopology = TEST_TOPOLOGY_SAVI; //NSFNET_TOPOLOGY TEST_TOPOLOGY_SAVI RANDOM_TOPOLOGY
	cnParameters->lookAheadTimeForForcasting = 2; //For hours to make a decision to migrate or not
	cnParameters->reoptimizationInterval = 4; //Four hours to make a decision to migrate or not

	//The costs parameters
	cnParameters->costPerUnitOfBandwidthInBackboneNetwork = 0.00015; //0.00015 $/Mbps/hour, 0.15 $/Gbps/hour
	cnParameters->costPerOnNodeInProvisionedNetwork = 0.05; //PowerPerNode * Electricty Price
	cnParameters->carbonPerUnitOfBandwidthInBackbone = 0.0000005; // 0.5*e-06
	cnParameters->costPerTonOfCarbon = 30; //30$ per ton of carbon

	//The Green SLA guarantee time
	cnParameters->performanceEvaluationReportingInterval = (double)5/(double)60; //In hours
	cnParameters->parameterUpdateInterval = 0.33; //In hours, 20 minutes
	cnParameters->SLAServiceGuaranteePeriod = 24; // 24 hours
	cnParameters->timeSlotDuration = (double)1/(double)6; //in hours
	//simulator->SLAViolationCostPerUnitOfCarbon = 50; //Violation penalty per unit of Carbon Emission
	cnParameters->SLAViolationProportionOfRevenueToRefund = 0.5; //Proportion of the the revenue of the request (bill) to be given back by the InP

	cnParameters->numberOfAlternativePaths = 5;

	//cnParameters->dataBaseName = "TestVDCPlanner";
	cnParameters->dataBaseName = "TestCentralController";
	cnParameters->dataBaseServerAddress = "127.0.0.1";
	cnParameters->dataBaseServerPort = 3306;

	//cnParameters->dataBaseUsername = "Ahmed";
	//cnParameters->dataBasePassword = "Ahmed";
	cnParameters->dataBaseUsername = "root";
	cnParameters->dataBasePassword = "";



	//CentralControllerServer server = CentralControllerServer(serverPort, cnParameters);

	CentralControllerServerSAVIFrontEnd server = CentralControllerServerSAVIFrontEnd(serverPort, cnParameters);

	cout<< "Creating the Database for the central controller" <<endl;
	if(server.underlyingCentralController->dataBaseConnector->doesDataBaseExist(cnParameters->dataBaseName)){
		//Load the request from the database and embed them
		double t = 0;
		server.underlyingCentralController->loadFromDataBaseAndEmbedRequstes(t);
	}else{
		server.underlyingCentralController->dataBaseConnector->createDataBase();
	}

	//vector<VDCRequestPartitioned*>* list = server.underlyingCentralController->dataBaseConnector->readAllVDCRequestsFromDataBase();
	cout<< "Starting the server" <<endl;
	server.startServer();
	return 0;

}
