/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include "http-headers.h"
#include "http-request.h"
#include "http-response.h"
using namespace std;

#define DEBUG_HTTP_PROXY 1

void debug(string msg)
{
	if(DEBUG_HTTP_PROXY)
	{
		cout << msg << endl;
	}
}

void error(string msg)
{
	cout << msg;
	exit(1);
}

int main (int argc, char *argv[])
{
 	// command line parsing
	if(argc > 1)
	{
		cout << "Too many arguments" << endl;
		return 1;
	}	

	int sockfd, newsockfd;
	uint16_t portnum = 14405;
	sockaddr_in servAddr, cliAddr;


	//Create Socket
	debug("Creating Socket");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		error("Error opening socket");

	//Create servAddr struct
	//Zero out struct
	//bzero((char *) &servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port =  htons(portnum);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	//Bind Socket
	debug("Binding Socket");
	if(bind(sockfd, (sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		error("Error binding socket");

	debug("Listening to Socket");
	if(listen(sockfd, 10) < 0)
		error("Error listening to socket");


	unsigned int cliLength = sizeof(cliAddr);

	int fd;
	while (fd > 0)
	{
		debug("Accepting Socket");
		newsockfd = accept(sockfd, (sockaddr *) &cliAddr, &cliLength);
		if(newsockfd < 0)
			error("Error on accept");
	} //Child Process will fall through here
  	
  	//Read from socket 
	char* buffer, iter;
	int bufferSize = 1024;
	buffer = new char[bufferSize];
	while(bufferSize == read(newsockfd, buffer, bufferSize))
	{
  		//More to read from Socket
		char * temp = new char[bufferSize * 2];
		memcpy(buffer, temp, bufferSize);

	}

	cout << "Message: " << buffer << endl;
	return 0;
}
