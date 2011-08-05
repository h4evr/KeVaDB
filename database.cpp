#include "database.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "io/file.h"
#include "io/driver.h"

Database::Database() {
	this->index_filename = NULL;
}

Database::~Database() {
	this->close();
}

void Database::open(const char* filename) {
	this->close();
	
	// Get index filename
	size_t filename_len = strlen(filename);
	index_filename = new char[filename_len + 5];
	memcpy(index_filename, filename, filename_len);
	memcpy(&index_filename[filename_len], ".idx", 5);
	
	Driver* drv = Driver::getInstance();
	drv->open(filename);
	
	this->files.clear();
	
	FILE* idx = fopen(index_filename, "rb");
	if(idx) {
		unsigned int num_indexes = 0;
		unsigned int key_len = 0;
		char* key;
		
		fread((unsigned char*)&num_indexes, 1, sizeof(unsigned int), idx);
		
		while(num_indexes-- > 0) {
			fread((unsigned char*)&key_len, 1, sizeof(unsigned int), idx);
			key = (char*)malloc(key_len + 1);
			fread(key, 1, key_len, idx);
			key[key_len] = 0;
			this->files.insert(std::pair<string, File*>(string(key), read_file_info(idx)));
			free(key);
		}
		
		fclose(idx);
	}
}

void Database::close() {
	if(!index_filename)
		return;
	
	Driver* drv = Driver::getInstance();
	
	unsigned int num_indexes = this->files.size(), key_len;
	string key;
	std::map<string, File*>::iterator it = this->files.begin();
	FILE* idx = fopen(index_filename, "wb");
	
	fwrite((unsigned char*)&num_indexes, 1, sizeof(unsigned int), idx);
	
	for(; it != this->files.end(); ++it) {
		key = it->first;
		key_len = key.length();
		fwrite((unsigned char*)&key_len, 1, sizeof(unsigned int), idx);
		fwrite((unsigned char*)key.c_str(), 1, key_len, idx);
		write_file_info(idx, it->second);
	}
	
	fclose(idx);
	
	drv->close();
	
	this->files.clear();
	
	delete[] index_filename;
	index_filename = NULL;
}

void Database::set(string key, string value) {
	std::map<string, File*>::iterator it = this->files.find(key);
	
	File* tmp;
	
	if(it == this->files.end()) {
		tmp = new File();
		this->files[key] = tmp;
	} else {
		tmp = it->second;
	}
	
	tmp->write((unsigned char*)value.c_str(), value.length() + 1);
}

char* Database::get(string key) {
	std::map<string, File*>::iterator it = this->files.find(key);
	
	if(it != this->files.end()) {
		return (char*)(it->second->getContents());
	} else {
		return NULL;
	}
}

void Database::write_file_info(FILE* dst, File* f) {
	list<unsigned int> page_nums = f->getPageNumbers();
	unsigned int num_pages = page_nums.size();
	
	list<unsigned int>::iterator it = page_nums.begin();
	
	// Write number of pages of file
	fwrite((unsigned char*)&num_pages, 1, sizeof(unsigned int), dst);
	
	// Write page numbers
	for(; it != page_nums.end(); ++it) {
		num_pages = *it;
		fwrite((unsigned char*)&num_pages, 1, sizeof(unsigned int), dst);
	}
}

File* Database::read_file_info(FILE* f) {		
	list<unsigned int> page_nums;
	
	unsigned int num_pages = 0, page = 0;
		
	fread((unsigned char*)&num_pages, 1, sizeof(unsigned int), f);
	
	while(num_pages-- > 0) {
		fread((unsigned char*)&page, 1, sizeof(unsigned int), f);
		page_nums.push_back(page);
	}
	
	return new File(page_nums);
}

list<std::pair<string,string> > Database::filter(bool(*filterfunc)(string key, string value)) {
	list<std::pair<string,string> > result;
	File* tmp;
	string tmpkey, tmpvalue;

	std::map<string,File*>::iterator it = this->files.begin(), end = this->files.end();
	
	while(it != end) {
		tmpkey = it->first;
		tmpvalue = (char*)it->second->getContents();
		if(filterfunc(tmpkey, tmpvalue)) {
			result.push_back(std::pair<string, string>(tmpkey, tmpvalue));
		}
		++it;
	}
	
	return result;
}