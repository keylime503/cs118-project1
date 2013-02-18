#include "http-cache.h"
#include <string>
#include <unistd.h>
using namespace std;

bool Cache::existsInCache(string path, string host)
{
	string name = path + host;
	if(access(name.c_str(), F_OK) == 0)
	{
		//File exists
		return true;
	}
	else
		return false;
}

bool Cache::isExpired(string path, string host)
{
	
}