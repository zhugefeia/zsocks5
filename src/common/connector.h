#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "asio.hpp"
#include <memory>
#include <functional>


class connector;
typedef std::unique_ptr<connector> connector_ptr;


class connector
{
public:
	static connector_ptr create(const char* ip, const char* port) { return std::make_unique<connector>(ip, port); }

	connector(const char* ip, const char* port);
	connector(asio::ip::tcp::endpoint endpoint);

public:
	void set_callback(std::function<void(std::error_code, connector*)> callback) { _callback = callback; }
	std::unique_ptr<asio::ip::tcp::socket>& get_socket() { return _socket; }

private:
	void do_connect(const char* ip, const char* port);

private:
	std::unique_ptr<asio::ip::tcp::socket> _socket;
	std::unique_ptr<asio::ip::tcp::resolver> _resolver;
	std::function<void(std::error_code, connector*)> _callback;
};


#endif // __CONNECTOR_H__
