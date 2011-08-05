#include "file.h"

#include <cstdlib>
#include <cstring>

File::File() {
	this->is_new = true;
	this->contents = NULL;
	this->file_size = 0;
}

File::File(list<unsigned int> page_nums) {
	this->page_nums.clear();
	list<unsigned int>::iterator it = page_nums.begin();
	for(; it != page_nums.end(); ++it) {
		this->page_nums.push_back(*it);
	}
	
	this->is_new = false;
	this->contents = NULL;
	this->file_size = 0;
}

File::~File() {
	if(this->contents) {
		free(this->contents);
		this->contents = NULL;
		this->file_size = 0;
	}
}

unsigned char* File::getContents() {
	if(!this->contents)
		this->read();
	return this->contents;
}

unsigned int File::getFileSize() {
	if(!this->contents)
		this->read();
	return this->file_size;
}

void File::read() {
	if(this->is_new) {
		this->contents = NULL;
		this->file_size = 0;
		return;
	}
	
	Driver* drv = Driver::getInstance();
	
	this->file_size = 0;
	this->contents = (unsigned char*)malloc((PAGE_SIZE - sizeof(unsigned int)) * page_nums.size());
	
	unsigned int tmpLen = 0;
	list<unsigned int>::iterator it = page_nums.begin();
	
	for(; it != page_nums.end(); ++it) {
		if(!drv->readPage(*it, &this->contents[this->file_size], &tmpLen)) {
			this->file_size = 0;
			this->contents = NULL;
			break;
		}
		this->file_size += tmpLen;
	}
}

void File::write(unsigned char* data, unsigned int length) {
	unsigned int actual_page_size = PAGE_SIZE - sizeof(unsigned int);
	unsigned int allocated_space = this->page_nums.size() * actual_page_size;
	
	Driver* drv = Driver::getInstance();
	
	if(allocated_space < length) {
		// Need to allocate more space!
		list<unsigned int> new_pages = drv->alloc(length - allocated_space);
		this->page_nums.merge(new_pages);
	}
	
	unsigned int bytes_to_go = length;
	unsigned int piece_len = std::min<unsigned int>(length, actual_page_size);
	list<unsigned int>::iterator it = page_nums.begin();
	
	if(this->contents) {
		free(this->contents);
	}
	
	this->contents = (unsigned char*)malloc(length * sizeof(unsigned char));
	memcpy(this->contents, data, length * sizeof(unsigned char));
	this->file_size = length;
	
	while(bytes_to_go > 0) {
		piece_len = std::min<unsigned int>(bytes_to_go, actual_page_size);
		
		drv->writePage(*it, data, piece_len);
		
		data += piece_len;
		bytes_to_go -= piece_len;
		++it;
	}
	
}

list<unsigned int> File::getPageNumbers() {
	return this->page_nums;
}