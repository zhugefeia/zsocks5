#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "asio.hpp"
#include <memory>
#include "connector.h"
#include <tunnel.h>
#include <unordered_map>
#include "listener.h"
#include "session.h"


class client
{
public:
	client(const char* sever_ip, const char* server_port, int listen_port);

private:
	//void do_accept();
	//void connect();
	void connect_server();
	void start_listener();
	void do_listener();

	void on_session_msg(std::error_code ec, buffer_ptr& buff, session* session);
	void on_tunnel_msg(std::error_code ec, buffer_ptr& buff);

private:
	std::string _server_ip;
	std::string _server_port;
	int _listen_port;
	connector_ptr _connector;
	tunnel_ptr _tunnel;
	listener_ptr _listener;
	std::unordered_map<int, session_ptr> _session_map;
	//asio::ip::tcp::acceptor _acceptor;
	//asio::ip::tcp::socket _socket;
};

#endif // !__CLIENT_H__
