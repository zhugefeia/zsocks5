#ifndef __SERVER_H__
#define __SERVER_H__

#include "listener.h"
#include <memory>
#include "tunnel.h"
#include "socks5_session.h"
#include <unordered_map>


class server
{
public:
	server(int port);

private:
	void do_listener();

	void on_session_msg(const std::error_code& ec, buffer_ptr& buff, socks5_session* session);
	void on_tunnel_msg(const std::error_code& ec, buffer_ptr& buff);

private:
	int _port;
	listener_ptr _listener;
	tunnel_ptr _tunnel;
	std::unordered_map<int, socks5_session_ptr> _session_map;
};

#endif // !__SERVER_H__
