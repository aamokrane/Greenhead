/*
 * GreenheadCentralControllerMysqlConnector.h
 *
 *  Created on: Mar 28, 2014
 *      Author: ahmedamokrane
 */

#ifndef GREENHEADCENTRALCONTROLLERMYSQLCONNECTOR_H_
#define GREENHEADCENTRALCONTROLLERMYSQLCONNECTOR_H_

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "SubstrateInfrastructure.h"
#include "VDCRequestPartitioned.h"

class GreenheadCentralControllerMysqlConnector{
public:


	string mysqlServerAddress;
	string dataBaseName;
	string userName;
	string password;
	int mysqlServerPort;



	GreenheadCentralControllerMysqlConnector(string mysqlServerAddress, int mysqlServerPort, string user, string pass, string dataBaseName);
	virtual ~GreenheadCentralControllerMysqlConnector();

	bool doesDataBaseExist(char* dataBaseName);
	int createDataBase();
	int deleteDataBase();

	int writeVDCRequestToDataBase(request_t* request);
	int writeVDCPartitionToDataBase(partition_t* partition);
	int writeVirtualLinkToDataBase(link_t* link);

	request_t* readVDCRequestFromDataBase(int idRequest);
	partition_t* readVDCPartitionFromDataBase(int idRequest, int idPartition);
	vector<partition_t*>* readAllPartitionsOfVDCRequestFromDataBase(int idRequest);

	vector<request_t*>* readAllVDCRequestsFromDataBase();

	int removeVDCRequestFromDataBase(request_t* request, double t);
	int removeVDCPartitionFromDataBase(partition_t* partition, double t);

	int updateVDCPartitionMappingInTheDataBase(partition_t* partition, double t);
	int updateVirtualLinkMappingInTheDataBase(link_t* link, double t);


	int writeInfratsructureToDataBase(SubstrateInfrastructure* infrastructure);
	SubstrateInfrastructure* readInfrastructureFromDataBase();

	int writeVDCRequestPartitionedToDataBase(VDCRequestPartitioned* vdcResuqt);
	VDCRequestPartitioned* readVDCRequestPartitionedFromDataBase(int idRequest);

	vector<VDCRequestPartitioned*>* readAllVDCRequestsPartitionedFromDataBase();
};

#endif /* GREENHEADCENTRALCONTROLLERMYSQLCONNECTOR_H_ */
