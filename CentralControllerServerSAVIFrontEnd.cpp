/*
 * CentralControllerServerSAVIFrontEnd.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: ahmedamokrane
 */

#include "CentralControllerServerSAVIFrontEnd.h"




CentralControllerServerSAVIFrontEnd::CentralControllerServerSAVIFrontEnd(int port, central_controller_parameters_t* cnParameters) {
	this->underlyingCentralController = this->createUnderlyingCentralController(cnParameters);
	listeningPort = port;
	openedSocket = -1;
}

CentralControllerServerSAVIFrontEnd::~CentralControllerServerSAVIFrontEnd() {

}

void CentralControllerServerSAVIFrontEnd::startServer() {

	//----------- Variables to use for the server
#if defined (WIN32)
	WSADATA WSAData;
	int error = WSAStartup(MAKEWORD(2,2), &WSAData);
#else
	int error = 0;
#endif

	char buffer[1000], buffer2[1000], response[1000], response2[3000];
	int sock_err;
	std::string status;
	//Socket and serveur context
	SOCKADDR_IN sin;
	SOCKET sock;
	socklen_t recsize = sizeof(sin);

	//Socket and client context
	SOCKADDR_IN csin;
	SOCKET csock;
	socklen_t crecsize = sizeof(csin);
	int idRequest = 1;
	idRequest = this->underlyingCentralController->getHighestRequestID() + 1;
	/*
	std::list<User> users;
	User user;
	 */

	//----------- Now we wait for requests
	if (!error) {
		// Socket creation
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock != INVALID_SOCKET) {
			cout << "Socket is now open, its number is " << sock << "\n";
			// Configuration
			sin.sin_addr.s_addr = htonl(INADDR_ANY); // automatique IP Adress
			sin.sin_family = AF_INET; // IP Protocol
			sin.sin_port = htons(this->listeningPort); // Port specification
			sock_err = bind(sock, (SOCKADDR*) &sin, recsize);

			if (sock_err != SOCKET_ERROR) {
				sock_err = listen(sock, 5);
				while (1) {
					if (sock_err != SOCKET_ERROR) {
						cout << "Waiting for a new command ...\n";
						csock = accept(sock, (SOCKADDR*) &csin, &crecsize);
						//New session has been opened let's create a User intance
						cout << "Command issued by "
								<< inet_ntoa(csin.sin_addr) << " , on port "
								<< htons(csin.sin_port) << "\n";

						recv(csock, buffer, sizeof(buffer), 0);
						//cout << buffer << endl;
						strcpy(buffer2, buffer);
						char * action = strtok(buffer2, "&,");
						char * session = strtok(NULL, "&,");
						/*
						if (!existsBySession(&users, session)) {
							user.SetId(csock);
							user.SetSession(session);
							//cout << "creation of a new user : session id = "<< session << "\n";
							users.push_back(user);
						} else {
							user = *(getUserBySession(&users, session));
						}
						 */

						//if (!user.GetRequests()->empty()) {user.GetRequestById(0)->DisplayRequest();}
						if (!strcmp(action, "newvdc")) {
							cout << "new VDC" << endl;

							request_t* rqst = NULL;
							rqst = staticUsefulMethods::loadVDCRequestFromString(buffer,idRequest);
							strcpy(rqst->sessionName, session);
							//Do the Embedding
							double cost = this->underlyingCentralController->PartitionAndEmbedVDCRequest(rqst, rqst->arrtime);

							if(cost < 0){ //The embedding went bad
								sprintf(response, "MAP_FAIL&");
								send(csock, response, sizeof(response), 0);
							} else {
								//The mapping was successful
								idRequest++;
								//sprintf(response, "MAP_DONE&%f&", request.GetMapping()->GetAvailability());
								sprintf(response, "MAP_DONE&%f&", cost);
								send(csock, response, sizeof(response), 0);
							}

						} else {
							if (!strcmp(action, "trackvdcs")) {
								cout << "track VDCs" << endl;

								sprintf(response, "TRACK_DONE");

								if (underlyingCentralController->vdcRequestsEmbedded->size() > 0) {
									//for testing-----------

									sprintf(response, "%s&%d", response, underlyingCentralController->vdcRequestsEmbedded->size());
									int idRequest;
									pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
									pthread_mutex_lock(&mutex);
									for (unsigned int i=0; i<underlyingCentralController->vdcRequestsEmbedded->size(); i++) {
										VDCRequestPartitioned* rqst = this->underlyingCentralController->vdcRequestsEmbedded->at(i);
										int reqNumber = rqst->idRequest;
										int nbNodes = rqst->originalRequest->nodes;
										int nbLinks = rqst->originalRequest->links;
										double duration =  rqst->originalRequest->duration;
										double avai = rqst->originalRequest->availability;
										int status = 1; //I do not keep track of the status of the VDC

										struct tm tstruct_tmp;
										char buf_tm_tmp[80];
										time_t arrTime = rqst->originalRequest->arrtime;

										time_t now = time(0);
										//tstruct_tmp = *localtime(&arrTime);
										strftime(buf_tm_tmp, sizeof(buf_tm_tmp),
												"%Y-%m-%d.%X",
												localtime(&arrTime));
										cout << buf_tm_tmp << endl;
										double remainTime;

										remainTime = rqst->originalRequest->duration; //TODO: Should be updated to a correct value
										sprintf(response,"%s&%d&%s&%d&%d&%lf&%lf&%d&%s&%lf",
												response, reqNumber,
												"Unknown User",
												nbNodes, nbLinks, duration,
												avai, status, buf_tm_tmp,
												remainTime);
									}

									pthread_mutex_unlock(&mutex);

								} else {
									sprintf(response, "%s&0", response);
								}
								sprintf(response, "%s&", response);
								send(csock, response, sizeof(response), 0);
							} else {
								if (!strcmp(action, "dltvdc")) {
									char * charId = strtok(NULL, "&,");
									int id = atoi(charId);
									cout << " asking to delete VDC with id " << id << endl;

									//Delete the VDC request by the central controller;
									double currentTime = 10000;
									double res = this->underlyingCentralController->removeRequest( id, currentTime);

									if (res > 0) {
										sprintf(response, "%s&%d", "RMV_DONE", id);
									} else {
										sprintf(response, "%s&%d", "RMV_FAILED", id);
									}
									send(csock, response, sizeof(response), 0);
								}else{

									if (!strcmp(action, "vdcdetails") || !strcmp(action, "lastvdcdetails") ) {
										//int id = -1;
										request_t* vdc = NULL;
										if(!strcmp(action, "vdcdetails")){
											char * charId = strtok(NULL, "&,");
											int id = atoi(charId);
											cout << " Asking to get the details of VDC with id " << id << endl;
											//get the request

											for(unsigned int i=0; i<this->underlyingCentralController->getVdcRequestsEmbedded()->size(); i++){
												if(this->underlyingCentralController->getVdcRequestsEmbedded()->at(i)->idRequest == id){
													vdc = this->underlyingCentralController->getVdcRequestsEmbedded()->at(i)->originalRequest;
													break;
												}
											}
										}else{
											//!strcmp(action, "lastvdcdetails"
											cout << " Asking to get the details of the last embedded VDC " << endl;
											if( this->underlyingCentralController->getVdcRequestsEmbedded()->size() > 0){
												vdc = this->underlyingCentralController->getVdcRequestsEmbedded()->at(this->underlyingCentralController->getVdcRequestsEmbedded()->size()-1)->originalRequest;
											}
										}

										//cout <<" user status : "<< user.GetRequestById(id)->GetStatus() << endl;


										if(vdc == NULL){
											sprintf(response, "%s", "FAILED_VDC_NOT_FOUND");
										}else{

											pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
											pthread_mutex_lock(&mutex);
											//for testing-----------cout<< " DISPLAY REQUEST @ BEG OF DETAILS VDC"<< endl;
											//user.GetRequestById(0)->DisplayRequest();
											//vdc->DisplayRequest();

											sprintf(response, "%s&%d&%s&%d&%d", "VDC_FOUND", vdc->reqID, vdc->userName, vdc->nodes, vdc->links);

											//sprintf(response, "%s&%d&%s&%d&%d", "VDC_FOUND", vdc->reqID, vdc->userName, vdc->nodes, vdc->links);

											//Write the VMs
											for(int i=0; i<vdc->nodes; i++){
												//sprintf(response,"%s&%d&%d&%s&%s&%s&%s", response, vdc->nodesID[i], 2, "RUNNING", "node", " ", " ");
												//Maybe send the datacenter that host the VM
												sprintf(response,"%s&%d&%d&%s&%s&%s&%s", response, vdc->nodesID[i], vdc->flavor[i], "RUNNING", "nobody", " ", " ");
											}

											//Write the links
											for(int i=0; i<vdc->links; i++){
												int id = i;
												int from_id = vdc->link[i]->nodeFrom;
												int to_id = vdc->link[i]->nodeTo;
												double bw = vdc->link[i]->bw;

												sprintf(response, "%s&%d&%d&%d&%lf&%s", response, id, from_id, to_id, bw, "RUNNING");
											}
											sprintf(response, "%s&%d&%d&%d&%lf&", response);
											//printf("%s\n", response);
											pthread_mutex_unlock(&mutex);
										}

										send(csock, response, sizeof(response),0);
									}else{

										//Process the other types of messages
										cout << " This is an unknown command from the front end, it is called " << action << endl;
										sprintf(response, "%s", "FAILED_UNKNOWN_ACTION");
									}
								}
							}
						}

					} else
						perror("listen error");
				}
			} else {
				perror("bind");
			}

			// Closing the client socket
			cout << "Closing the client socket\n";
			closesocket(csock);
			cout << "Closing the server socket\n";
			closesocket(sock);

		} else
			perror("socket");

#if defined (WIN32)
		WSACleanup();
#endif
	}
	//return fgetc(stdin);

}

CentralController* CentralControllerServerSAVIFrontEnd::createUnderlyingCentralController(central_controller_parameters_t* cnParameters){

	SubstrateInfrastructure* substrateInf = TopologyGenerator::generateTopology(cnParameters);

	substrateInf->buildAlternativePathsBetweenDataCenters(cnParameters->numberOfAlternativePaths);

	//Create the central controller
	CentralController* cn = new CentralController(cnParameters);
	cn->substrateInfrastructure = substrateInf;

	//Initialize the variables of performance evaluation
	cn->datacenterParameterOverTime = new vector< vector<datacenter_utilization_t*>* >();
	cn->wholeInfrastructureOverTime = new vector<infrastructure_utilization_t*>();
	cn->reoptimazationSummariesOverTime = new vector<reoptimization_summary_t*>();
	cn->SLAviolatioOverTime = new vector<sla_violation_summary_t*>();
	cn->centralControllerParameters = new central_controller_parameters_t;
	cn->centralControllerParameters = cnParameters;

	//Create the connection with the database;
	if(cnParameters->dataBaseServerAddress != NULL){
		cn->dataBaseConnector = new GreenheadCentralControllerMysqlConnector(string(cnParameters->dataBaseServerAddress), cnParameters->dataBaseServerPort, string(cnParameters->dataBaseUsername),string(cnParameters->dataBasePassword), string(cnParameters->dataBaseName));
	}else{
		cn->dataBaseConnector = NULL;
	}

	cn->stopReoptimization = false;
	cn->stopSLAViolationReporting = false;
	cn->stopUtilizationReporting = false;

	return cn;
}



