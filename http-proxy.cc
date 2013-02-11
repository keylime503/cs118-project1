/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
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

void process(int sockfd)
{
	//Read from socket
	int bufferSize = 1024;
	int dataSize = 0;
	int bytesRead = 0;
	int tempSize = 1024;
	char* temp = new char[tempSize];
	char* buffer = new char[bufferSize];
	while((bytesRead = read(sockfd, temp, tempSize)) > 0)
	{
		//Check if buffer is big enough
		if(bufferSize < dataSize + bytesRead)
		{
			char* bigBuffer = new char[bufferSize * 2];
			memcpy(buffer, bigBuffer, dataSize);
			bufferSize *= 2;
			free(buffer);
			buffer = bigBuffer;
		}
		//Add to buffer
		memcpy(buffer + dataSize, temp, bytesRead);
		dataSize += bytesRead;
	}
	cout << "Message: " << buffer << endl;
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

	while (1)
	{
		debug("Accepting Socket");
		newsockfd = accept(sockfd, (sockaddr *) &cliAddr, &cliLength);
		if(newsockfd < 0)
			error("Error on accept");
		int pid = fork();
		if(pid < 0)
		{
			error("Error on Fork");
		}
		else if (pid == 0) //Child
		{
			close(sockfd);
			process(newsockfd);
			exit(0);
		}
		else //Parent
		{
			close(newsockfd);
		}
	}

	// Create HTTP Request object
	HttpRequest req;
	try
	{
		ParseRequest(buffer, dataSize);
	}
	catch
	{
		debug("An exception occurred.");
	}
	
	string host = req.getHost();
	short port = req.getPort();
	string path = req.getPath();
	
	// Create buffer for HTTPRequest object
	size_t bufLength = req.GetTotalLength();
	char * buffer = new char[bufLength];
	req.FormatRequest(buffer);
	

	return 0;
}
