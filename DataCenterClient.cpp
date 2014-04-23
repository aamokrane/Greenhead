/*
 * DataCenterClient.cpp
 *
 *  Created on: Mar 21, 2014
 *      Author: ahmedamokrane
 */

#include "DataCenterClient.h"

DataCenterClient::DataCenterClient(char* remoteDataCenterIP, int remoteDataCenterPort) {
	int res = this->openConnexionWithRemoteDataCenter(remoteDataCenterIP, remoteDataCenterPort);
	if(res > 0){
		this->remoteDataCenterSocket = res;
	}else{
		cout << "FATAL ERROR opening socket at the client, this application should stop";
		this->remoteDataCenterSocket = res;
	}
}

DataCenterClient::~DataCenterClient() {

}

void DataCenterClient::callRemoteMethod(remote_call_t* remoteCallParam, remote_call_t* remoteCallResult) {
	int n;
	//n = write(this->remoteDataCenterSocket,&remoteCallParam,sizeof(remote_call_t));

	//FILE *f = fdopen(this->remoteDataCenterSocket, "w+");
	n = send(this->remoteDataCenterSocket,remoteCallParam,sizeof(remote_call_t), 0);
	//fflush(f);

	if (n < 0){
		cout << "ERROR writing to socket";
	}else{
		cout << "I sent "<< n <<" bytes to the server" <<endl;
	}

	//n = read(this->remoteDataCenterSocket,remoteCallResult,sizeof(remote_call_t));
	n = recv(this->remoteDataCenterSocket, remoteCallResult,sizeof(remote_call_t), 0);
	//n = recv(this->remoteDataCenterSocket,remoteCallResult,sizeof(int),0);

	if (n < 0){
		cout << "ERROR reading from socket";
		exit(0);
	}else{
		cout << "The Data Center answered, I received "<< n <<" bytes " <<endl;
	}
	//printf("%s\n",buffer);
}


int DataCenterClient::openConnexionWithRemoteDataCenter(char* ipServer, int serverPort) {
	//Connecting to the server
		int sockfd;
		//struct sockaddr_in serv_addr;
		struct hostent *server;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0){
			cout << "ERROR opening socket at the client";
			return sockfd;
		}
		//server = gethostbyname("localhost");
		in_addr_t data;
		data = inet_addr(ipServer);
		server = gethostbyaddr(&data, 4, AF_INET);

		if (server == NULL) {
			fprintf(stderr,"ERROR, no such host\n");
			exit(0);
		}
		struct sockaddr_in serv_addr;
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(serverPort);
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
			cout << "ERROR connecting";
			exit(0);
		}
		return sockfd;
}

void DataCenterClient::closeConnexion() {
	remote_call_t* remoteCallParam = new remote_call_t;
	remoteCallParam->method = STOP_SERVER;
	remote_call_t remoteResult;
	callRemoteMethod(remoteCallParam, &remoteResult);
	close(this->remoteDataCenterSocket);
}
