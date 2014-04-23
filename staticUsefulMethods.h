/*
 * staticusefulMethods.h
 *
 *  Created on: Mar 21, 2014
 *      Author: ahmedamokrane
 */

#ifndef STATICUSEFULMETHODS_H_
#define STATICUSEFULMETHODS_H_

/*
 *
 */

#include "RemoteCalls.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <sstream>


using namespace std;

class staticUsefulMethods {
public:
	staticUsefulMethods();
	virtual ~staticUsefulMethods();

	static void getSerializableVDCRequest(request_t* request, request_savi_remote_call_t* result);
	static void getDeserializedVDCRequest(request_savi_remote_call_t* request, request_t* result);
	static void getSerializablePartition(partition_t* partition, partition_savi_remote_call_t* result);
	static void getDeserializedPartition(partition_savi_remote_call_t* partition, partition_t* result);
	static void getSerializableLink(link_t* link, link_savi_remote_call_t* result);
	static void gerDeserializedLink(link_savi_remote_call_t* link, link_t* result);

	/*
	static void convertRequestClassToRequestStructure( Request requestClass, request_t* requestStruct);
	static void convertRequestStructureToRequestClass(request_t* requestStruct, Request* requestClass);
	*/

	static request_t* loadVDCRequestFromString(char* strRequest, int requestID);

};

#endif /* STATICUSEFULMETHODS_H_ */
