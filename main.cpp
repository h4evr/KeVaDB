#include <ctime>
#include <iostream>
using std::cout;
using std::endl;

#include "database.h"

double getCurrentTime() {
	struct timespec sp;
	clock_gettime(CLOCK_REALTIME, &sp);
	return (double)sp.tv_sec + (double)sp.tv_nsec / 1000000000.0;
}

void insert_test(Database* db) {
	char name[13];
	char value[14];
	double total_time, start_time;
	
	total_time = 0;
	
	for(unsigned int i = 0; i < 1000000; ++i) {
		sprintf(name, "key_%u", i);
		sprintf(value, "value_%u", i);
		start_time = getCurrentTime();
		db->set(string(name), string(value));
		total_time += getCurrentTime() - start_time;
	}
	
	cout << "Insert test: Took " << total_time << " seconds" << endl;
}

void read_test(Database* db) {
	char name[13];
	char exp_value[14];
	string value;
	double total_time, start_time;
	
	total_time = 0;
	
	for(unsigned int i = 0; i < 1000000; ++i) {
		sprintf(name, "key_%u", i);
		sprintf(exp_value, "value_%u", i);
		start_time = getCurrentTime();
		value = db->get(string(name));
		if(value.compare(exp_value) != 0) {
			cout << "Value different! Is " << value << ", expected " << exp_value << endl;
		}
		total_time += getCurrentTime() - start_time;
	}
	
	cout << "Read test: Took " << total_time << " seconds" << endl;
}

bool filterfunc(string key, string value) {
	return (key.find("_123") != string::npos);
}

void filter_test(Database* db) {
	double total_time, start_time = getCurrentTime();
		
	list<std::pair<string, string> > results = db->filter(filterfunc);
	list<std::pair<string, string> >::iterator it = results.begin();
	
	total_time = getCurrentTime() - start_time;
	
	for(; it != results.end(); ++it) {
		cout << it->first << endl;
	}
	
	cout << "Filter test: Took " << total_time << " seconds" << endl;
}

int main(int argc, char **argv) {
	Database db;
	
	db.open("teste");
	
	//insert_test(&db);
	//read_test(&db);
	filter_test(&db);
	
	db.close();
	
    return 0;
}
