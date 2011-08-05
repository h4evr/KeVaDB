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

class FileHolder {
public:
	FileHolder(const char* key, File* file) {
		size_t len = strlen(key) + 1;
		this->key = (char*)malloc(len);
		memcpy(this->key, key, len);
		this->file = file;
		this->next = NULL;
	}
	
	~FileHolder() {
		if(key) {
			free(key);
			key = NULL;
		}
		
		if(file) {
			delete file;
			file = NULL;
		}
		
		if(next) {
			delete next;
			next = NULL;
		}
	}
	
	char* key;
	File* file;
	FileHolder* next;
};

class Database {
public:
	Database();
	~Database();
	
	void open(const char* filename);
	void close();
	
	void set(string key, string value);
	char* get(string key);
	
private:
	char* index_filename;
	
	void write_file_info(FILE* dst, FileHolder* f);
	FileHolder* read_file_info(FILE* f);
	
	static unsigned int hashfunc(const char* key);
	map<unsigned int, FileHolder*> files;
};

#endif