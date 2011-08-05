#ifndef __IO_FILE_H__
#define __IO_FILE_H__

#include "driver.h"
#include <list>
using std::list;

class File {
public:
	File();
	File(list<unsigned int> page_nums);
	~File();
	
	unsigned char* getContents();
	unsigned int getFileSize();
	
	void write(unsigned char* data, unsigned int length);
	
	list<unsigned int> getPageNumbers();
	
protected:
	void read();
	
	unsigned char* contents;
	unsigned int file_size;
	list<unsigned int> page_nums;
	bool is_new;
};

#endif