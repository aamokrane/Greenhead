/*
 * CenralControllerServer.h
 *
 *  Created on: Mar 25, 2014
 *      Author: ahmedamokrane
 */

#ifndef CENRALCONTROLLERSERVER_H_
#define CENRALCONTROLLERSERVER_H_


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


/*
 *
 *
 */

using namespace std;

class CentralControllerServer {
public:

	//the data members
	int openedSocket;
	int listeningPort;
	CentralController* underlyingCentralController;


	CentralControllerServer(int port, central_controller_parameters_t* cnParameters);
	virtual ~CentralControllerServer();

	bool startServer();

	double processRemoteCall(remote_call_t remoteCallParam, remote_call_t* remoteCallResult);

	CentralController* createUnderlyingCentralController(central_controller_parameters_t* cnParameters);

};

#endif /* CENRALCONTROLLERSERVER_H_ */
