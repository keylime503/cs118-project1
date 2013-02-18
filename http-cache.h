#ifndef _HTTP_CACHE_H_
#define _HTTP_CACHE_H_

#include <string>

class Cache 
{
	public:
		bool existsInCache(string path, string host);
		bool isExpired(string path, string host);
};

#endif