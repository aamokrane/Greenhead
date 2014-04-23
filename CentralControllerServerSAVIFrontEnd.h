/*
 * CentralControllerServerSAVIFrontEnd.h
 *
 *  Created on: Apr 14, 2014
 *      Author: ahmedamokrane
 */

#ifndef CENTRALCONTROLLERSERVERSAVIFRONTEND_H_
#define CENTRALCONTROLLERSERVERSAVIFRONTEND_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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


//These includes are from the SAVI code
//In case of using windows
#if defined (WIN32)
#include <winsock2.h>
typedef int socklen_t;
#elif defined (linux)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#endif


class CentralControllerServerSAVIFrontEnd {


public:

	//the data members
	int openedSocket;
	int listeningPort;
	CentralController* underlyingCentralController;


	CentralControllerServerSAVIFrontEnd(int port, central_controller_parameters_t* cnParameters);
	virtual ~CentralControllerServerSAVIFrontEnd();

	void startServer();

	CentralController* createUnderlyingCentralController(central_controller_parameters_t* cnParameters);

};

#endif /* CENTRALCONTROLLERSERVERSAVIFRONTEND_H_ */
