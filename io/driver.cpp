#include "driver.h"

#include <iostream>
#include <cstdio>
#include <cstring>

Driver* __instance = NULL;

Driver::Driver() {

}

Driver::~Driver() {
    this->close();
}

void Driver::open(const char* filename) {
	this->fp = fopen(filename, "r+b");
	
	// File doesn't exist.. Create it!
	if(this->fp == NULL) {
		this->fp = fopen(filename, "w+b");
	}
}

void Driver::close() {
	if(this->fp) {
		fclose(this->fp);
		this->fp = NULL;
	}
}

Driver* Driver::getInstance() {
	if(!__instance)
		__instance = new Driver();
	
	return __instance;
}

list<unsigned int> Driver::alloc(unsigned int length) {
	unsigned int actual_page_size = PAGE_SIZE - sizeof(unsigned int);
	unsigned int num_pages_to_create = length / actual_page_size;
	unsigned int num_existing_pages = 0, i = 0, page_len = length;
	list<unsigned int> page_list;
	
	// Correct number of pages
	if(num_pages_to_create * (PAGE_SIZE - sizeof(unsigned int)) < length)
		++num_pages_to_create;
	
	memset(&page_contents_buffer[0], 0, PAGE_SIZE);
	
	fseek(this->fp, 0, SEEK_END);
	num_existing_pages = (unsigned int)(ftell(this->fp) / PAGE_SIZE);
	
	page_len = std::min<unsigned int>(PAGE_SIZE, length);
	
	for(i = 0; i < num_pages_to_create; ++i) {
		fwrite((unsigned char*)&page_len, 1, sizeof(unsigned int), this->fp);
		fwrite(&page_contents_buffer[0], 1, actual_page_size, this->fp);
		page_list.push_back(num_existing_pages + i);
		
		page_len = std::min<unsigned int>(PAGE_SIZE, page_len - PAGE_SIZE);
	}
	
	return page_list;
}

bool Driver::readPage(unsigned int page_num, unsigned char* data, unsigned int* length) {
	if(!this->fp) 
		return false;
	
	if(fseek(this->fp, getPagePositionOnFile(page_num), SEEK_SET))
		return false;
	
	if(fread(&page_contents_buffer[0], sizeof(unsigned char), PAGE_SIZE, this->fp) != PAGE_SIZE) 
		return false;
			
	unsigned int len = *(unsigned int*)page_contents_buffer;

	if(len == (unsigned int)-1) 
		return false;
	
	memcpy(data, &page_contents_buffer[sizeof(unsigned int)], len);
	
	*length = len;
	
	return true;
}

unsigned int Driver::writePage(unsigned int page_num, unsigned char* data, unsigned int length) {
	if(!this->fp) 
		return 0;
	
	unsigned int len = std::min<unsigned int>(length, PAGE_SIZE - sizeof(unsigned int));
	
	if(fseek(this->fp, getPagePositionOnFile(page_num), SEEK_SET))
		return 0;
	
	fwrite((unsigned char*)&len, 1, sizeof(unsigned int), this->fp);
	fwrite(data, 1, len, this->fp);
	
	return len;
}

void Driver::deletePage(unsigned int page_num) {
	unsigned int tmp = -1;
	
	if(!this->fp) 
		return;
	
	if(fseek(this->fp, getPagePositionOnFile(page_num), SEEK_SET))
		return;
	
	fwrite((unsigned char*)&tmp, 1, sizeof(unsigned int), this->fp);
}
