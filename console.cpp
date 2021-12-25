#include "console.hpp"


boost::asio::io_context _context;
boost::system::error_code error;
class client 
	:public enable_shared_from_this<client>
{
public:
	client(const struct host _h, const shared_ptr<tcp::socket> socket, int _i): host_(_h), socket_(std::move(socket)), id(_i) {}
	
	void start() {
		// create template
		int permit = Request();
		if (permit == 0) 
			return;
		ReadFile();
		ReadPrompt();
	}
private:
	int Request() {
		uint8_t req[9];
		memset(req, 0, sizeof(req));
		req[0] = 4;
		req[1] = 1;
		req[2] = host_.port / 256;
		req[3] = host_.port % 256;
		tcp::resolver resolv(_context);
		tcp::resolver::results_type res = resolv.resolve(host_.hname, to_string(host_.port), error);
		for (auto it = res.begin(); it != res.end(); it++) {
			array<unsigned char, 4> ip = it->endpoint().address().to_v4().to_bytes();
			for (int i = 0; i < 4; i++) {
				req[4+i] = int(ip[i]);
			}	
		}
		socket_->write_some(boost::asio::buffer(req, 9));
		socket_->read_some(boost::asio::buffer(buf, MAXLEN));
		int stat = int(buf[1]);
		if (stat == 91) {
			boost::system::error_code ec;
			socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			return 0;
		}
		return 1;
	}
	void ReadPrompt () {
		auto self(shared_from_this());
		memset(buf, 0, sizeof(buf));
		socket_->async_read_some(boost::asio::buffer(buf, MAXLEN), 
				[this, self] (const boost::system::error_code &ec, size_t length)
				{
					string recv = string(buf);
					PrintRes(recv);
					if ( recv.find("%") == -1) {
						ReadPrompt();
					} else {
						Write();
					}
				
				});
	
	}
	void ReadFile() {
		string filename = "./test_case/" + host_.fname;
		file.open(filename);
	}

	string ReadLine() {
		string cmd;
		getline(file, cmd);
		int pos;
	    if ( (pos = cmd.find("\r")) != -1) {
		    cmd.erase(pos);
		}
		cmd += "\n";
		//cerr << cmd ;
		return cmd;
		 
	}

	void Write() {
		auto self(shared_from_this());
		string cmd = ReadLine();
		socket_->async_write_some(boost::asio::buffer(cmd.c_str(), cmd.size()),
				[this, self, cmd](const boost::system::error_code &ec, size_t length) {
					PrintCmd(cmd);
					if ( strncmp(cmd.c_str(), "exit", 4) != 0) {
						ReadPrompt();
					} else {
						exit();
					}
				});
	}
	void exit() {
		socket_->close();
	}
	void PrintCmd(string s) {
    	string script =  "<script>document.getElementById(\"s" + to_string(id) + "\").innerHTML += \"<b>" + escape(s) + "</b>\" ;</script>";
		cout << script;
	}
	
	void PrintRes(string s) {
    	string script =  "<script>document.getElementById(\"s" + to_string(id) + "\").innerHTML += \"" + escape(s) + "\" ;</script>";
		cout << script;
	}
	
	shared_ptr<tcp::socket> socket_;
	struct host host_;
	char buf[MAXLEN];
	char wbuf[MAXLEN];
	ifstream file;
	int id;
};
int main () {
	string query = string(getenv("QUERY_STRING"));
	vector<host> hosts = parse(query);


	tcp::resolver resolver_(_context);
	string sh = hosts[0].sh;
	string sp = hosts[0].sp;
	cout << "Content-Type: text/html\r\n\r\n";
	scope(hosts);
	tcp::resolver::results_type res = resolver_.resolve(sh, sp, error);

	for (auto it = res.begin(); it != res.end(); it++) {
		sh = it->endpoint().address().to_string();
	}

	for (int i = 0; i < 5; i++) {
		if (hosts[i].hname.empty())
			break;
		shared_ptr<tcp::socket> sock = make_shared<tcp::socket>(_context);
		tcp::endpoint ep(boost::asio::ip::address::from_string(sh), stoi(sp));
		sock->async_connect(ep, 
				[sock, hosts, i](const boost::system::error_code &err){
					if (!err) {
						make_shared<client>(hosts[i], std::move(sock), i)->start();
					}
				});	
	}	
	_context.run();
}
