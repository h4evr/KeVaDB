#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "io/file.h"

#include <string>
using std::string;

#include <map>
using std::map;

class Database {
public:
	Database();
	~Database();
	
	void open(const char* filename);
	void close();
	
	void set(string key, string value);
	char* get(string key);
	
	list<std::pair<string,string> > filter(bool(*filterfunc)(string key, string value));
private:
	char* index_filename;
	std::map<string, File*> files;
	
	void write_file_info(FILE* dst, File* f);
	File* read_file_info(FILE* f);
	
};

#endif