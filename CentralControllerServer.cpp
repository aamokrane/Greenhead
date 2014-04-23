/*
 * CenralControllerServer.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: ahmedamokrane
 */

#include "CentralControllerServer.h"

CentralControllerServer::CentralControllerServer(int port, central_controller_parameters_t* cnParameters) {
	this->underlyingCentralController = this->createUnderlyingCentralController(cnParameters);
	listeningPort = port;
	openedSocket = -1;
}

CentralControllerServer::~CentralControllerServer() {
	// TODO Auto-generated destructor stub
}


bool CentralControllerServer::startServer() {
	fd_set master, read_fds;    // master file descriptor list
	int fdmax;
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		cout << "ERROR opening socket at the server" << endl;
		return false;
	}
	cout << "Socket opened at the Server" << endl;

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(listeningPort);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		cout << "ERROR on binding in the server"<< endl;
		return false;
	}
	cout << "Binding done in the server" << endl;
	cout << "Start Listening" << endl;

	// add the listener filedescriptor to the master set
	FD_SET(sockfd, &master);

	fdmax = sockfd;
	//openedSocket = sockfd;

	//Start the threads for the
	this->underlyingCentralController->startThreadsToCollectStatistics(0);

	bool endServer = false;

	cout << "I am the server, I am listening on port "<< this->listeningPort <<", and the Socket is " << sockfd <<endl;
	cout << "I am the server, I am listening" <<endl;
	listen(sockfd,1);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	cout << "A Client is connected, its file descriptor is " << newsockfd <<endl;
	if (newsockfd < 0){
		cout << "ERROR on accept at the server level" <<endl;
	}

	FD_SET(newsockfd, &master);
	fdmax = fdmax>newsockfd?fdmax:newsockfd;

	while(!endServer){

		read_fds = master; // copy it
		cout << "We are about to perform the select" <<endl;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			fprintf(stderr, "Error in select\n");
			exit(4);
		}

		if(!FD_ISSET(newsockfd, &read_fds)){
			cout << "I was not expecting data on this file descriptor" <<endl;
			continue;
		}

		remote_call_t rcall;
		//bzero(&rcall,sizeof(request_t));

		cout << "I am expecting " <<sizeof(remote_call_t) << " bytes from the client" <<endl;
		//n = read(newsockfd,&rcall,sizeof(remote_call_t));

		sleep(2);
		memset(&rcall,0,sizeof(remote_call_t));
		unsigned int n_done = 0;

		while(n_done < sizeof(remote_call_t)){
			n = recv(newsockfd,&rcall + n_done, sizeof(remote_call_t)-n_done, 0); //MSG_OOB MSG_DONTWAIT 0to get same as read
			if(n <= 0) break;
			n_done += n;
			cout << "I finished reading "<< n <<" bytes from the client" <<endl;

		}


		/*
		int n_done = recv(newsockfd,&rcall , sizeof(remote_call_t), 0); //MSG_OOB MSG_DONTWAIT 0to get same as read
		cout << "I finished reading "<< n_done <<" bytes from the client" <<endl;
		*/

		remote_call_t* resultCall = new remote_call_t;
		if (n_done != sizeof(remote_call_t)){
			cout << "ERROR reading from socket in the server"<<endl;
		}else{
			cout << "I received "<< n_done <<" bytes from the client" <<endl;
			if(rcall.method == STOP_SERVER){
				//Stop the server
				resultCall->result = 1;
				n = send(newsockfd,resultCall,sizeof(remote_call_t), 0);
				if (n < 0){
					cout << "ERROR writing to socket" <<endl;
				}
				cout << "I received the order to stop serving clients" <<endl;
				FD_CLR(newsockfd, &master); // remove from master set
				close(newsockfd);

				this->processRemoteCall(rcall, resultCall);

				endServer = true;
			}else{
				this->processRemoteCall(rcall, resultCall);
				n = send(newsockfd,resultCall,sizeof(remote_call_t), 0);
				sleep(2);
				if (n < 0){
					cout << "ERROR writing to socket" <<endl;
				}
			}
		}
		//printf("Here is the message: %s\n", buffer);
		//n = write(newsockfd,resultCall,sizeof(remote_call_t));
	}//End While(!endServer)

	close(sockfd);
	return true;
}


double CentralControllerServer::processRemoteCall(remote_call_t remoteCallParam, remote_call_t* remoteCallResult) {

	request_savi_remote_call_t request;
	double result = -1;
	switch(remoteCallParam.method){

	case GET_INSTANTANEOUS_UTILIZATION:{
		cout << "This is a remote call to get the instantaneous data center utilization and parameters" <<endl;
		infrastructure_utilization_t* resStrcut = this->underlyingCentralController->getInstanteousUtilizationWholeInfrastructure(remoteCallParam.tBegin);
		remoteCallResult->parameter.infrastructureUtilization = *resStrcut;
		result = 1;
		remoteCallResult->result = result;
	}
	break;
	case EMBED_REQUEST:{
		request = remoteCallParam.parameter.request;
		request_t* requestLocal = new request_t;
		staticUsefulMethods::getDeserializedVDCRequest(&request, requestLocal);
		cout << "This is a remote call to embed the VDC request "<< requestLocal->reqID <<endl;
		//Embed the VDC request
		result = this->underlyingCentralController->PartitionAndEmbedVDCRequest(requestLocal,remoteCallParam.tBegin);
		remoteCallResult->result = result;
	}
	break;

	case REMOVE_REQUEST:{
		request = remoteCallParam.parameter.request;
		request_t* requestLocal = new request_t;
		staticUsefulMethods::getDeserializedVDCRequest(&request, requestLocal);
		cout << "This is a remote call to remove the VDC request "<< requestLocal->reqID <<endl;
		//Embed the VDC request
		double res1,res2;
		result = this->underlyingCentralController->removeRequestWithLinksBetweenPartitions(requestLocal,remoteCallParam.tBegin, &res1, &res2);
		remoteCallResult->result = result;
	}
	break;

	case EXPORT_RESULTS:{
		cout << "This is a remote call to embed the export the results at time "<< remoteCallParam.tBegin <<endl;
		//Embed the VDC request
		this->underlyingCentralController->writeResultsInFiles(remoteCallParam.parameter.simulation_parameters);
		remoteCallResult->result = result;
	}
	break;

	case STOP_SERVER:{
		cout << "This is a remote call to stop the server at time "<< remoteCallParam.tBegin <<endl;
		//Close the connexion with the data centers
		this->underlyingCentralController->stopThreadsCollectingStatistics();
		this->underlyingCentralController->writeResultsInFiles(remoteCallParam.parameter.simulation_parameters);
		this->underlyingCentralController->closeConnexionsWithRemoteDataCenters();
		remoteCallResult->result = 1;
	}
	break;

	case CLIENT_IS_LEAVING :{
		cout << "This is a remote call to say that the client is leaving "<< remoteCallParam.tBegin <<endl;
		remoteCallResult->result = 1;
	}
	break;

	default:{
		cout << "This is a remote call for an unknown method" << endl;
	}break;

	}
	remoteCallResult->result = result;

	return result;
}


CentralController* CentralControllerServer::createUnderlyingCentralController(central_controller_parameters_t* cnParameters){

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
