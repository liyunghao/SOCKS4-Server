#ifndef CONSOLE
#define CONSOLE
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
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#define MAXLEN 16000
using boost::asio::ip::tcp;
using namespace std;


struct host {
	string hname;
	int port;
	string fname;
	string sh, sp;
	void print() {
		cerr << "host: " << hname << '\n';
		cerr << "port: " << port << '\n';
		cerr << "filename: " << fname << '\n';
	}
};

vector<host> parse(string input) {
	//string sep = "&";
	vector<string> res;
	vector<host> out;
	int start = 0;
	int end = input.find("&");
	while (end != -1) {
		res.push_back(input.substr(start, end - start));
		start = end + 1;
		end = input.find("&", start);
	}
	res.push_back(input.substr(start));
	int i;
	for (i = 0; i < 15;) {
		struct host tmp;
		tmp.hname = res[i++].substr(3);
		tmp.port = atoi(res[i++].substr(3).c_str());
		tmp.fname = res[i++].substr(3);
		out.push_back(tmp);
	}
	out[0].sh = res[i++].substr(3);
	out[0].sp = res[i++].substr(3);
	return out;
}

string escape(string s) {
	string res;
	for (auto x : s) {
		switch (x) {
			case '\n':
				res += "&NewLine;";
				break;
			case '&':
				res += "&amp;";
				break;
			case '>':
				res += "&gt;";
				break;
			case '<':
				res += "&lt;";
				break;
			default:
				res += x; 
		}
	}
	return res;
}

void scope(vector<host> hosts) {
	cout << "<!DOCTYPE html> \
				<html lang=\"en\"> \
				  <head> \
					<meta charset=\"UTF-8\" /> \
					<title>NP Project 3 Sample Console</title> \
					<link \
					  rel=\"stylesheet\" \
					  href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\" \
					  integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\" \
					  crossorigin=\"anonymous\" \
					/> \
					<link \
					  href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\" \
					  rel=\"stylesheet\" \
					/> \
					<link \
					  rel=\"icon\" \
					  type=\"image/png\" \
					  href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\" \
					/> \
					<style> \
					  * { \
						font-family: 'Source Code Pro', monospace; \
						font-size: 1rem !important; \
					  } \
					  body { \
						background-color: #212529; \
					  } \
					  pre { \
						color: #cccccc; \
					  } \
					  b { \
						color: #01b468; \
					  } \
					</style> \
				  </head> \
				  <body> \
					<table class=\"table table-dark table-bordered\"> \
					  <thead> \
						<tr> ";
		for (int i = 0; i < 5; i++) {
			if (hosts[i].hname.empty())
				break;
			cout << "<th scope=\"col\">" << hosts[i].hname << ":" << hosts[i].port << "</th> ";
		}
		cout << "</tr> \
					  </thead> \
					  <tbody> \
						<tr>";
		for (int i = 0; i < 5; i++) {
			if (hosts[i].hname.empty())
				break;
			cout << "<td><pre id=\"s" << i << "\" class=\"mb-0\"></pre></td>";
		}
		cout << "</tr> \
					  </tbody> \
					</table> \
				  </body> \
				</html> " ;

}



#endif
