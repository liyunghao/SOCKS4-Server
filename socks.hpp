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
#include <fstream>
#define MAXLEN 10500
using namespace std;

struct parseRes  {
	int VN, CD, port;
	string ip;
	parseRes(int v=0, int c=0, int p=0, string i=""):VN(v), CD(c), port(p), ip(i) {}
	void print() {
		cout << VN << ' ' << CD << ' ' << port << ' ' << ip << '\n';
	}	
};

parseRes parse (uint8_t *data_) {
	int VN = int(data_[0]);
	int CD = int(data_[1]);
	//cout << "CD = " << CD << '\n';
	int port = (int(data_[2]) << 8) + int(data_[3]);
	string ip;
	for (int i = 4; i < 8; i++) {
		ip += to_string(int(data_[i])) + '.';
	}
	ip.pop_back();
	parseRes res(VN, CD, port, ip);
	
	return res;
}

bool check(int cd, string ip) {
	ifstream f("socks.conf");
	if (!f) {
		perror("open error");
	}
	string line;
	bool deny = 1;
	if (cd == 1) {
		while (getline(f, line)) {
			if (line[7] == 'c') {
				auto fnd = line.find('*');
				int sz = fnd - 9;
				if (!strncmp(ip.c_str(), line.substr(9, sz).c_str(), sz)) {
					deny = 0;
					break;
				}
			}
		}
	} else {
		while (getline(f, line)) {
			if (line[7] == 'b') {
				auto fnd = line.find('*');
				int sz = fnd - 9;
				if (!strncmp(ip.c_str(), line.substr(9, sz).c_str(), sz)) {
					deny = 0;
					break;
				}
				
			}	
		}
	} 
	return !deny;
}
#endif
