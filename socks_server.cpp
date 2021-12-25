#include "socks.hpp"


using boost::asio::ip::tcp;
using namespace std;

boost::asio::io_context io_context;

class session :public std::enable_shared_from_this<session> {
public:
	session(tcp::socket socket)
		: socket_(std::move(socket)), servsock(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), 0)) {

		}
	
	void start() {
		readReq();
	}

private:
	void readReq() {
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, MAXLEN),
			[this, self](const boost::system::error_code ec, std::size_t length)
			{
				if (!ec) {
					do_parse(length);
					permit = check(res.CD, res.ip);

					if (res.CD == 1) {
						connect();
					} else {
						bind();
					}
					msg();
				}
			});
	}
	void do_parse(int length) {
		res = parse(data_);
		
		if ( !strncmp(res.ip.c_str(), "0.0.0.", 6) ) {
			string host;
			int i;
			for (i = 8; i < length; i++) 
				if (data_[i] == 0)
					break;
			for (i+=1; i < length-1; i++) {
				host += data_[i];
			}
			tcp::resolver resolver_(io_context);
			boost::system::error_code ec;
			tcp::resolver::results_type result = resolver_.resolve(host, to_string(res.port), ec);
			
			for (auto it = result.begin(); it != result.end(); it++) {
				res.ip = it->endpoint().address().to_string();
			}
		}
	}

	void msg () {
		uint8_t reply[8];
		memset(reply, 0, sizeof(reply));
		cout << "<S_IP>: " << socket_.remote_endpoint().address().to_string() << '\n';
		cout <<	"S_PORT>: "<< socket_.remote_endpoint().port() << '\n'; 
		cout << "<D_IP>: " << res.ip << '\n';
		cout << "<D_PORT>: " << res.port << '\n';
		if (res.CD == 1) {
			array<unsigned char, 4> ip = socket_.local_endpoint().address().to_v4().to_bytes();
			int port = acceptor_.local_endpoint().port();
			for (int i = 0; i < 4; i++) {
				reply[4+i] = int(ip[i]);
			}
			reply[2] = port / 256;
			reply[3] = port % 256;
			cout << "<Command>: CONNECT\n";
		} else {
			array<unsigned char, 4> ip = socket_.local_endpoint().address().to_v4().to_bytes();
			int port = acceptor_.local_endpoint().port();
			for (int i = 0; i < 4; i++) {
				reply[4+i] = int(ip[i]);
			}
			reply[2] = port / 256;
			reply[3] = port % 256;
				
			cout << "<Command>: BIND\n";
		}
		if (permit == 1) {
			reply[1] = 90;
			cout << "<Reply>: Accept\n";
		} else {
			reply[1] = 91;
			cout << "<Reply>: Reject\n";
		}
		for (int i = 0; i < 8; i++)
			cout << int(reply[i]) << ' ';
		cout << '\n';
		//	write reply to client
		auto self(shared_from_this());
		socket_.write_some(boost::asio::buffer(reply, 8)); 
		return;
	}
	
	void connect() {
		tcp::endpoint ep(boost::asio::ip::address::from_string(res.ip), res.port);
		auto self(shared_from_this());
		servsock.open(tcp::v4());
		servsock.async_connect(ep, 
				[this, self](const boost::system::error_code &err) {
					if (!err) {
						cerr << "connect\n";
						//cout << servsock->remote_endpoint().address().to_string() << ' ' << servsock->remote_endpoint().port() << '\n';
						//cout << servsock->local_endpoint().address().to_string() << ' ' << servsock->local_endpoint().port() << '\n';
						readServer();
						readClient();
					} else {
						cerr << err.message() << '\n';
					}
				});
	}
	void bind() {
		auto self(shared_from_this());
		acceptor_.async_accept(
				[this, self] (const boost::system::error_code &ec, tcp::socket sock ) {
					if (!ec) {
						//check ip
						cerr << "bind\n";
						uint8_t reply[8];
						memset(reply, 0, sizeof(reply));
						if (socket_.remote_endpoint().address().to_string() == res.ip) {
							servsock = move(sock);
							reply[1] = 90;
							socket_.write_some(boost::asio::buffer(reply, 8)); 
						} else {
							reply[1] = 91;
							socket_.write_some(boost::asio::buffer(reply, 8)); 
						}
						readServer();
						readClient();
					} else {
						cerr << ec.message() << '\n';
					}
				});
	}
	void readServer() {
		cout << "read Server\n";
		//string test = "test";
		//servsock->write_some(boost::asio::buffer(test.c_str(), test.size()));
		
		auto self(shared_from_this());
		servsock.async_read_some(boost::asio::buffer(sbuf, MAXLEN), 
				[this, self] (const boost::system::error_code &ec, size_t length) {
					if (!ec) {
						cerr << "readdone\n";
						writeClient(length);
					} else {
						cerr << ec.message() << '\n';
					}
				});
	}
	void writeClient(size_t length) {
		auto self(shared_from_this());
		socket_.async_write_some(boost::asio::buffer(sbuf, length),
				[this, self] (const boost::system::error_code &ec, size_t length) {
					if (!ec) {
						readServer();
					} else {
						cerr << ec.message() << '\n';
					}
				});
	}
	void readClient() {
		cout << "read client\n";
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(cbuf, MAXLEN),
				[this, self] (const boost::system::error_code &ec, size_t length) {
					if (!ec) {
						writeServer(length);
					} else {
						cerr << ec.message() << '\n';
					}
				});
	
	}

	void writeServer(size_t length) {
		auto self(shared_from_this());
		servsock.async_write_some(boost::asio::buffer(cbuf, length),
				[this, self] (const boost::system::error_code &ec, size_t length) {
					if (!ec) {
						readClient();
					} else {
						cerr << ec.message() << '\n';
					}
				});
	}

	tcp::socket socket_;
	tcp::socket servsock;
	tcp::acceptor acceptor_;
	bool permit;
	uint8_t data_[MAXLEN];
	char sbuf[MAXLEN];
	char cbuf[MAXLEN];
	struct parseRes res;
	
};

class server {
public:
	server( short port ) 
		: acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
		signal(SIGCHLD, SIG_IGN);
		do_accept();
	}

private:
	void do_accept() {
		acceptor_.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket) {
				if (!ec) {
					//socket_ = move(socket);
					io_context.notify_fork(boost::asio::io_context::fork_prepare);
					while ( (pid = fork() ) == -1) ;
					//connect();
					if (pid == 0) {
						io_context.notify_fork(boost::asio::io_context::fork_child);
						std::make_shared<session>(std::move(socket))->start();
						//exit()????
					} else {
						io_context.notify_fork(boost::asio::io_context::fork_parent);
						do_accept();
					}
				} else {
					do_accept();	
				}
			});
	}
	
	tcp::acceptor acceptor_;
	int pid;
	

};

int main (int argc, char **argv) {
	try 
	{
		if (argc != 2) {
			std::cerr << "Usage: socks_server <port>\n";
			return -1;
		}

		server s(std::stoi(argv[1]));
		io_context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "exception " << e.what() << '\n';
	
	}
	return 0;
}

