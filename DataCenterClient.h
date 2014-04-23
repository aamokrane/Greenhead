/*
 * DataCenterClient.h
 *
 *  Created on: Mar 21, 2014
 *      Author: ahmedamokrane
 */

#ifndef DATACENTERCLIENT_H_
#define DATACENTERCLIENT_H_

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Embed.h"
#include "RemoteCalls.h"


/*
 *
 */

using namespace std;

class DataCenterClient {
public:
	DataCenterClient(char* remoteDataCenterIP, int remoteDataCenterPort);
	virtual ~DataCenterClient();

	void callRemoteMethod(remote_call_t* remoteCallParam, remote_call_t* remoteCallResult);

	int openConnexionWithRemoteDataCenter(char* ipServer, int serverPort);

	void closeConnexion();

	int remoteDataCenterSocket;

};

#endif /* DATACENTERCLIENT_H_ */
