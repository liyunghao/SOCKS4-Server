#ifndef SERVER
#define SERVER
#include <boost/asio.hpp>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <memory>
#include <utility>
#include <cstdlib>
#include <string.h>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <signal.h>
#define MAXLEN 10500
using namespace std;

struct parseRes {
	string method, url, proto, query, host, target;
	
	void print() {
		cerr << "Method: " << method << '\n';
		cerr << "Url: " << url << '\n';
		cerr << "Protocol: " << proto << '\n';
		cerr << "Query: " << query << '\n';
		cerr << "Host: " << host << '\n';
		cerr << "Target: " << target << '\n';
	}

};

parseRes parse(string input) {
	parseRes r;
	vector<string> line;
	int pre = 0;
	for (int i = 0; i < input.size(); i++) {
		if (input[i] == '\r' && input[i+1] == '\n' && pre != i) {
			line.push_back(input.substr(pre, i-pre));
			i += 2;
			pre = i;
		} 
	}
	
	//first line is always start line with get

	stringstream ss(line[0]);
	ss >> r.method;
	ss >> r.url;
	ss >> r.proto;

	size_t fnd = r.url.find("?");	
	r.target = r.url;
	if (fnd != -1) {
		r.target = r.url.substr(0, fnd);
		r.query = r.url.substr(fnd+1);
	}
	
	for (int i = 1; i < line.size(); i++) {
		if (!strncmp(line[i].c_str(), "Host:", 5)) {
			r.host = line[i].substr(6);		
		}
	}
	
	return r;
}

#endif
