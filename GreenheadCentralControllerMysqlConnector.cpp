/*
 * GreenheadCentralControllerMysqlConnector.cpp
 *
 *  Created on: Mar 28, 2014
 *      Author: ahmedamokrane
 */

#include "GreenheadCentralControllerMysqlConnector.h"

GreenheadCentralControllerMysqlConnector::GreenheadCentralControllerMysqlConnector(string mysqlServerAddress, int mysqlServerPort, string user, string pass, string dataBaseName){
	this->mysqlServerPort = mysqlServerPort;
	this->mysqlServerAddress = string("");
	this->mysqlServerAddress.append(mysqlServerAddress);
	this->dataBaseName = string("");
	this->dataBaseName.append(dataBaseName);
	this->userName = string("");
	this->userName.append(user);
	this->password = string("");
	this->password.append(pass);
}

GreenheadCentralControllerMysqlConnector::~GreenheadCentralControllerMysqlConnector() {

}




bool GreenheadCentralControllerMysqlConnector::doesDataBaseExist(char* dataBaseName) {
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::PreparedStatement *pstmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		stmt = con->createStatement();
		string msg =("show databases like \"");
		msg.append(this->dataBaseName);
		msg.append("\"");
		pstmt = con->prepareStatement(msg);
		res = pstmt->executeQuery();

		bool toReturn = false;
		if(res->next()){
			toReturn = true;
		}

		delete stmt;
		delete pstmt;
		delete con;

		return toReturn;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;
	return false;

}


int GreenheadCentralControllerMysqlConnector::createDataBase() {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;

		// Create a connection
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		stmt = con->createStatement();
		//Create the data base
		string s1("DROP DATABASE IF EXISTS ");
		s1.append(this->dataBaseName);
		stmt->execute(s1);

		string s2("CREATE DATABASE ");
		s2.append(this->dataBaseName);
		stmt->execute(s2);

		con->setSchema(this->dataBaseName);
		//vdc_requests (requestID, arrival, duration, revenue, numberNodes, numberLinks, limitCarbonEmission, availability, vdcName, userName, sessionName, applicationName)
		stmt->execute("CREATE TABLE IF NOT EXISTS vdc_requests ("
				"requestID    INT UNSIGNED  NOT NULL,"
				"arrival   DOUBLE(20,10) NOT NULL,"
				"duration   DOUBLE(20,10) NOT NULL,"
				"revenue  DOUBLE(20,10) NOT NULL,"
				"numberNodes INT NOT NULL,"
				"numberLinks INT NOT NULL,"
				"limitCarbonEmission   DOUBLE(20,10) NOT NULL,"
				"Availability   DOUBLE(15,10) NOT NULL,"
				"vdcName VARCHAR(30),"
				"userName VARCHAR(30),"
				"sessionName VARCHAR(30),"
				"applicationName VARCHAR(30),"
				"PRIMARY KEY  (requestID) )"
		);

		//vdc_partitions (requestID,partitionID, numberNodes, numberLinks, availability, vdcName, userName, sessionName, applicationName)
		stmt->execute("CREATE TABLE IF NOT EXISTS vdc_partitions ("
				"requestID    INT UNSIGNED  NOT NULL,"
				"partitionID INT UNSIGNED  NOT NULL,"
				"numberNodes INT NOT NULL,"
				"numberLinks INT NOT NULL,"
				"Availability   DOUBLE(15,10) NOT NULL,"
				"vdcName VARCHAR(30),"
				"userName VARCHAR(30),"
				"sessionName VARCHAR(30),"
				"applicationName VARCHAR(30),"
				"PRIMARY KEY  (requestID,partitionID))"
		);

		//virtual_links(requestID,nodeSource, nodeDestination, partitionSource, partitionDestination, bandwidth, delay)
		stmt->execute("CREATE TABLE IF NOT EXISTS virtual_links ("
				"requestID    INT UNSIGNED  NOT NULL,"
				"nodeSource	  INT UNSIGNED  NOT NULL,"
				"nodeDestination INT UNSIGNED  NOT NULL,"
				"partitionSource	  INT UNSIGNED,"
				"partitionDestination INT UNSIGNED,"
				"bandwidth  DOUBLE(20,10) NOT NULL,"
				"delay  DOUBLE(20,10) NOT NULL,"
				"PRIMARY KEY  (requestID,nodeSource,nodeDestination))"
		);

		//virtual_machines(requestID, nodeID, partitionID, cpu, memory, migrationCost, nodeType, xLocation, yLocation, maxDistanceLocation, flavor, disk)
		stmt->execute("CREATE TABLE IF NOT EXISTS virtual_machines ("
				"requestID INT UNSIGNED  NOT NULL,"
				"nodeID INT UNSIGNED  NOT NULL,"
				"partitionID INT UNSIGNED,"
				"cpu  DOUBLE(20,10) NOT NULL,"
				"memory  DOUBLE(20,10) NOT NULL,"
				"migrationCost  DOUBLE(20,10) NOT NULL,"
				"nodeType INT NOT NULL,"
				"xLocation INT NOT NULL,"
				"yLocation INT NOT NULL,"
				"maxDistanceLocation INT NOT NULL,"
				"flavor  INT UNSIGNED,"
				"disk  DOUBLE(20,10) NOT NULL,"
				"PRIMARY KEY  (requestID,nodeID))"
		);

		//physical_links(nodeSource, nodeDestination, bandwidth, delay)
		stmt->execute("CREATE TABLE IF NOT EXISTS physical_links ("
				"nodeSource INT UNSIGNED  NOT NULL,"
				"nodeDestination INT UNSIGNED  NOT NULL,"
				"bandwidth  DOUBLE(20,10) NOT NULL,"
				"delay  DOUBLE(20,10) NOT NULL,"
				"PRIMARY KEY  (nodeSource,nodeDestination))"
		);

		//physical_nodes(nodeID, nodeType)
		stmt->execute("CREATE TABLE IF NOT EXISTS physical_nodes ("
				"nodeID  INT UNSIGNED  NOT NULL,"
				"nodeType VARCHAR(20)  NOT NULL,"
				"PRIMARY KEY  (nodeID))"
		);

		//mapping_partition (requestID,partitionID, datacenterID, mappingBegin, mappingEnd)
		stmt->execute("CREATE TABLE IF NOT EXISTS mapping_partition ("
				"requestID INT UNSIGNED  NOT NULL,"
				"partitionID  INT UNSIGNED  NOT NULL,"
				"datacenterID INT UNSIGNED  NOT NULL,"
				"mappingBegin   DOUBLE(20,10) NOT NULL,"
				"mappingEnd   DOUBLE(20,10),"
				"PRIMARY KEY  (requestID,partitionID,mappingBegin))"
		);

		//mapping_virtual_link (requestID,virtualNodeSource, virtualNodeDestination, datacenterID, physicalNodeSource, physicalNodeDestination,  mappingBegin, mappingEnd)
		stmt->execute("CREATE TABLE IF NOT EXISTS mapping_virtual_link ("
				"requestID INT UNSIGNED  NOT NULL,"
				"virtualNodeSource  INT UNSIGNED  NOT NULL,"
				"virtualNodeDestination INT UNSIGNED  NOT NULL,"
				"datacenterID INT UNSIGNED,"
				"physicalNodeSource INT UNSIGNED,"
				"physicalNodeDestination INT UNSIGNED,"
				"mappingBegin   DOUBLE(20,10) NOT NULL,"
				"mappingEnd   DOUBLE(20,10),"
				"UNIQUE  (requestID,virtualNodeSource, virtualNodeDestination, physicalNodeSource, physicalNodeDestination, mappingBegin),"
				"UNIQUE  (requestID,virtualNodeSource, virtualNodeDestination, datacenterID, mappingBegin))"
		);


		//Create the data bases for the removed VDC requests

		stmt->execute("CREATE TABLE IF NOT EXISTS vdc_requests_archive ("
				"requestID    INT UNSIGNED  NOT NULL,"
				"arrival   DOUBLE(20,10) NOT NULL,"
				"duration   DOUBLE(20,10) NOT NULL,"
				"revenue  DOUBLE(20,10) NOT NULL,"
				"numberNodes INT NOT NULL,"
				"numberLinks INT NOT NULL,"
				"limitCarbonEmission   DOUBLE(20,10) NOT NULL,"
				"Availability   DOUBLE(15,10) NOT NULL,"
				"vdcName VARCHAR(30),"
				"userName VARCHAR(30),"
				"sessionName VARCHAR(30),"
				"applicationName VARCHAR(30),"
				"PRIMARY KEY  (requestID) )"
		);

		stmt->execute("CREATE TABLE IF NOT EXISTS vdc_partitions_archive ("
				"requestID    INT UNSIGNED  NOT NULL,"
				"partitionID INT UNSIGNED  NOT NULL,"
				"numberNodes INT NOT NULL,"
				"numberLinks INT NOT NULL,"
				"Availability   DOUBLE(15,10) NOT NULL,"
				"vdcName VARCHAR(30),"
				"userName VARCHAR(30),"
				"sessionName VARCHAR(30),"
				"applicationName VARCHAR(30),"
				"PRIMARY KEY  (requestID,partitionID))"
		);

		stmt->execute("CREATE TABLE IF NOT EXISTS virtual_links_archive ("
				"requestID    INT UNSIGNED  NOT NULL,"
				"nodeSource	  INT UNSIGNED  NOT NULL,"
				"nodeDestination INT UNSIGNED  NOT NULL,"
				"partitionSource	  INT UNSIGNED,"
				"partitionDestination INT UNSIGNED,"
				"bandwidth  DOUBLE(20,10) NOT NULL,"
				"delay  DOUBLE(20,10) NOT NULL,"
				"PRIMARY KEY  (requestID,nodeSource,nodeDestination))"
		);

		stmt->execute("CREATE TABLE IF NOT EXISTS virtual_machines_archive ("
				"requestID INT UNSIGNED  NOT NULL,"
				"nodeID INT UNSIGNED  NOT NULL,"
				"partitionID INT UNSIGNED,"
				"cpu  DOUBLE(20,10) NOT NULL,"
				"memory  DOUBLE(20,10) NOT NULL,"
				"migrationCost  DOUBLE(20,10) NOT NULL,"
				"nodeType INT NOT NULL,"
				"xLocation INT NOT NULL,"
				"yLocation INT NOT NULL,"
				"flavor  INT UNSIGNED,"
				"disk  DOUBLE(20,10) NOT NULL,"
				"maxDistanceLocation INT NOT NULL,"
				"PRIMARY KEY  (requestID,nodeID))"
		);

		stmt->execute("CREATE TABLE IF NOT EXISTS physical_links_archive ("
				"nodeSource INT UNSIGNED  NOT NULL,"
				"nodeDestination INT UNSIGNED  NOT NULL,"
				"bandwidth  DOUBLE(20,10) NOT NULL,"
				"delay  DOUBLE(20,10) NOT NULL,"
				"PRIMARY KEY  (nodeSource,nodeDestination))"
		);

		stmt->execute("CREATE TABLE IF NOT EXISTS physical_nodes_archive ("
				"nodeID  INT UNSIGNED  NOT NULL,"
				"nodeType VARCHAR(20)  NOT NULL,"
				"PRIMARY KEY  (nodeID))"
		);

		stmt->execute("CREATE TABLE IF NOT EXISTS mapping_partition_archive ("
				"requestID INT UNSIGNED  NOT NULL,"
				"partitionID  INT UNSIGNED  NOT NULL,"
				"datacenterID INT UNSIGNED  NOT NULL,"
				"mappingBegin   DOUBLE(20,10) NOT NULL,"
				"mappingEnd   DOUBLE(20,10),"
				"PRIMARY KEY  (requestID,partitionID,mappingBegin))"
		);

		stmt->execute("CREATE TABLE IF NOT EXISTS mapping_virtual_link_archive ("
				"requestID INT UNSIGNED  NOT NULL,"
				"virtualNodeSource  INT UNSIGNED  NOT NULL,"
				"virtualNodeDestination INT UNSIGNED  NOT NULL,"
				"datacenterID INT UNSIGNED,"
				"physicalNodeSource INT UNSIGNED,"
				"physicalNodeDestination INT UNSIGNED,"
				"mappingBegin   DOUBLE(20,10) NOT NULL,"
				"mappingEnd   DOUBLE(20,10),"
				"UNIQUE  (requestID,virtualNodeSource, virtualNodeDestination, physicalNodeSource, physicalNodeDestination, mappingBegin),"
				"UNIQUE  (requestID,virtualNodeSource, virtualNodeDestination, datacenterID, mappingBegin))"
		);



		//Create the TRIGGERS

		//Remove the virtual links mappings whenever the links are removed

		stmt->execute("CREATE TRIGGER delete_links_and_mappings_on_delete_partition AFTER DELETE on vdc_partitions "
				"FOR EACH ROW "
				"BEGIN "
				"DELETE FROM mapping_partition "
				"WHERE mapping_partition.requestID = old.requestID "
				"AND mapping_partition.partitionID = old.partitionID "
				";"
				"DELETE FROM virtual_machines "
				"WHERE virtual_machines.requestID = old.requestID "
				"AND virtual_machines.partitionID = old.partitionID "
				";"
				"DELETE FROM virtual_links "
				"WHERE virtual_links.requestID = old.requestID "
				"AND virtual_links.partitionSource = old.partitionID "
				"AND virtual_links.partitionDestination = old.partitionID"
				";"
				"INSERT INTO vdc_partitions_archive "
				"VALUES (old.requestID, old.partitionID, old.numberNodes, old.numberLinks, old.availability, old.vdcName, old.userName, old.sessionName, old.applicationName)"
				";"
				"END"
		);

		stmt->execute("CREATE TRIGGER delete_links_and_mappings_on_delete_request AFTER DELETE on vdc_requests "
				"FOR EACH ROW "
				"BEGIN "
				"DELETE FROM vdc_partitions "
				"WHERE vdc_partitions.requestID = old.requestID "
				";"
				"DELETE FROM virtual_machines "
				"WHERE virtual_machines.requestID = old.requestID "
				";"
				"DELETE FROM virtual_links "
				"WHERE virtual_links.requestID = old.requestID "
				";"
				"INSERT INTO vdc_requests_archive "
				"VALUES (old.requestID, old.arrival, old.duration, old.revenue, old.numberNodes, old.numberLinks, old.limitCarbonEmission, old.availability, old.vdcName, old.userName, old.sessionName, old.applicationName) "
				";"
				"END"
		);


		stmt->execute("CREATE TRIGGER delete_mappings_on_delete_virtual_machine AFTER DELETE on virtual_machines "
				"FOR EACH ROW "
				"BEGIN "
				"INSERT INTO virtual_machines_archive "
				"VALUES (old.requestID, old.nodeID, old.partitionID, old.cpu, old.memory, old.migrationCost, old.nodeType, old.xLocation, old.yLocation, old.maxDistanceLocation, old.flavor, old.disk) "
				";"
				"END"
		);

		stmt->execute("CREATE TRIGGER delete_mappings_on_delete_virtual_link AFTER DELETE on virtual_links "
				"FOR EACH ROW "
				"BEGIN "
				"DELETE FROM mapping_virtual_link "
				"WHERE mapping_virtual_link.requestID = old.requestID "
				"AND mapping_virtual_link.virtualNodeSource = old.nodeSource "
				"AND mapping_virtual_link.virtualNodeDestination = old.nodeDestination"
				";"
				"INSERT INTO virtual_links_archive "
				"VALUES (old.requestID, old.nodeSource, old.nodeDestination, old.partitionSource, old.partitionDestination, old.bandwidth, old.delay) "
				";"
				"END"
		);

		stmt->execute("CREATE TRIGGER insert_archive_mappings_on_delete_mapping_partition AFTER DELETE on mapping_partition "
				"FOR EACH ROW "
				"BEGIN "
				"INSERT INTO mapping_partition_archive "
				"VALUES (old.requestID, old.partitionID, old.datacenterID, old.mappingBegin, old.mappingEnd) "
				";"
				"END"
		);

		stmt->execute("CREATE TRIGGER insert_archive_mappings_on_delete_mapping_virtual_link AFTER DELETE on mapping_virtual_link "
				"FOR EACH ROW "
				"BEGIN "
				"INSERT INTO mapping_virtual_link_archive "
				"VALUES(old.requestID, old.virtualNodeSource, old.virtualNodeDestination, old.datacenterID, old.physicalNodeSource, old.physicalNodeDestination,  old.mappingBegin, old.mappingEnd) "
				";"
				"END"
		);


		delete stmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;
}



int GreenheadCentralControllerMysqlConnector::deleteDataBase() {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		stmt = con->createStatement();
		//Create the data base
		string s1("DROP DATABASE IF EXISTS ");
		s1.append(this->dataBaseName);
		stmt->execute(s1);

		delete stmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;
}


int GreenheadCentralControllerMysqlConnector::writeVDCRequestToDataBase(request_t* request) {


	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		//Write the request
		//vdc_requests (requestID, arrival, duration, revenue, numberNodes, numberLinks, limitCarbonEmission, availability, vdcName, userName, sessionName, applicationName)
		pstmt = con->prepareStatement("INSERT IGNORE INTO vdc_requests VALUES (?,?,?,?,?,?,?,?,?,?,?,?)");
		pstmt->setInt(1, request->reqID);
		pstmt->setDouble(2, request->arrtime);
		pstmt->setDouble(3, request->duration);
		pstmt->setDouble(4, request->revenue);
		pstmt->setInt(5, request->nodes);
		pstmt->setInt(6, request->links);
		pstmt->setDouble(7, request->limitOfCarbonEmissionPerServicePeriodGuarantee);
		pstmt->setDouble(8, request->availability);
		pstmt->setString(9, request->VDCName);
		pstmt->setString(10, request->userName);
		pstmt->setString(11, request->sessionName);
		pstmt->setString(12, request->applicationName);

		pstmt->executeUpdate();


		//Write the VMs and their corresponding mappings
		//virtual_machines(requestID, nodeID, partitionID, cpu, memory, migrationCost, nodeType, xLocation, yLocation, maxDistanceLocation, flavor, disk)
		for(int i=0; i<request->nodes;i++){
			pstmt = con->prepareStatement("INSERT IGNORE INTO virtual_machines VALUES (?,?,?,?,?,?,?,?,?,?,?,?)");
			pstmt->setInt(1, request->reqID);
			pstmt->setInt(2, request->nodesID[i]);
			//pstmt->setString(3, "NULL");
			pstmt->setNull(3, 0);
			pstmt->setDouble(4, request->cpu[i]);
			pstmt->setDouble(5, request->mem[i]);
			pstmt->setDouble(6, request->migrationCostOfVM[i]);
			pstmt->setInt(7, request->type[i]);
			pstmt->setInt(8, request->nodeLocation[i]->xLocation);
			pstmt->setInt(9, request->nodeLocation[i]->yLocation);
			pstmt->setInt(10, request->nodeLocation[i]->maxDistance);
			pstmt->setInt(11, request->flavor[i]);
			pstmt->setDouble(12, request->disk[i]);

			pstmt->executeUpdate();
		}
		//Add the mapping of the nodes to the datacenters, for Greenhead, this is contained in the partition

		//Write the Links and their corresponding mappings
		for(int i=0; i<request->links;i++){
			pstmt = con->prepareStatement("INSERT IGNORE INTO virtual_links VALUES (?,?,?,?,?,?,?)");
			pstmt->setInt(1, request->reqID);

			link_t* l = request->link[i];
			pstmt->setInt(2, l->nodeFrom);
			pstmt->setInt(3, l->nodeTo);
			pstmt->setInt(4, l->partitionFrom);
			pstmt->setInt(5, l->partitionTo);
			pstmt->setDouble(6,l->bw);
			pstmt->setDouble(7,l->delay);
			pstmt->executeUpdate();
		}

		//Write the mapping of the link to the database
		for(int i=0; i<request->links;i++){
			link_t* l = request->link[i];
			if(l->mapping != NULL && (l->mapping->DCId >= 0 || l->mapping!=NULL)){
				//This link has been embedded, check out its mapping
				if(l->mapping->DCId >= 0){
					//This link is mapped to a data center
					pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
					pstmt->setInt(1, l->idRequest);
					pstmt->setInt(2, l->nodeFrom);
					pstmt->setInt(3, l->nodeTo);
					pstmt->setInt(4, l->mapping->DCId);
					//pstmt->setString(5,"NULL");
					//pstmt->setString(6,"NULL");
					pstmt->setNull(5, 0);
					pstmt->setNull(6, 0);
					pstmt->setDouble(7,l->mapping->mapBegin);
					pstmt->setDouble(8,l->mapping->mapEnd);
					pstmt->executeUpdate();
				}else{
					//This link is embedded in the backbon network
					path_t* path = l->mapping->substratePath;
					for(int j=0; j<path->len-1; j++){
						int src = path->link[j];
						int dest = path->link[j+1];
						pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
						pstmt->setInt(1, l->idRequest);
						pstmt->setInt(2, l->nodeFrom);
						pstmt->setInt(3, l->nodeTo);
						//pstmt->setString(4, "NULL");
						pstmt->setNull(4, 0);
						pstmt->setInt(5,src);
						pstmt->setInt(6,dest);
						pstmt->setDouble(7,l->mapping->mapBegin);
						pstmt->setDouble(8,l->mapping->mapEnd);
						pstmt->executeUpdate();
					}
				}
			}
		}

		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;

}

int GreenheadCentralControllerMysqlConnector::writeVDCPartitionToDataBase(partition_t* partition) {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		//Write the partition
		pstmt = con->prepareStatement("INSERT IGNORE INTO vdc_partitions VALUES (?,?,?,?,?,?,?,?,?)");
		pstmt->setInt(1, partition->reqID);
		pstmt->setInt(2, partition->partitionID);
		pstmt->setInt(3, partition->nodes);
		pstmt->setInt(4, partition->links);
		pstmt->setDouble(5, partition->availability);
		pstmt->setString(6, partition->VDCName);
		pstmt->setString(7, partition->userName);
		pstmt->setString(8, partition->sessionName);
		pstmt->setString(9, partition->applicationName);
		pstmt->executeUpdate();

		//Write the mapping of the partition
		if(partition->mapping != NULL){
			pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_partition VALUES (?,?,?,?,?)");
			pstmt->setInt(1, partition->reqID);
			pstmt->setInt(2, partition->partitionID);
			pstmt->setInt(3, partition->mapping->datacenterID);
			pstmt->setDouble(4, partition->mapping->mapBegin);
			pstmt->setDouble(5, partition->mapping->mapEnd);
			pstmt->executeUpdate();
		}

		//Write the VMs and their corresponding mappings

		//virtual_machines(requestID, nodeID, partitionID, cpu, memory, migrationCost, nodeType, xLocation, yLocation, maxDistanceLocation, flavor, disk)

		for(int i=0; i<partition->nodes;i++){
			pstmt = con->prepareStatement("INSERT IGNORE INTO virtual_machines VALUES (?,?,?,?,?,?,?,?,?,?,?,?) ON DUPLICATE KEY UPDATE partitionID = (?)");
			pstmt->setInt(1, partition->reqID);
			pstmt->setInt(2, partition->nodesID[i]);
			//pstmt->setString(3, "NULL");
			pstmt->setInt(3,partition->partitionID);
			pstmt->setDouble(4, partition->cpu[i]);
			pstmt->setDouble(5, partition->mem[i]);
			pstmt->setDouble(6, partition->migrationCost/partition->nodes);
			pstmt->setInt(7, partition->type[i]);
			pstmt->setInt(8, partition->nodeLocation[i]->xLocation);
			pstmt->setInt(9, partition->nodeLocation[i]->yLocation);
			pstmt->setInt(10, partition->nodeLocation[i]->maxDistance);
			pstmt->setInt(11, partition->flavor[i]);
			pstmt->setDouble(12, partition->disk[i]);
			pstmt->setInt(13,partition->partitionID);

			pstmt->executeUpdate();
		}
		//Add the mapping of the nodes to the datacenters, for Greenhead, this is contained in the partition

		//Write the Links and their corresponding mappings
		for(int i=0; i<partition->links;i++){
			pstmt = con->prepareStatement("INSERT IGNORE INTO virtual_links VALUES (?,?,?,?,?,?,?)");
			pstmt->setInt(1, partition->reqID);

			link_t* l = partition->link[i];
			pstmt->setInt(2, l->nodeFrom);
			pstmt->setInt(3, l->nodeTo);
			pstmt->setInt(4, l->partitionFrom);
			pstmt->setInt(5, l->partitionTo);
			pstmt->setDouble(6,l->bw);
			pstmt->setDouble(7,l->delay);
			pstmt->executeUpdate();
		}

		//Write the mapping of the link to the database
		for(int i=0; i<partition->links;i++){
			link_t* l = partition->link[i];
			if(l->mapping != NULL && (l->mapping->DCId >= 0 || l->mapping!=NULL)){
				//This link has been embedded, check out its mapping
				if(l->mapping->DCId >= 0){
					//This link is mapped to a data center
					pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
					pstmt->setInt(1, l->idRequest);
					pstmt->setInt(2, l->nodeFrom);
					pstmt->setInt(3, l->nodeTo);
					pstmt->setInt(4, l->mapping->DCId);
					//pstmt->setString(5,"NULL");
					//pstmt->setString(6,"NULL");
					pstmt->setNull(5, 0);
					pstmt->setNull(6, 0);
					pstmt->setDouble(7,l->mapping->mapBegin);
					pstmt->setDouble(8,l->mapping->mapEnd);
					pstmt->executeUpdate();
				}else{
					//This link is embedded in the backbon network
					path_t* path = l->mapping->substratePath;
					for(int j=0; j<path->len-1; j++){
						int src = path->link[j];
						int dest = path->link[j+1];
						pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
						pstmt->setInt(1, l->idRequest);
						pstmt->setInt(2, l->nodeFrom);
						pstmt->setInt(3, l->nodeTo);
						//pstmt->setString(4, "NULL");
						pstmt->setNull(4, 0);
						pstmt->setInt(5,src);
						pstmt->setInt(6,dest);
						pstmt->setDouble(7,l->mapping->mapBegin);
						pstmt->setDouble(8,l->mapping->mapEnd);
						pstmt->executeUpdate();
					}
				}
			}
		}

		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;

}

request_t* GreenheadCentralControllerMysqlConnector::readVDCRequestFromDataBase(int idRequest) {

	request_t* request = NULL;

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		pstmt = con->prepareStatement("SELECT * FROM vdc_requests WHERE requestID = (?)");
		pstmt->setInt(1,idRequest);
		res = pstmt->executeQuery();
		if(!res->next()){
			//The request does not exist in the data base

		}else{
			//The request exists in the data base, build the VDC request
			//vdc_requests (requestID, arrival, duration, revenue, numberNodes, numberLinks, limitCarbonEmission, availability, vdcName, userName, sessionName, applicationName)
			request = new request_t;
			request->reqID = res->getInt("requestID");
			request->arrtime = res->getDouble("arrival");
			request->duration = res->getDouble("duration");
			request->revenue = res->getDouble("revenue");
			request->nodes = res->getInt("numberNodes");
			request->links = res->getInt("numberLinks");
			request->limitOfCarbonEmissionPerServicePeriodGuarantee = res->getInt("limitCarbonEmission");
			request->availability = res->getDouble("availability");
			strcpy(request->VDCName,res->getString("vdcName").c_str());
			strcpy(request->userName,res->getString("userName").c_str());
			strcpy(request->sessionName,res->getString("sessionName").c_str());
			strcpy(request->applicationName,res->getString("applicationName").c_str());

			//Read the VMs
			pstmt = con->prepareStatement("SELECT * FROM virtual_machines WHERE requestID = (?) ORDER BY nodeID ASC ");
			pstmt->setInt(1,idRequest);
			res = pstmt->executeQuery();
			int i = 0;
			while(res->next()){
				request->nodesID[i] = res->getInt("nodeID");
				request->cpu[i] = res->getDouble("cpu");
				request->mem[i] = res->getDouble("memory");
				request->migrationCostOfVM[i] = res->getDouble("migrationCost");
				request->type[i] = res->getInt("nodeType");
				request->flavor[i] = res->getInt("flavor");
				request->disk[i] = res->getDouble("disk");
				location_t* loc = new location_t;
				loc->xLocation = res->getInt("xLocation");
				loc->yLocation = res->getInt("yLocation");
				loc->maxDistance = res->getInt("maxDistanceLocation");
				request->nodeLocation[i] = loc;
				i++;
			}

			//Read the links
			pstmt = con->prepareStatement("SELECT * FROM virtual_links WHERE requestID = (?)");
			pstmt->setInt(1,idRequest);
			res = pstmt->executeQuery();
			i = 0;
			while(res->next()){
				link_t* l = new link_t;
				l->nodeFrom = res->getInt("nodeSource");
				l->nodeTo = res->getInt("nodeDestination");
				l->partitionFrom = res->getInt("partitionSource");
				l->partitionTo = res->getInt("partitionDestination");
				l->bw = res->getDouble("bandwidth");
				l->delay = res->getDouble("delay");

				l->mapping = new mapping_virtual_link_to_substrate_path;
				l->mapping->virtualLink = l;
				l->mapping->requestID = l->idRequest;
				l->mapping->substratePath = NULL;
				l->mapping->DCId = -1;
				l->mapping->requestID = idRequest;
				l->mapping->mapBegin = -1;
				l->mapping->mapEnd = -1;

				sql::PreparedStatement* pstmtMappings = con->prepareStatement("SELECT * FROM mapping_virtual_link WHERE requestID = (?) AND virtualNodeSource = (?) AND  virtualNodeDestination = (?) AND ((mappingEnd IS NULL) OR (mappingEnd = -1))");
				pstmtMappings->setInt(1,idRequest);
				pstmtMappings->setInt(2,l->nodeFrom);
				pstmtMappings->setInt(3,l->nodeTo);
				sql::ResultSet *resMappings = pstmtMappings->executeQuery();

				if(resMappings->next()){
					l->mapping->mapBegin = resMappings->getDouble("mappingBegin");
					l->mapping->mapEnd = resMappings->getDouble("mappingEnd");
					if(!resMappings->isNull("datacenterID") && resMappings->getInt("datacenterID")>=0){
						//This link is mapped to a data center
						l->mapping->DCId = resMappings->getInt("datacenterID");
					}else{
						//This link has been mapped to a path in the backbone network
						vector<int> sources = vector<int>();
						vector<int> destinations = vector<int>();
						do{
							sources.push_back(resMappings->getInt("physicalNodeSource"));
							destinations.push_back(resMappings->getInt("physicalNodeDestination"));
						}while(resMappings->next());

						//build the path
						path_t* path = new path_t;
						int k=0;
						bool found = false;
						int indSrc=-1;
						while(!found && k<sources.size()){
							int v = sources[k];
							if(find(destinations.begin(), destinations.end(),v) != destinations.end()){
								k++;
							}else{
								//This is the source of the path
								indSrc = k;
								found = true;
							}
						}
						//we have the source of the path, we build the path from the two tables
						path->len = sources.size()+1;
						int z = 0;
						path->link[z] = sources[indSrc];
						z++;
						while(z<path->len){
							path->link[z] = destinations[indSrc];
							indSrc = find(sources.begin(), sources.end(),destinations[indSrc]) - sources.begin();
							z++;
						}
						l->mapping->substratePath = path;
					}//End Else DCid == 0
				}//End ResMappings.next()

				delete pstmtMappings;
				delete resMappings;
				request->link[i] = l;
				i++;
			}//end While res->next() //Number of links
		}
		delete res;
		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
		delete request;
		request = NULL;
	}

	cout << endl;

	return request;
}



int GreenheadCentralControllerMysqlConnector::writeVirtualLinkToDataBase(link_t* link) {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		//Write the Links and their corresponding mappings

		pstmt = con->prepareStatement("INSERT IGNORE INTO virtual_links VALUES (?,?,?,?,?,?,?)");
		pstmt->setInt(1, link->idRequest);
		pstmt->setInt(2, link->nodeFrom);
		pstmt->setInt(3, link->nodeTo);
		pstmt->setInt(4, link->partitionFrom);
		pstmt->setInt(5, link->partitionTo);
		pstmt->setDouble(6,link->bw);
		pstmt->setDouble(7,link->delay);
		pstmt->executeUpdate();


		//Write the mapping of the link to the database

		if(link->mapping != NULL && (link->mapping->DCId >= 0 || link->mapping!=NULL)){
			//This link has been embedded, check out its mapping
			if(link->mapping->DCId >= 0){
				//This link is mapped to a data center
				pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
				pstmt->setInt(1, link->idRequest);
				pstmt->setInt(2, link->nodeFrom);
				pstmt->setInt(3, link->nodeTo);
				pstmt->setInt(4, link->mapping->DCId);
				pstmt->setNull(5,0);
				pstmt->setNull(6,0);
				pstmt->setDouble(7,link->mapping->mapBegin);
				pstmt->setDouble(8,link->mapping->mapEnd);
				pstmt->executeUpdate();
			}else{
				//This link is embedded in the backbon network
				path_t* path = link->mapping->substratePath;
				for(int j=0; j<path->len-1; j++){
					int src = path->link[j];
					int dest = path->link[j+1];
					pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
					pstmt->setInt(1, link->idRequest);
					pstmt->setInt(2, link->nodeFrom);
					pstmt->setInt(3, link->nodeTo);
					pstmt->setNull(4, 0);
					pstmt->setInt(5,src);
					pstmt->setInt(6,dest);
					pstmt->setDouble(7,link->mapping->mapBegin);
					pstmt->setDouble(8,link->mapping->mapEnd);
					pstmt->executeUpdate();
				}
			}
		}

		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;

}



partition_t* GreenheadCentralControllerMysqlConnector::readVDCPartitionFromDataBase(int idRequest, int idPartition) {

	partition_t* partition = NULL;

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		pstmt = con->prepareStatement("SELECT * FROM vdc_partitions WHERE requestID = (?) AND partitionID = (?)");
		pstmt->setInt(1,idRequest);
		pstmt->setInt(2,idPartition);
		res = pstmt->executeQuery();
		if(!res->next()){
			//The request does not exist in the data base

		}else{
			//The request exists in the data base, build the VDC request
			partition = new partition_t;
			partition->reqID = res->getInt("requestID");
			partition->request = NULL;
			partition->partitionID = idPartition;
			partition->nodes = res->getInt("numberNodes");
			partition->links = res->getInt("numberLinks");
			partition->availability = res->getDouble("availability");
			strcpy(partition->VDCName,res->getString("vdcName").c_str());
			strcpy(partition->userName,res->getString("userName").c_str());
			strcpy(partition->sessionName,res->getString("sessionName").c_str());
			strcpy(partition->applicationName,res->getString("applicationName").c_str());

			//Read the mapping of the partition
			pstmt = con->prepareStatement("SELECT * FROM mapping_partition WHERE requestID = (?) AND partitionID = (?) AND ((mappingEnd IS NULL) OR (mappingEnd = -1))");
			pstmt->setInt(1,idRequest);
			pstmt->setInt(2,idPartition);
			res = pstmt->executeQuery();
			if(res->next()){
				partition->mapping = new mapping_parition_to_datacenter;
				partition->mapping->datacenterID = res->getInt("datacenterID");
				partition->mapping->mapBegin = res->getInt("mappingBegin");
				partition->mapping->mapEnd = res->getInt("mappingEnd");
				partition->mapping->partitioID = idPartition;
				partition->mapping->requestID = idRequest;
				partition->mapping->costPerUnitOfTime = 0; //Chaneg ethe code if there is a cost per unit of time
			}else{
				partition->mapping = NULL;
			}


			//Read the VMs
			pstmt = con->prepareStatement("SELECT * FROM virtual_machines WHERE requestID = (?)  AND partitionID = (?) ORDER BY nodeID ASC ");
			pstmt->setInt(1,idRequest);
			pstmt->setInt(2,idPartition);
			res = pstmt->executeQuery();
			int i = 0;
			while(res->next()){
				partition->nodesID[i] = res->getInt("nodeID");
				partition->cpu[i] = res->getDouble("cpu");
				partition->mem[i] = res->getDouble("memory");
				partition->migrationCost = res->getDouble("migrationCost")*partition->nodes;
				partition->type[i] = res->getInt("nodeType");
				partition->flavor[i] = res->getInt("flavor");
				partition->disk[i] = res->getDouble("disk");
				location_t* loc = new location_t;
				loc->xLocation = res->getInt("xLocation");
				loc->yLocation = res->getInt("yLocation");
				loc->maxDistance = res->getInt("maxDistanceLocation");
				partition->nodeLocation[i] = loc;
				i++;
			}

			//Read the links
			pstmt = con->prepareStatement("SELECT * FROM virtual_links WHERE requestID = (?) AND partitionSource = (?) AND partitionDestination = (?)");
			pstmt->setInt(1,idRequest);
			pstmt->setInt(2,idPartition);
			pstmt->setInt(3,idPartition);
			res = pstmt->executeQuery();
			i = 0;
			while(res->next()){
				link_t* l = new link_t;
				l->nodeFrom = res->getInt("nodeSource");
				l->nodeTo = res->getInt("nodeDestination");
				l->partitionFrom = res->getInt("partitionSource");
				l->partitionTo = res->getInt("partitionDestination");
				l->bw = res->getDouble("bandwidth");
				l->delay = res->getDouble("delay");

				l->mapping = new mapping_virtual_link_to_substrate_path;
				l->mapping->virtualLink = l;
				l->mapping->requestID = l->idRequest;
				l->mapping->substratePath = NULL;
				l->mapping->DCId = -1;
				l->mapping->requestID = idRequest;
				l->mapping->mapBegin = -1;
				l->mapping->mapEnd = -1;

				sql::PreparedStatement* pstmtMappings = con->prepareStatement("SELECT * FROM mapping_virtual_link WHERE requestID = (?) AND virtualNodeSource = (?) AND  virtualNodeDestination = (?)  AND ((mappingEnd IS NULL) OR (mappingEnd = -1))");
				pstmtMappings->setInt(1,idRequest);
				pstmtMappings->setInt(2,l->nodeFrom);
				pstmtMappings->setInt(3,l->nodeTo);
				sql::ResultSet *resMappings = pstmtMappings->executeQuery();

				if(resMappings->next()){
					l->mapping->mapBegin = resMappings->getDouble("mappingBegin");
					l->mapping->mapEnd = resMappings->getDouble("mappingEnd");
					if(!resMappings->isNull("datacenterID") && resMappings->getInt("datacenterID")>=0){
						//This link is mapped to a data center
						l->mapping->DCId = resMappings->getInt("datacenterID");
					}else{
						//This link has been mapped to a path in the backbone network
						vector<int> sources = vector<int>();
						vector<int> destinations = vector<int>();
						do{
							sources.push_back(resMappings->getInt("physicalNodeSource"));
							destinations.push_back(resMappings->getInt("physicalNodeDestination"));
						}while(resMappings->next());

						//build the path
						path_t* path = new path_t;
						int k=0;
						bool found = false;
						int srcPath, indexSrcPath;
						while(!found && k<sources.size()){
							int v = sources[k];
							bool stop = true;

							if(find(destinations.begin(), destinations.end(),v) != destinations.end()){
								k++;
							}else{
								//This is the source of the path
								srcPath = v;
								indexSrcPath = k;
								found = true;
							}
						}
						//we have the source of the path, we build the path from the two tables
						int indSrc = indexSrcPath;
						path->len = sources.size()+1;
						int z = 0;
						path->link[z] = sources[indSrc];
						z++;
						while(z<path->len){
							path->link[z] = destinations[indSrc];
							indSrc = find(sources.begin(), sources.end(),destinations[indSrc]) - sources.begin();
							z++;
						}
						l->mapping->substratePath = path;
					}//End Else DCid == 0
				}//End ResMappings.next()

				delete pstmtMappings;
				delete resMappings;
				partition->link[i] = l;
				i++;
			}//end While res->next() //Number of links
		}
		delete res;
		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
		delete partition;
		partition = NULL;
	}

	cout << endl;

	return partition;
}



vector<partition_t*>* GreenheadCentralControllerMysqlConnector::readAllPartitionsOfVDCRequestFromDataBase(int idRequest) {

	vector<partition_t*>* listResults = new vector<partition_t*>();
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::ResultSet *res1;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		pstmt = con->prepareStatement("SELECT * FROM vdc_partitions WHERE requestID = (?)");
		pstmt->setInt(1,idRequest);
		res1 = pstmt->executeQuery();

		while(res1->next()){

			int partirionID = res1->getInt("partitionID");
			partition_t* partition = this->readVDCPartitionFromDataBase(idRequest, partirionID);
			listResults->push_back(partition);

		}//End While on the number of Partitions
		delete res1;
		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;
	return listResults;

}








vector<request_t*>* GreenheadCentralControllerMysqlConnector::readAllVDCRequestsFromDataBase() {

	vector<request_t*>* listResults = new vector<request_t*>();
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		pstmt = con->prepareStatement("SELECT * FROM vdc_requests");
		res = pstmt->executeQuery();

		while(res->next()){
			int idRequest = res->getInt("requestID");
			request_t* req = this->readVDCRequestFromDataBase(idRequest);
			listResults->push_back(req);
		}//End While on the number of request

		delete res;
		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;
	return listResults;


}

int GreenheadCentralControllerMysqlConnector::removeVDCRequestFromDataBase(request_t* request, double t) {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		//Update the mapping end first

		//Update the old mapping
		pstmt = con->prepareStatement(
				"UPDATE mapping_partition SET mappingEnd = (?)"
				"WHERE requestID = (?) AND ((mappingEnd IS NULL) OR (mappingEnd = -1))"
		);
		pstmt->setDouble(1,t);
		pstmt->setInt(2,request->reqID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement(
				"UPDATE mapping_virtual_link SET mappingEnd = (?)"
				"WHERE requestID = (?) AND ((mappingEnd IS NULL) OR (mappingEnd = -1))"
		);
		pstmt->setDouble(1,t);
		pstmt->setInt(2,request->reqID);
		pstmt->executeUpdate();


		pstmt = con->prepareStatement("DELETE FROM vdc_requests WHERE requestID = (?)");
		pstmt->setInt(1,request->reqID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM vdc_partitions WHERE requestID = (?)");
		pstmt->setInt(1,request->reqID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM virtual_links WHERE requestID = (?)");
		pstmt->setInt(1,request->reqID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM virtual_machines WHERE requestID = (?)");
		pstmt->setInt(1,request->reqID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM mapping_partition WHERE requestID = (?)");
		pstmt->setInt(1,request->reqID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM mapping_virtual_link WHERE requestID = (?)");
		pstmt->setInt(1,request->reqID);
		pstmt->executeUpdate();

		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
		delete request;
		request = NULL;
	}

	cout << endl;

	return EXIT_SUCCESS;


}

int GreenheadCentralControllerMysqlConnector::removeVDCPartitionFromDataBase(partition_t* partition, double t) {
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);


		//Update the old mapping
		pstmt = con->prepareStatement(
				"UPDATE mapping_partition SET mappingEnd = (?)"
				"WHERE requestID = (?) AND partitionID = (?)"
		);
		pstmt->setDouble(1,t);
		pstmt->setInt(2,partition->reqID);
		pstmt->setInt(2,partition->partitionID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement(
				"UPDATE mapping_virtual_link SET mappingEnd = (?)"
				"WHERE requestID = (?) AND partitionSource = (?) AND partitionDestination = (?)"
		);
		pstmt->setInt(1,partition->reqID);
		pstmt->setInt(2,partition->partitionID);
		pstmt->setInt(3,partition->partitionID);
		pstmt->executeUpdate();


		//

		pstmt = con->prepareStatement("DELETE FROM vdc_partitions WHERE requestID = (?) AND partitionID = (?)");
		pstmt->setInt(1,partition->reqID);
		pstmt->setInt(2,partition->partitionID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM virtual_links WHERE requestID = (?) AND partitionSource = (?) AND partitionDestination = (?)");
		pstmt->setInt(1,partition->reqID);
		pstmt->setInt(2,partition->partitionID);
		pstmt->setInt(3,partition->partitionID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM virtual_machines WHERE requestID = (?) AND partitionID = (?)");
		pstmt->setInt(1,partition->reqID);
		pstmt->setInt(2,partition->partitionID);
		pstmt->executeUpdate();

		pstmt = con->prepareStatement("DELETE FROM mapping_partition WHERE requestID = (?) AND partitionID = (?)");
		pstmt->setInt(1,partition->reqID);
		pstmt->setInt(2,partition->partitionID);
		pstmt->executeUpdate();

		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;

}

int GreenheadCentralControllerMysqlConnector::updateVDCPartitionMappingInTheDataBase(partition_t* partition, double t) {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		//Update the old mapping
		pstmt = con->prepareStatement(
				"UPDATE mapping_partition SET mappingEnd = (?)"
				"WHERE requestID = (?) AND partitionID = (?) AND ((mappingEnd IS NULL) OR (mappingEnd = -1))"
		);
		pstmt->setDouble(1,t);
		pstmt->setInt(2,partition->reqID);
		pstmt->setInt(3,partition->partitionID);
		pstmt->executeUpdate();

		//Insert the new mapping to the data base
		pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_partition VALUES (?,?,?,?,?)");
		pstmt->setInt(1, partition->reqID);
		pstmt->setInt(2, partition->partitionID);
		pstmt->setInt(3, partition->mapping->datacenterID);
		pstmt->setDouble(4, partition->mapping->mapBegin);
		pstmt->setDouble(5, partition->mapping->mapEnd);
		pstmt->executeUpdate();


		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;

}

int GreenheadCentralControllerMysqlConnector::updateVirtualLinkMappingInTheDataBase(link_t* link, double t) {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		//Update the old mapping
		pstmt = con->prepareStatement(
				"UPDATE mapping_virtual_link SET mappingEnd = (?)"
				"WHERE requestID = (?) AND virtualNodeSource = (?) AND virtualNodeDestination = (?) AND ((mappingEnd IS NULL) OR (mappingEnd = -1))"
		);
		pstmt->setDouble(1,t);
		pstmt->setInt(2,link->nodeFrom);
		pstmt->setInt(3,link->nodeTo);
		pstmt->executeUpdate();

		//Insert the new mapping to the data base
		if(link->mapping->DCId >= 0){
			//This link is mapped to a data center
			pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
			pstmt->setInt(1, link->idRequest);
			pstmt->setInt(2, link->nodeFrom);
			pstmt->setInt(3, link->nodeTo);
			pstmt->setInt(4, link->mapping->DCId);
			pstmt->setNull(5,0);
			pstmt->setNull(6,0);
			pstmt->setDouble(7,link->mapping->mapBegin);
			pstmt->setDouble(8,link->mapping->mapEnd);
			pstmt->executeUpdate();
		}else{
			//This link is embedded in the backbon network
			path_t* path = link->mapping->substratePath;
			for(int j=0; j<path->len-1; j++){
				int src = path->link[j];
				int dest = path->link[j+1];
				pstmt = con->prepareStatement("INSERT IGNORE INTO mapping_virtual_link VALUES (?,?,?,?,?,?,?,?)");
				pstmt->setInt(1, link->idRequest);
				pstmt->setInt(2, link->nodeFrom);
				pstmt->setInt(3, link->nodeTo);
				pstmt->setNull(4, 0);
				pstmt->setInt(5,src);
				pstmt->setInt(6,dest);
				pstmt->setDouble(7,link->mapping->mapBegin);
				pstmt->setDouble(8,link->mapping->mapEnd);
				pstmt->executeUpdate();
			}
		}


		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;
}





int GreenheadCentralControllerMysqlConnector::writeVDCRequestPartitionedToDataBase(VDCRequestPartitioned* vdcResuqt) {

	//Write the request to the database
	this->writeVDCRequestToDataBase(vdcResuqt->originalRequest);

	//Write the partitions
	for(int i=0; i<vdcResuqt->numberOfPartitions; i++){
		this->writeVDCPartitionToDataBase(vdcResuqt->partitions->at(i));
	}

	//Write the links between the partitions
	for(int i=0; i<vdcResuqt->numberOfPartitions; i++){
		for(int j=0; j<vdcResuqt->numberOfPartitions; j++){
			if(i != j){
				for(unsigned int k=0; k<vdcResuqt->linksBetweenPartirions[i][j]->size();k++ ){
					link_t* link = vdcResuqt->linksBetweenPartirions[i][j]->at(k);
					this->writeVirtualLinkToDataBase(link);
				}
			}
		}
	}

	//The Alternate way is to write the original request_t
	//this->writeVDCRequestToDataBase(vdcResuqt->originalRequest);

	return EXIT_SUCCESS;
}



VDCRequestPartitioned* GreenheadCentralControllerMysqlConnector::readVDCRequestPartitionedFromDataBase(int idRequest) {

	try{
		request_t* originalRequest = this->readVDCRequestFromDataBase(idRequest);
		if(originalRequest == NULL){
			return NULL;
		}
		VDCRequestPartitioned* vdcRequestPartitioned = new VDCRequestPartitioned(originalRequest);
		vdcRequestPartitioned->partitions = this->readAllPartitionsOfVDCRequestFromDataBase(idRequest);
		vdcRequestPartitioned->numberOfPartitions = vdcRequestPartitioned->partitions->size();

		//read the set of links between partition from the data base
		for(int i=0; i<vdcRequestPartitioned->numberOfPartitions; i++){
			vdcRequestPartitioned->partitions->at(i)->request = originalRequest;
			for(int j=i+1; j<vdcRequestPartitioned->numberOfPartitions; j++){
				vdcRequestPartitioned->linksBetweenPartirions[i][j] = new vector<link_t*>();
				vdcRequestPartitioned->linksBetweenPartirions[j][i] = new vector<link_t*>();
			}
		}

		sql::Driver *driver;
		sql::Connection *con;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);



		//Read the links between the partitions
		pstmt = con->prepareStatement("SELECT * FROM virtual_links WHERE requestID = (?) AND partitionSource <> partitionDestination");
		pstmt->setInt(1,idRequest);
		res = pstmt->executeQuery();
		vector<link_t*> linkList = vector<link_t*>();
		while(res->next()){
			link_t* l = new link_t;
			l->nodeFrom = res->getInt("nodeSource");
			l->nodeTo = res->getInt("nodeDestination");
			l->partitionFrom = res->getInt("partitionSource");
			l->partitionTo = res->getInt("partitionDestination");
			l->bw = res->getDouble("bandwidth");
			l->delay = res->getDouble("delay");

			l->mapping = new mapping_virtual_link_to_substrate_path;
			l->mapping->virtualLink = l;
			l->mapping->requestID = l->idRequest;
			l->mapping->substratePath = NULL;
			l->mapping->DCId = -1;
			l->mapping->requestID = idRequest;
			l->mapping->mapBegin = -1;
			l->mapping->mapEnd = -1;

			sql::PreparedStatement* pstmtMappings = con->prepareStatement("SELECT * FROM mapping_virtual_link WHERE requestID = (?) AND virtualNodeSource = (?) AND  virtualNodeDestination = (?)");
			pstmtMappings->setInt(1,idRequest);
			pstmtMappings->setInt(2,l->nodeFrom);
			pstmtMappings->setInt(3,l->nodeTo);
			sql::ResultSet *resMappings = pstmtMappings->executeQuery();

			if(resMappings->next()){
				l->mapping->mapBegin = resMappings->getDouble("mappingBegin");
				l->mapping->mapEnd = resMappings->getDouble("mappingEnd");
				if(resMappings->getInt("datacenterID")>=0){
					//This link is mapped to a data center
					l->mapping->DCId = resMappings->getInt("datacenterID");
				}else{
					//This link has been mapped to a path in the backbone network
					vector<int> sources = vector<int>();
					vector<int> destinations = vector<int>();
					while(resMappings){
						sources.push_back(resMappings->getInt("physicalNodeSource"));
						destinations.push_back(resMappings->getInt("physicalNodeDestination"));
						resMappings->next();
					}

					//build the path
					path_t* path = new path_t;
					int k=0;
					bool found = false;
					int indSrc=-1;
					while(!found && k<sources.size()){
						int v = sources[k];
						if(find(destinations.begin(), destinations.end(),v) != destinations.end()){
							k++;
						}else{
							//This is the source of the path
							indSrc = k;
							found = true;
						}
					}
					//we have the source of the path, we build the path from the two tables
					path->len = sources.size()+1;
					int z = 0;
					path->link[z] = sources[indSrc];
					z++;
					while(z<path->len){
						path->link[z] = destinations[indSrc];
						indSrc = find(sources.begin(), sources.end(),destinations[indSrc]) - sources.begin();
						z++;
					}
					l->mapping->substratePath = path;
				}//End Else DCid == 0
			}//End ResMappings.next()

			delete pstmtMappings;
			delete resMappings;
			linkList.push_back(l);
		}//End while on the number of links

		delete res;
		delete pstmt;
		delete con;

		//Add the list of links between partitions
		for(int i =0; i<linkList.size(); i++){
			link_t* l = linkList.at(i);
			int src = l->partitionFrom;
			int dest = l->partitionTo;
			vdcRequestPartitioned->linksBetweenPartirions[src][dest]->push_back(l);
		}

		cout << endl;
		return vdcRequestPartitioned;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
		return NULL;
	}

}




int GreenheadCentralControllerMysqlConnector::writeInfratsructureToDataBase(SubstrateInfrastructure* infrastructure) {

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);




		//Write the Physical nodes and their corresponding mappings

		for(int i=0; i<infrastructure->numberOfDatacenters;i++){
			pstmt = con->prepareStatement("INSERT IGNORE INTO physical_nodes VALUES (?,?)");
			pstmt->setInt(1, infrastructure->dataCenters->at(i)->id);
			pstmt->setString(2,"Data Center");
			pstmt->executeUpdate();
		}

		for(int i=0; i<infrastructure->numberOfNodesInProvisionedNetwork;i++){
			pstmt = con->prepareStatement("INSERT IGNORE INTO physical_nodes VALUES (?,?)");
			pstmt->setInt(1, infrastructure->getProvisionedNetworkNodes()->at(i)->idNode);
			pstmt->setString(2,"Router");
			pstmt->executeUpdate();
		}
		//Add the mapping of the nodes to the datacenters, for Greenhead, this is contained in the partition

		//Write the Links and their corresponding mappings
		for(int i=0; i<infrastructure->links->size(); i++){
			pstmt = con->prepareStatement("INSERT IGNORE INTO physical_links VALUES (?,?,?,?)");
			link_t* l = infrastructure->links->at(i);
			pstmt->setInt(1, l->nodeFrom);
			pstmt->setInt(2, l->nodeTo);
			pstmt->setDouble(3,l->bw);
			pstmt->setDouble(4,l->delay);
			pstmt->executeUpdate();
		}

		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;



}

SubstrateInfrastructure* GreenheadCentralControllerMysqlConnector::readInfrastructureFromDataBase() {


}


vector<VDCRequestPartitioned*>* GreenheadCentralControllerMysqlConnector::readAllVDCRequestsPartitionedFromDataBase() {

	vector<VDCRequestPartitioned*>* listRequests = new vector<VDCRequestPartitioned*>();
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();

		string connectTo("tcp://");
		connectTo.append(this->mysqlServerAddress);
		connectTo.append(":");
		stringstream ss1;
		ss1 << this->mysqlServerPort;
		connectTo.append(ss1.str());
		con = driver->connect(connectTo, this->userName, this->password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "root");

		con->setSchema(this->dataBaseName);

		pstmt = con->prepareStatement("SELECT * FROM vdc_requests");
		res = pstmt->executeQuery();

		while(res->next()){
			int idRequest = res->getInt("requestID");
			VDCRequestPartitioned* req = this->readVDCRequestPartitionedFromDataBase(idRequest);
			listRequests->push_back(req);
		}//End While on the number of request

		delete res;
		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() <<" )" << endl;
	}

	cout << endl;
	return listRequests;


}








/*
 * These are parts not used at this time
 *
 */

/*
			partition->reqID = res1->getInt("requestID");
			partition->request = NULL;
			partition->partitionID = res1->getInt("partitionID");
			partition->nodes = res1->getInt("numberNodes");
			partition->links = res1->getInt("numberLinks");
			partition->availability = res1->getDouble("availability");
			strcpy(partition->VDCName,res1->getString("vdcName").c_str());
			strcpy(partition->userName,res1->getString("userName").c_str());
			strcpy(partition->sessionName,res1->getString("sessionName").c_str());
			strcpy(partition->applicationName,res1->getString("applicationName").c_str());

			//Read the mapping of the partition
			pstmt = con->prepareStatement("SELECT * FROM mapping_partition WHERE requestID = (?) AND partitionID = (?) AND ((mappingEnd IS NULL) OR (mappingEnd = -1))");
			pstmt->setInt(1,idRequest);
			pstmt->setInt(2,idPartition);
			res = pstmt->executeQuery();
			if(res->next()){
				partition->mapping = new mapping_parition_to_datacenter;
				partition->mapping->datacenterID = res->getInt("datacenterID");
				partition->mapping->mapBegin = res->getInt("mappingBegin");
				partition->mapping->mapEnd = res->getInt("mappingEnd");
				partition->mapping->partitioID = idPartition;
				partition->mapping->requestID = idRequest;
				partition->mapping->costPerUnitOfTime = 0; //Chaneg ethe code if there is a cost per unit of time
			}else{
				partition->mapping = NULL;
			}


			//Read the VMs
			pstmt = con->prepareStatement("SELECT * FROM virtual_machines WHERE requestID = (?) and partitionID = (?) ORDER BY nodeID ASC ");
			pstmt->setInt(1,idRequest);
			pstmt->setInt(2,partition->partitionID);
			res2 = pstmt->executeQuery();
			int i = 0;
			while(res2->next()){
				partition->nodesID[i] = res2->getInt("nodeID");
				partition->cpu[i] = res2->getDouble("cpu");
				partition->mem[i] = res2->getDouble("memory");
				partition->migrationCost = res2->getDouble("migrationCost")*partition->nodes;
				partition->type[i] = res2->getInt("nodeType");
				partition->flavor[i] = res2->getInt("flavor");
				partition->disk[i] = res2->getDouble("disk");
				location_t* loc = new location_t;
				loc->xLocation = res2->getInt("xLocation");
				loc->yLocation = res2->getInt("yLocation");
				loc->maxDistance = res2->getInt("maxDistanceLocation");
				partition->nodeLocation[i] = loc;
				i++;
			}

			//Read the links
			pstmt = con->prepareStatement("SELECT * FROM virtual_links WHERE requestID = (?) AND partitionSource = (?) AND partitionDestination = (?)");
			pstmt->setInt(1,idRequest);
			pstmt->setInt(2,partition->partitionID);
			pstmt->setInt(3,partition->partitionID);
			res2 = pstmt->executeQuery();
			i = 0;
			while(res2->next()){
				link_t* l = new link_t;
				l->nodeFrom = res2->getInt("nodeSource");
				l->nodeTo = res2->getInt("nodeDestination");
				l->partitionFrom = res2->getInt("partitionSource");
				l->partitionTo = res2->getInt("partitionDestination");
				l->bw = res2->getDouble("bandwidth");
				l->delay = res2->getDouble("delay");

				l->mapping = new mapping_virtual_link_to_substrate_path;
				l->mapping->virtualLink = l;
				l->mapping->requestID = l->idRequest;
				l->mapping->substratePath = NULL;
				l->mapping->DCId = -1;
				l->mapping->requestID = idRequest;
				l->mapping->mapBegin = -1;
				l->mapping->mapEnd = -1;

				sql::PreparedStatement* pstmtMappings = con->prepareStatement("SELECT * FROM mapping_virtual_link WHERE requestID = (?) AND virtualNodeSource = (?) AND  virtualNodeDestination = (?)");
				pstmtMappings->setInt(1,idRequest);
				pstmtMappings->setInt(2,l->nodeFrom);
				pstmtMappings->setInt(3,l->nodeTo);
				sql::ResultSet *resMappings = pstmtMappings->executeQuery();

				if(resMappings->next()){
					l->mapping->mapBegin = resMappings->getDouble("mappingBegin");
					l->mapping->mapEnd = resMappings->getDouble("mappingEnd");
					if(!resMappings->isNull("datacenterID") && resMappings->getInt("datacenterID")>=0){
						//This link is mapped to a data center
						l->mapping->DCId = resMappings->getInt("datacenterID");
					}else{
						//This link has been mapped to a path in the backbone network
						vector<int> sources = vector<int>();
						vector<int> destinations = vector<int>();
						while(resMappings){
							sources.push_back(resMappings->getInt("physicalNodeSource"));
							destinations.push_back(resMappings->getInt("physicalNodeDestination"));
							resMappings->next();
						}

						//build the path
						path_t* path = new path_t;
						int k=0;
						bool found = false;
						int srcPath, indexSrcPath;
						while(!found && k<sources.size()){
							int v = sources[k];
							bool stop = true;

							if(find(destinations.begin(), destinations.end(),v) != destinations.end()){
								k++;
							}else{
								//This is the source of the path
								srcPath = v;
								indexSrcPath = k;
								found = true;
							}
						}
						//we have the source of the path, we build the path from the two tables
						int indSrc = indexSrcPath;
						path->len = sources.size()+1;
						int z = 0;
						path->link[z] = sources[indSrc];
						z++;
						while(z<path->len){
							path->link[z] = destinations[indSrc];
							indSrc = find(sources.begin(), sources.end(),destinations[indSrc]) - sources.begin();
							z++;
						}
						l->mapping->substratePath = path;
					}//End Else DCid == 0
				}//End ResMappings.next()

				delete pstmtMappings;
				delete resMappings;
				partition->link[i] = l;
				i++;
			}//end While res->next() //Number of links

			*/

