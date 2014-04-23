//============================================================================
// Name        : MysqlGreenheadTest.cpp
// Author      : Ahmped Amokrane
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "GreenheadCentralControllerMysqlConnector.h"

using namespace std;


request_t* generateTestRequest();

partition_t* createPartitionOutOfRequest(request_t* request);


int main10() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	GreenheadCentralControllerMysqlConnector connector = GreenheadCentralControllerMysqlConnector("127.0.0.1", 3306, "Ahmed", "Ahmed", "DataBaseForTest");
	connector.createDataBase();

	request_t* request = generateTestRequest();
	connector.writeVDCRequestToDataBase(request);

	request_t* requestRes = connector.readVDCRequestFromDataBase(request->reqID);

	partition_t* partition = createPartitionOutOfRequest(request);

	connector.writeVDCPartitionToDataBase(partition);

	partition->mapping->datacenterID = 10;
	partition->mapping->mapBegin = 5;

	connector.updateVDCPartitionMappingInTheDataBase(partition, 5);

	partition_t* partitionBack = connector.readVDCPartitionFromDataBase(partition->reqID, partition->partitionID);

	cout <<"Nothing was bad I think "<<endl;

}


request_t* generateTestRequest(){

	double revenuePerUnitOfCPU = 0.06;
	double revenuPerUnitOfMemory = 0.00001;
	double revenuPerUnitOfBandwidth = 0.001;

	request_t* p = new request_t;
	p->arrtime = 1.0;
	p->nodes = 8;
	p->duration = 100.0;
	p->revenue = 0;
	p->reqID = 1;
	//p->reqID = this->idRqsts;
	//this->idRqsts++;
	double r;
	for(int i=0; i<p->nodes; i++){
		r = ((double)rand())/RAND_MAX;
		int sizeVM = (int)(r*(double)3) + 1;
		p->cpu[i] = sizeVM;
		p->mem[i] = 0;
		p->revenue += p->cpu[i]*revenuePerUnitOfCPU + p->mem[i]*revenuPerUnitOfMemory;
		p->nodesID[i] = i;
		location_t* locat = new location_t;
		locat->maxDistance = -1;
		locat->xLocation = -1;
		locat->yLocation = -1;
		p->nodeLocation[i] = locat;
	}

	//Generate some location constrained nodes
	r = ((double)rand())/RAND_MAX;
	if(r < 0.0){
		p->nodeLocation[0]->maxDistance = 80;
		p->nodeLocation[0]->xLocation = 50;
		p->nodeLocation[0]->yLocation = 0;

		p->nodeLocation[4]->maxDistance = 80;
		p->nodeLocation[4]->xLocation = 50;
		p->nodeLocation[4]->yLocation = 100;
	}



	p->links = 0;
	link_t* l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 1;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 2;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 3;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 20;
	p->revenue += l->bw*revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 0;
	l->nodeTo = 4;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 4;
	l->nodeTo = 6;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 4;
	l->nodeTo = 7;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	l = new link_t;
	l->bw = 40;
	p->revenue += l->bw*revenuPerUnitOfBandwidth;
	l->delay = 0.5;
	l->idRequest = p->reqID;
	l->nodeFrom = 4;
	l->nodeTo = 5;
	l->partitionFrom = -1;
	l->partitionTo = -1;
	//l->numberOfMappings =0;
	l->mapping = NULL;
	p->link[p->links] = l;
	p->links = p->links + 1;

	return p;
}

partition_t* createPartitionOutOfRequest(request_t* request){
	partition_t* partition = new partition_t;

	partition->reqID = request->reqID;
	partition->partitionID = 1;
	partition->mapping = new mapping_parition_to_datacenter;
	partition->mapping->mapBegin = 1;
	partition->mapping->mapEnd = -1;
	partition->mapping->datacenterID = 1;
	partition->numberOfMigrations = 0;

	partition->nodes = request->nodes;
	for(int i=0; i<request->nodes; i++){
		partition->nodeLocation[i] = request->nodeLocation[i];
		partition->cpu[i] = request->cpu[i];
		partition->mem[i] = request->mem[i];
		partition->type[i] = request->type[i];
	}

	partition->links = request->links;
	for(int i=0; i<partition->links; i++){
		partition->link[i] = request->link[i];
	}



	return partition;
}


