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
		unsigned int key = 0;
		
		fread((unsigned char*)&num_indexes, 1, sizeof(unsigned int), idx);
		
		while(num_indexes-- > 0) {
			fread((unsigned char*)&key, 1, sizeof(unsigned int), idx);
			this->files.insert(std::pair<unsigned int, FileHolder*>(key, read_file_info(idx)));
		}
		
		fclose(idx);
	}
}

void Database::close() {
	if(!index_filename)
		return;
	
	Driver* drv = Driver::getInstance();
	
	unsigned int num_indexes = this->files.size(), key = 0;	
	std::map<unsigned int, FileHolder*>::iterator it = this->files.begin();
	FILE* idx = fopen(index_filename, "wb");
	
	fwrite((unsigned char*)&num_indexes, 1, sizeof(unsigned int), idx);
	
	for(; it != this->files.end(); ++it) {
		key = it->first;
		fwrite((unsigned char*)&key, 1, sizeof(unsigned int), idx);
		write_file_info(idx, it->second);
	}
	
	fclose(idx);
	
	drv->close();
	
	this->files.clear();
	
	delete[] index_filename;
	index_filename = NULL;
}

void Database::set(string key, string value) {
	unsigned int hash_key = Database::hashfunc(key.c_str());
	std::map<unsigned int, FileHolder*>::iterator it = this->files.find(hash_key);
	
	FileHolder* tmp;
	
	if(it == this->files.end()) {
		tmp = new FileHolder(key.c_str(), new File());
		this->files[hash_key] = tmp;
	} else {
		tmp = it->second;
		while(tmp) {
			if(strcmp(tmp->key, key.c_str()) == 0)
				break;
			else
				tmp = tmp->next;
		}
		
		if(!tmp) {
			tmp = new FileHolder(key.c_str(), new File());
			tmp->next = it->second;
			this->files[hash_key] = tmp;
		}
	}
	
	tmp->file->write((unsigned char*)value.c_str(), value.length() + 1);
}

char* Database::get(string key) {
	FileHolder* tmp = this->files[Database::hashfunc(key.c_str())];
	if(tmp) {
		if(!tmp->next)
			return (char*)tmp->file->getContents();
		
		while(tmp) {
			if(strcmp(tmp->key, key.c_str()) == 0)
				return (char*)tmp->file->getContents();
			else
				tmp = tmp->next;
		}
		return NULL;
	}
	else
		return NULL;
}

void Database::write_file_info(FILE* dst, FileHolder* f) {
	list<unsigned int> page_nums;
	unsigned int num_pages;
	unsigned int length_key;
	
	while(f) {
		page_nums = f->file->getPageNumbers();
		num_pages = page_nums.size();
		length_key = strlen(f->key);
		
		list<unsigned int>::iterator it = page_nums.begin();
		
		// Write key
		fwrite((unsigned char*)&length_key, 1, sizeof(unsigned int), dst);
		fwrite((unsigned char*)(f->key), 1, length_key, dst);
		
		// Write number of pages of file
		fwrite((unsigned char*)&num_pages, 1, sizeof(unsigned int), dst);
		
		// Write page numbers
		for(; it != page_nums.end(); ++it) {
			num_pages = *it;
			fwrite((unsigned char*)&num_pages, 1, sizeof(unsigned int), dst);
		}
		
		if(f->next) {
			fwrite("0", 1, 1, dst);
		} else {
			fwrite("1", 1, 1, dst);
		}
		
		// Write next file
		f = f->next;
	}
}

FileHolder* Database::read_file_info(FILE* f) {
	FileHolder* orig = NULL, *nextfile = NULL;
	
	char* key;
	
	list<unsigned int> page_nums;
	
	unsigned int num_pages = 0, page = 0, length_key;
	char is_end = 0;
	
	fread((unsigned char*)&length_key, 1, sizeof(unsigned int), f);
	
	key = (char*)malloc(length_key + 1);
	fread(key, 1, length_key, f);
	key[length_key] = 0;
	
	fread((unsigned char*)&num_pages, 1, sizeof(unsigned int), f);
	
	while(num_pages-- > 0) {
		fread((unsigned char*)&page, 1, sizeof(unsigned int), f);
		page_nums.push_back(page);
	}
	
	orig = new FileHolder(key, new File(page_nums));
	free(key);
	
	fread(&is_end, 1, 1, f);
	
	nextfile = orig;
	
	while(is_end == '0') {
		page_nums.clear();
		
		fread((unsigned char*)&length_key, 1, sizeof(unsigned int), f);
		
		key = (char*)malloc(length_key + 1);
		fread(key, 1, length_key, f);
		key[length_key] = 0;
		
		fread((unsigned char*)&num_pages, 1, sizeof(unsigned int), f);
		
		while(num_pages-- > 0) {
			fread((unsigned char*)&page, 1, sizeof(unsigned int), f);
			page_nums.push_back(page);
		}
		
		nextfile->next = new FileHolder(key, new File(page_nums));
		nextfile = nextfile->next;
		
		free(key);
		
		fread(&is_end, 1, 1, f);
	}
	
	return orig;
}

list<std::pair<string,string> > Database::filter(bool(*filterfunc)(string key, string value)) {
	list<std::pair<string,string> > result;
	FileHolder* tmp;
	string tmpkey, tmpvalue;
	std::map<unsigned int, FileHolder*>::iterator it = this->files.begin();
	
	for(; it != this->files.end(); ++it) {
		tmp = it->second;
		while(tmp) {
			tmpkey = tmp->key;
			tmpvalue = (char*)tmp->file->getContents();
			
			if(filterfunc(tmpkey, tmpvalue)) {
				result.push_back(std::pair<string, string>(tmpkey, tmpvalue));
			}
			
			tmp = tmp->next;
		}
	}
	
	return result;
}

/*MurMur v2*/
unsigned int Database::hashfunc(const char* key) {
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.
	
	int len = strlen(key);
	const unsigned int seed = 123456;
	const unsigned int m = 0x5bd1e995;
	const int r = 24;
	
	// Initialize the hash to a 'random' value
	
	unsigned int h = seed ^ len;
	
	// Mix 4 bytes at a time into the hash
	
	const unsigned char * data = (const unsigned char *)key;
	
	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;
		
		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h *= m; 
		h ^= k;
		
		data += 4;
		len -= 4;
	}
	
	// Handle the last few bytes of the input array
	
	switch(len)
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
		h *= m;
	};
	
	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	
	return h;
}