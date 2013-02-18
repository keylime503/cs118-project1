/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctime>
#include <errno.h>


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

//Reads all of the data on a socket, and puts it in a char buffer.
//Returns pointer to char buffer, sets buffSize to the allocated size
//and dataSize to the amount of data. Caller must free the buffer.
char * readResponse(int sockfd, int& buffSize, int& dataSize)
{
	//debug("In readResponse()");

	//TODO: Check for overflow problems

	dataSize = 0;
	buffSize = 1024;
	int bytesRead = 0;
	int tempSize = 1024;
	char* temp = new char[tempSize];
	char* buffer = new char[buffSize];
	bzero(temp, tempSize);
	bzero(buffer, buffSize);
	
	while(1)
	{
		//debug("Top of BytesRead loop");

		fd_set rfds;
		struct timeval tv;
		int retval;

		/* Watch socket to see when it has data to be read */
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);

		/* Wait up to two seconds */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		retval = select(sockfd+1, &rfds, NULL, NULL, &tv);

		//cout << "$$$ retval: " << retval << endl;

		if (retval == -1)
		{	
			error("select() ERROR! errno: " + errno);
		}
		else if (retval == 0)
		{
			debug("Select timed out. Nothing to read from socket.");
			break;
		}

		//cout << "tempSize: " << tempSize << endl;

		bytesRead = recv(sockfd, temp, tempSize, 0);

		if (bytesRead < 0)
			error("Error reading from socket");
		else if (bytesRead == 0)
		{
			debug("ZERO bytes read");
			sleep(5);
		}

		//cout << "Bytes read: " << bytesRead << endl;

		cout << temp << endl;

		//Check if buffer is big enough
		if(buffSize < dataSize + bytesRead)
		{
			debug("*** resizing buffer ***");
			char* bigBuffer = new char[buffSize * 2];
			memcpy(buffer, bigBuffer, dataSize);
			buffSize *= 2;
			free(buffer);
			buffer = bigBuffer;
		}
		//Add to buffer
		memcpy(buffer + dataSize, temp, bytesRead);
		dataSize += bytesRead;

		/* Check for content-length header */
		HttpHeaders hdr;

		try
		{
			hdr.ParseHeaders(buffer, dataSize);
		}
		catch(ParseException e)
		{
			//cout << "In catch block!!!" << endl;
			//debug(e.what());
			//continue;
		}
		
		/* Get content length */
		int headerLength = hdr.GetTotalLength();
		if (headerLength == 0)
			error("Header length of 0???");

		string cl = hdr.FindHeader("Content-Length"); // IN 8-BYTE OCTETS!!!
		if (cl == "")
			debug("Cannot find contentLength header");

		int contentLength = 8*(atoi(cl.c_str())); 
		int totalLength = headerLength + contentLength;
		cout << "contentLength: " << contentLength << endl;
		cout << "headerLength: " << headerLength << endl;
		cout << "totalLength: " << totalLength << endl;

		if (dataSize < totalLength) // More to read!!!
			continue;
		else
			break;
	}
	debug("End of readResponse()");
	free(temp);
	return buffer;
}

void process(int clientSockfd)
{
	/*time_t startTime, endTime;
	time(&startTime);
	double timeElapsed = 0;*/
	bool persistentConnection = true;

	while(persistentConnection) // TODO: infinite for now!!!
	{
		debug("In process loop");

		// Timer
		/*time(&endTime);
		timeElapsed = difftime(endTime, startTime);
		cout << "timeElapsed: " << timeElapsed << endl;
		if (timeElapsed > 5.0)
			break;*/

		//Read from socket
		int buffSize = 0; 
		int dataSize = 0;
		char * buffer = readResponse(clientSockfd, buffSize, dataSize);

		//cout << "Buffer w/ request from client: " << buffer << endl;

		//debug("After readResponse()");

		// Create HTTP Request
		HttpRequest req;
		try
		{
			req.ParseRequest(buffer, dataSize);
			//req.ParseHeaders(buffer, dataSize);
		}
		catch(ParseException e)
		{
			cout << "In catch block!!!" << endl;
			debug(e.what());
		}

		free(buffer);

		// Check for persistent connection
		string connHeader = req.FindHeader("Connection");
		cout << "--- conn value: " << connHeader << endl;
		if (connHeader == "close")
		{
			persistentConnection = false;
			debug("close connection specified");
		}

		//Check cache for item
		//Open file for item
		//If item exists in cache
			//If item is expired
				//Append to request if-modified-since-expire-time
				//Send request to server
				//If response is 300
					//reset expire time
				//Else
					//replace cached item 
		//Else
		//Send request to server, store in cache
		//Send item back to client

		// HTTP Request good, send to server
		string path = req.GetPath();
		string host = req.GetHost();
		unsigned short port = req.GetPort();
		
		//cout << "Path: " << path << endl;
		//cout << "Host: " << host << endl;
		//cout << "Port: " << port << endl;

		// Create buffer for HTTPRequest object
		size_t bufLength = req.GetTotalLength();
		buffer = new char[bufLength];
		req.FormatRequest(buffer);

		// Send Request to server
		addrinfo hints;
		addrinfo * result, * rp;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;
		hints.ai_protocol = 0;

		//Convert unsigned short port to c string
		char portstr[10];
		sprintf(portstr, "%u", port);

		int s = getaddrinfo(host.c_str(), portstr, &hints, &result);

		if(s != 0)
		{
			error("No Such server");
		}
		int servSockfd;	
		for (rp = result; rp != NULL; rp = rp->ai_next) {
	        servSockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	        if (servSockfd == -1)
	            continue;

	    	if (connect(servSockfd, rp->ai_addr, rp->ai_addrlen) != -1)
	            break;                  /* Success */

	       close(servSockfd);
	    }

	    if (rp == NULL) {               /* No address succeeded */
	        error("Could not connect");
	    }

	    free(result);
		debug("Connected to server. Attempting to write to server socket.");

		cout << "*** buff to server: " << buffer << "***" << endl;
		cout << "servSockfd: " << servSockfd << endl;
		cout << "bufLength: " << bufLength << endl;

		fd_set wfds;
		struct timeval tv;
		int retval;

		/* Watch socket to see when it has data to be read */
		FD_ZERO(&wfds);
		FD_SET(servSockfd, &wfds);

		/* Wait up to two seconds */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		retval = select(servSockfd+1, NULL, &wfds, NULL, &tv);

		if (retval == -1)
		{	
			error("select() ERROR! errno: " + errno);
		}
		else if (retval == 0)
		{
			debug("Select timed out. Unable to write to socket.");
			break;
		}

	  	int bytesWritten = write(servSockfd, buffer, bufLength);

	  	cout << "bytesWritten: " << bytesWritten << endl;

	  	if(bytesWritten < 0)
	  	{
	  		error("Error writing to servSockfd");
	  	}

		//debug("bytesWritten to server. Before free now.");	

		free(buffer);
		sleep(2);
		usleep(500000);
		// Listening to response from server
		debug("Listening for response from server");
		buffer = readResponse(servSockfd, buffSize, dataSize);

		cout << "Buffer w/ response for client: " << buffer << endl;

		//Send back to client
		debug("Sending response to client");

		/* Watch socket to see when it has data to be read */
		FD_ZERO(&wfds);
		FD_SET(clientSockfd, &wfds);

		/* Wait up to two seconds */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		retval = select(clientSockfd+1, NULL, &wfds, NULL, &tv);

		if (retval == -1)
		{	
			error("select() ERROR! errno: " + errno);
		}
		else if (retval == 0)
		{
			debug("Select timed out. Unable to write to socket.");
			break;
		}

		bytesWritten = write(clientSockfd, buffer, dataSize);
		if(bytesWritten < 0)
			error("Error writing to clientSockfd");

		free(buffer);
		debug("End of request");
		debug("*********************************************");
		sleep(5);
	}

	debug("Closing connection");
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
	uint16_t portnum = 14459;
	sockaddr_in servAddr, cliAddr;


	//Create Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		error("Error opening socket");

	//Create servAddr struct
	//Zero out struct
	//bzero((char *) &servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(portnum);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	//Bind Socket
	if(bind(sockfd, (sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		error("Error binding socket");

	if(listen(sockfd, 10) < 0)
		error("Error listening to socket");


	unsigned int cliLength = sizeof(cliAddr);

	while (1)
	{
		//debug("Top of while loop");
		newsockfd = accept(sockfd, (sockaddr *) &cliAddr, &cliLength);
		debug("Accepted Socket");
		if(newsockfd < 0)
			error("Error on accept");
		//debug("Forking");
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

	return 0;
}
