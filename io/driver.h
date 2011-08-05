#ifndef __IO_DRIVER_H__
#define __IO_DRIVER_H__

#include <cstdio>
#include <list>
using std::list;

#define PAGE_SIZE 24

class Driver {
public:
	Driver();
	~Driver();
	
	void open(const char* filename);
	void close();
	
	list<unsigned int> alloc(unsigned int length);
	bool readPage(unsigned int page_num, unsigned char* data, unsigned int* length);
	unsigned int writePage(unsigned int page_num, unsigned char* data, unsigned int length);
	void deletePage(unsigned int page_num);
	
	static Driver* getInstance();
protected:
	
	FILE* fp;
	
	inline unsigned int getPagePositionOnFile(unsigned int page_num) {
		return page_num * PAGE_SIZE;
	}
	
	unsigned char page_contents_buffer[PAGE_SIZE];
};

#endif