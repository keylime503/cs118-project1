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
  	
  	char buffer[1024];
  	int sockfd, newsockfd;
  	uint16_t portnum = 14805;
	sockaddr_in servAddr, cliAddr;

	//Create Socket
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
	if(bind(sockfd, (sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		error("Error binding socket");

	if(listen(sockfd, 10) < 0)
		error("Error listening to socket");
  	
  	unsigned int cliLength = sizeof(cliAddr);
  	newsockfd = accept(sockfd, (sockaddr *) &cliAddr, &cliLength);
  	if(newsockfd < 0)
  		error("Error on accept");

  	read(newsockfd, buffer, 1024);
  	cout << "Message: " << buffer << endl;
  	return 0;
}
