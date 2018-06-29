#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "asio.hpp"
#include <memory>
#include <functional>
#include "globals.h"


class listener;
typedef std::unique_ptr<listener> listener_ptr;

class listener
{
public:
	static listener_ptr create(int port) { return std::move(std::make_unique<listener>(port)); }

	listener(int port);

	void start_accept();
	void set_callback(std::function<void(std::error_code ec, socket_ptr&)> callback) { _callback = callback; }

private:
	void do_accept();

private:
	asio::ip::tcp::acceptor _acceptor;
	std::unique_ptr<asio::ip::tcp::socket> _socket;
	std::function<void(std::error_code ec, socket_ptr&)> _callback;
};

#endif // !__ACCEPTOR_H__