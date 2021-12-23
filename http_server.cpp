#include "server.hpp"

using boost::asio::ip::tcp;
using namespace std;

class session :public std::enable_shared_from_this<session> {
public:
	session(tcp::socket socket)
		: socket_(std::move(socket)){}
	
	void start() {
		do_read();
	}

private:
	void do_read() {
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, MAXLEN),
			[this, self](const boost::system::error_code ec, std::size_t length)
			{
				if (!ec) {
					do_work();
				}
			});
	}
	void do_work() {
		string input(data_);
		res = parse(input); 
		//res.print();
		
		//exec cgi
		do_exec(res.target);
	}
	void do_exec(string cmd) {
		int pid;
		signal(SIGCHLD, SIG_IGN);
		if ( (pid = fork()) < 0) {
			perror("fork error\n");
		}
		if (pid == 0) {
			setenv_();
			dup2(socket_.native_handle(), 0);
			dup2(socket_.native_handle(), 1);
			char *arg[10];
			string tmp = "." + cmd;
			char *c = strdup(tmp.c_str());
			arg[0] = c;
			arg[1] = NULL;
			socket_.close();
			cout << "HTTP/1.1 200 OK\r\n";
			fflush(stdout);
			if (execvp(arg[0], arg) < 0) {
				perror("Exec error");
				exit(-1);
			}
		
		} else {
			socket_.close();
		}
	}
	void setenv_() {
		setenv("REQUEST_METHOD", res.method.c_str(), 1);
		setenv("REQUEST_URI", res.url.c_str(), 1);
		setenv("QUERY_STRING", res.query.c_str(), 1);
		setenv("SERVER_PROTOCOL", res.proto.c_str(), 1);
		setenv("HTTP_HOST", res.host.c_str(), 1);
		setenv("SERVER_ADDR", socket_.local_endpoint().address().to_string().c_str(), 1);
		setenv("SERVER_PORT", to_string(socket_.local_endpoint().port()).c_str(), 1);
		setenv("REMOTE_ADDR", socket_.remote_endpoint().address().to_string().c_str(), 1);
		setenv("REMOTE_PORT", to_string(socket_.remote_endpoint().port()).c_str(), 1);
	}	

	tcp::socket socket_;
	char data_[MAXLEN];
	struct parseRes res;
	
};

class server {
public:
	server(boost::asio::io_context &io_context, short port) 
		: acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
		do_accept();
	}

private:
	void do_accept() {
		acceptor_.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket) {
				if (!ec) {
					//std::cout << socket.remote_endpoint().address().to_string() << '\n';
					//std::cout << socket.remote_endpoint().port() << '\n';
					//std::cout << "accept\n";
					std::make_shared<session>(std::move(socket))->start();
				}
				do_accept();	
			});
	}
	
	tcp::acceptor acceptor_;
	

};

int main (int argc, char **argv) {
	try 
	{
		if (argc != 2) {
			std::cerr << "Usage: http_server <port>\n";
			return -1;
		}

		boost::asio::io_context io_context;
		
		server s(io_context, std::stoi(argv[1]));
		io_context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "exception " << e.what() << '\n';
	
	}
	return 0;
}

