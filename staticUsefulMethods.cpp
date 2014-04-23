/*
 * staticusefulMethods.cpp
 *
 *  Created on: Mar 21, 2014
 *      Author: ahmedamokrane
 */

#include "staticUsefulMethods.h"

using namespace std;

staticUsefulMethods::staticUsefulMethods() {

}

staticUsefulMethods::~staticUsefulMethods() {
}

void staticUsefulMethods::getSerializableVDCRequest(request_t* request, request_savi_remote_call_t* result) {
	result->reqID = request->reqID;
	result->arrtime = request->arrtime;
	result->duration = request->duration;
	result->revenue = request->revenue;
	result->limitOfCarbonEmissionPerServicePeriodGuarantee = request->limitOfCarbonEmissionPerServicePeriodGuarantee;
	result->links = request->links;
	result->nodes = request->nodes;
	result->availability = request->availability;
	strcpy(result->VDCName,request->VDCName);
	strcpy(result->applicationName,request->applicationName);
	strcpy(result->sessionName,request->sessionName);
	strcpy(result->userName,request->userName);

	//Copy the links into
	for(int i=0; i<result->links; i++){
		result->link[i].bw = request->link[i]->bw;
		result->link[i].delay = request->link[i]->delay;
		result->link[i].idRequest = request->link[i]->idRequest;
		result->link[i].nodeFrom = request->link[i]->nodeFrom;
		result->link[i].nodeTo = request->link[i]->nodeTo;
		result->link[i].partitionFrom = request->link[i]->partitionFrom;
		result->link[i].partitionTo = request->link[i]->partitionTo;
	}
	for(int i=0; i<result->nodes; i++){
		result->nodesID[i] = request->nodesID[i];
		result->nodeLocation[i].id = request->nodeLocation[i]->id;
		result->nodeLocation[i].maxDistance = request->nodeLocation[i]->maxDistance;
		result->nodeLocation[i].xLocation = request->nodeLocation[i]->xLocation;
		result->nodeLocation[i].yLocation = request->nodeLocation[i]->yLocation;
		result->cpu[i] = request->cpu[i];
		result->mem[i] = request->mem[i];
		result->migrationCostOfVM[i] = request->migrationCostOfVM[i];
		result->type[i] = request->type[i];
		result->bwt[i] = request->bwt[i];
		result->flavor[i] = request->flavor[i];
		result->disk[i] = request->disk[i];
	}
}

void staticUsefulMethods::getSerializablePartition(partition_t* partition, partition_savi_remote_call_t* result) {
	result->reqID = partition->reqID;
	result->partitionID = partition->partitionID;
	result->numberOfMigrations = partition->numberOfMigrations;
	result->revenue = partition->revenue;
	result->links = partition->links;
	result->nodes = partition->nodes;
	result->availability = partition->availability;
	strcpy(result->VDCName,partition->VDCName);
	strcpy(result->applicationName,partition->applicationName);
	strcpy(result->sessionName,partition->sessionName);
	strcpy(result->userName,partition->userName);

	//Copy the links into the new structure
	for(int i=0; i<result->links; i++){
		result->link[i].bw = partition->link[i]->bw;
		result->link[i].delay = partition->link[i]->delay;
		result->link[i].idRequest = partition->link[i]->idRequest;
		result->link[i].nodeFrom = partition->link[i]->nodeFrom;
		result->link[i].nodeTo = partition->link[i]->nodeTo;
		result->link[i].partitionFrom = partition->link[i]->partitionFrom;
		result->link[i].partitionTo = partition->link[i]->partitionTo;
	}

	//Copy the nodes into the new structure
	for(int i=0; i<result->nodes; i++){
		result->nodesID[i] = partition->nodesID[i];
		result->nodeLocation[i].id = partition->nodeLocation[i]->id;
		result->nodeLocation[i].maxDistance = partition->nodeLocation[i]->maxDistance;
		result->nodeLocation[i].xLocation = partition->nodeLocation[i]->xLocation;
		result->nodeLocation[i].yLocation = partition->nodeLocation[i]->yLocation;
		result->cpu[i] = partition->cpu[i];
		result->mem[i] = partition->mem[i];
		result->type[i] = partition->type[i];
		result->bwt[i] = partition->bwt[i];
		result->flavor[i] = partition->flavor[i];
		result->disk[i] = partition->disk[i];
	}
}

void staticUsefulMethods::getDeserializedVDCRequest(request_savi_remote_call_t* request, request_t* result) {

	result->reqID = request->reqID;
	result->arrtime = request->arrtime;
	result->duration = request->duration;
	result->revenue = request->revenue;
	result->limitOfCarbonEmissionPerServicePeriodGuarantee = request->limitOfCarbonEmissionPerServicePeriodGuarantee;
	result->links = request->links;
	result->nodes = request->nodes;
	result->availability = request->availability;
	strcpy(result->VDCName,request->VDCName);
	strcpy(result->applicationName,request->applicationName);
	strcpy(result->sessionName,request->sessionName);
	strcpy(result->userName,request->userName);

	//Copy the links into
	for(int i=0; i<result->links; i++){
		result->link[i] = new link_t;
		result->link[i]->bw = request->link[i].bw;
		result->link[i]->delay = request->link[i].delay;
		result->link[i]->idRequest = request->link[i].idRequest;
		result->link[i]->nodeFrom = request->link[i].nodeFrom;
		result->link[i]->nodeTo = request->link[i].nodeTo;
		result->link[i]->partitionFrom = request->link[i].partitionFrom;
		result->link[i]->partitionTo = request->link[i].partitionTo;
		result->link[i]->mapping = NULL;
	}
	for(int i=0; i<result->nodes; i++){
		result->nodesID[i] = request->nodesID[i];
		result->nodeLocation[i] = new location_t;
		result->nodeLocation[i]->id = request->nodeLocation[i].id;
		result->nodeLocation[i]->maxDistance = request->nodeLocation[i].maxDistance;
		result->nodeLocation[i]->xLocation = request->nodeLocation[i].xLocation;
		result->nodeLocation[i]->yLocation = request->nodeLocation[i].yLocation;
		result->cpu[i] = request->cpu[i];
		result->mem[i] = request->mem[i];
		result->migrationCostOfVM[i] = request->migrationCostOfVM[i];
		result->type[i] = request->type[i];
		result->bwt[i] = request->bwt[i];
		result->flavor[i] = request->flavor[i];
		result->disk[i] = request->disk[i];
	}

}

void staticUsefulMethods::getDeserializedPartition(partition_savi_remote_call_t* partition, partition_t* result) {

	result->reqID = partition->reqID;
	result->partitionID = partition->partitionID;
	result->numberOfMigrations = partition->numberOfMigrations;
	result->revenue = partition->revenue;
	result->links = partition->links;
	result->nodes = partition->nodes;
	result->mapping = NULL;
	result->availability = partition->availability;
	strcpy(result->VDCName,partition->VDCName);
	strcpy(result->applicationName,partition->applicationName);
	strcpy(result->sessionName,partition->sessionName);
	strcpy(result->userName,partition->userName);


	//Copy the links into the new structure
	for(int i=0; i<result->links; i++){
		result->link[i] = new link_t;
		result->link[i]->bw = partition->link[i].bw;
		result->link[i]->delay = partition->link[i].delay;
		result->link[i]->idRequest = partition->link[i].idRequest;
		result->link[i]->nodeFrom = partition->link[i].nodeFrom;
		result->link[i]->nodeTo = partition->link[i].nodeTo;
		result->link[i]->partitionFrom = partition->link[i].partitionFrom;
		result->link[i]->partitionTo = partition->link[i].partitionTo;
		result->link[i]->mapping = NULL;
	}

	//Copy the nodes into the new structure
	for(int i=0; i<result->nodes; i++){
		result->nodesID[i] = partition->nodesID[i];
		result->nodeLocation[i] = new location_t;
		result->nodeLocation[i]->id = partition->nodeLocation[i].id;
		result->nodeLocation[i]->maxDistance = partition->nodeLocation[i].maxDistance;
		result->nodeLocation[i]->xLocation = partition->nodeLocation[i].xLocation;
		result->nodeLocation[i]->yLocation = partition->nodeLocation[i].yLocation;
		result->cpu[i] = partition->cpu[i];
		result->mem[i] = partition->mem[i];
		result->type[i] = partition->type[i];
		result->bwt[i] = partition->bwt[i];
		result->flavor[i] = partition->flavor[i];
		result->disk[i] = partition->disk[i];
	}
}

void staticUsefulMethods::getSerializableLink(link_t* link, link_savi_remote_call_t* result) {
	result->bw = link->bw;
	result->delay = link->delay;
	result->idRequest = link->idRequest;
	result->nodeFrom = link->nodeFrom;
	result->nodeTo = link->nodeTo;
	result->partitionFrom = link->partitionFrom;
	result->partitionTo = link->partitionTo;
}

void staticUsefulMethods::gerDeserializedLink(link_savi_remote_call_t* link, link_t* result) {
	result->bw = link->bw;
	result->delay = link->delay;
	result->idRequest = link->idRequest;
	result->nodeFrom = link->nodeFrom;
	result->nodeTo = link->nodeTo;
	result->partitionFrom = link->partitionFrom;
	result->partitionTo = link->partitionTo;
	result->mapping = new mapping_virtual_link_to_substrate_path;
	result->mapping->DCId = -1;
	result->mapping->substratePath = NULL;
	result->mapping->virtualLink = result;
}


/*

void staticUsefulMethods::convertRequestClassToRequestStructure(Request requestClass, request_t* requestStruct) {

	if(requestStruct != NULL){
		delete requestStruct;
	}
	requestStruct = new request_t;
	requestStruct->reqID = requestClass.GetRequestNumber();
	requestStruct->arrtime = requestClass.GetArrTime();
	requestStruct->duration = requestClass.GetDuration();
	requestStruct->revenue = requestClass.GetRevenue();

	requestStruct->links = requestClass.GetLinks()->size();

	//Write the VMs and their corresponding mappings
	std::list<Priority_group>*list = requestClass.GetGroups();
	int i = -1;
	for (std::list<Priority_group>::iterator it = list->begin(); it != list->end(); it++){
		Priority_group gr = *it;
		i++;
		for (std::list<Virtual_node>::iterator itVirtualNode = gr.GetNodes()->begin(); itVirtualNode != gr.GetNodes()->end(); itVirtualNode++){
			Virtual_node vm = *itVirtualNode;
			requestStruct->nodesID[i] = vm.GetId();
			requestStruct->nodeLocation[i] = new location_t;
			requestStruct->nodeLocation[i]->id = requestClass.GetRequestNumber();
			requestStruct->nodeLocation[i]->maxDistance =10000;
			requestStruct->nodeLocation[i]->xLocation = -1;
			requestStruct->nodeLocation[i]->yLocation = -1;
			requestStruct->cpu[i] = vm.GetCpu();
			requestStruct->mem[i] = vm.GetMemory();
			requestStruct->migrationCostOfVM[i] = 0;
			requestStruct->type[i] = vm.GetType();
			requestStruct->bwt[i] = 0;
		}
	}


	//Write the Links and their corresponding mappings
	i = -1;
	for (std::list<Path>::iterator it = requestClass.GetLinks()->begin(); it != requestClass.GetLinks()->end(); it++){
		i++;
		Path link = *it;
		link_t* l = new link_t;
		l->bw = link.GetBandwidth();
		l->delay = 10000;
		l->idRequest = requestClass.GetRequestNumber();
		l->mapping = NULL;
		l->nodeFrom = link.GetSourceNode_id();
		l->nodeTo = link.GetDestinationNode_id();
		l->partitionFrom = 0;
		l->partitionTo = 0;
		requestStruct->link[i] = l;
	}

}



void staticUsefulMethods::convertRequestStructureToRequestClass( request_t* requestStruct, Request* requestClass) {



}

 */

request_t* staticUsefulMethods::loadVDCRequestFromString(char* strRequest, int requestID) {

	request_t* requestStruct = new request_t;

	//variables
	int id, nbNodes, nbLinks, type, from, to, split, topo, step, nbGrps, grp, flavor;
	double bw, cpu, mem, disk, arrTime, duration, avai;
	string vdc_id;
	char * pch;
	char * session;
	char * user_name;
	char * application;

	//operations
	requestStruct->reqID = requestID;
	vdc_id =string("VDC_");
	stringstream ss1;
	ss1 << requestID;
	vdc_id.append(ss1.str());
	vdc_id.append("_");
	//std::size_t length = vdc_id.copy(requestStruct->VDCName,vdc_id.length(),0);
	//requestStruct->VDCName[length]='\0';

	requestStruct->revenue = 0;

	pch = strtok(strRequest, "&,"); //action = pch;


	pch = strtok(NULL, "&,");
	session = pch;
	strcpy(requestStruct->sessionName, session);

	pch = strtok(NULL, "&,");
	user_name = pch; //This is the VDC name
	strcpy(requestStruct->VDCName, user_name);

	//The user is not sent as part of the request, we put the VDCName as a user name
	strcpy(requestStruct->userName, user_name);


	pch = strtok(NULL, "&,");
	nbNodes = atoi(pch);
	requestStruct->nodes = nbNodes;

	pch = strtok(NULL, "&,");
	nbLinks = atoi(pch);
	requestStruct->links = nbLinks;

	pch = strtok(NULL, "&,");
	nbGrps = atoi(pch);

	pch = strtok(NULL, "&,");
	avai = atof(pch);
	requestStruct->availability = avai;

	pch = strtok(NULL, "&,");
	split = atoi(pch);
	requestStruct->split = LINK_UNSPLITTABLE;

	pch = strtok(NULL, "&,");
	arrTime = atof(pch);
	requestStruct->arrtime = arrTime/3600; //Change the time to seconds

	pch = strtok(NULL, "&,");
	duration = atof(pch);
	requestStruct->duration = duration;

	pch = strtok(NULL, "&,");
	topo = atoi(pch);
	requestStruct->topo = topo;

	pch = strtok(NULL, "&,");
	application = pch;
	strcpy(requestStruct->applicationName, application);

	//req->SetApplication(application);
	//req->SetStatus(STATE_RECEIVED);
	time_t now = time(0); //Make sure that the time is returned in hours
	requestStruct->arrtime = now/3600;

	requestStruct->limitOfCarbonEmissionPerServicePeriodGuarantee = 100;


	//Getting the nodes
	for(int i=0; i<nbNodes; i++){
		pch = strtok(NULL, "&,");
		id = atoi(pch);
		pch = strtok(NULL, "&,");
		flavor = atoi(pch);
		pch = strtok(NULL, "&,");
		cpu = atof(pch);
		pch = strtok(NULL, "&,");
		mem = atof(pch);
		pch = strtok(NULL, "&,");
		disk = atof(pch);
		pch = strtok(NULL, "&,");
		type = atoi(pch);
		pch = strtok(NULL, "&,");
		grp = atoi(pch);
		bw = 0;

		requestStruct->nodesID[i] = id;
		requestStruct->nodeLocation[i] = new location_t;
		requestStruct->nodeLocation[i]->id = requestStruct->reqID;
		requestStruct->nodeLocation[i]->maxDistance = 50;
		requestStruct->nodeLocation[i]->xLocation = 0;
		requestStruct->nodeLocation[i]->yLocation = 0;
		requestStruct->cpu[i] = cpu;
		requestStruct->mem[i] = mem;
		requestStruct->flavor[i] = flavor;
		requestStruct->disk[i] = disk;
		requestStruct->migrationCostOfVM[i] = 0; //This is not provided by the portal
		requestStruct->type[i] = type;
		requestStruct->bwt[i] = 0;
		requestStruct->revenue += 0.002315 * cpu * 4 + mem * 8 * 0.00335;
	}


	//Getting the links
	for (int i = 0; i < nbLinks; i++) {
		pch = strtok(NULL, "&,");
		id = atoi(pch);
		pch = strtok(NULL, "&,");
		from = atoi(pch);
		pch = strtok(NULL, "&,");
		to = atoi(pch);
		pch = strtok(NULL, "&,");
		bw = atof(pch);
		//for testing-----------printf("%d %d %d %lf\n", id, from, to, bw);
		link_t* l = new link_t;
		l->bw = bw;
		l->delay = 10000;
		l->idRequest = requestStruct->reqID;
		l->mapping = NULL;
		l->nodeFrom = from;
		l->nodeTo = to;
		l->partitionFrom = 0;
		l->partitionTo = 0;
		requestStruct->link[i] = l;
		requestStruct->revenue += bw / 100;
	}

	return requestStruct;

}
