/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>

using namespace std;

int main (int argc, char *argv[])
{
	// command line parsing
	if(argc > 1)
	{
		cout << "Too many arguments" << endl;
		return 1;
	}
	cout << "Hi. Im the client" << endl;
  
	return 0;
}

// Can you see me?
