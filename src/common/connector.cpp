#include "connector.h"
#include "network.h"


using namespace asio::ip;


connector::connector(const char* ip, const char* port):
_socket(new tcp::socket(network::get_io_service())), _resolver(std::make_unique<tcp::resolver>(network::get_io_service())) {
	_socket->non_blocking();
	do_connect(ip, port);
}

void connector::do_connect(const char* ip, const char* port) {
	//tcp::resolver r(network::get_io_service());
	_resolver->async_resolve(ip, port, [this](std::error_code ec, tcp::resolver::iterator iter) {
		if (ec) {
			if (_callback) {
				_callback(ec, this);
			}
		} else {
			_socket->async_connect(*iter, [this](const std::error_code& ec) {
				if (_callback) {
					_socket->non_blocking(true);
					_callback(ec, this);
				}
			});
		}
	});
}

connector::connector(asio::ip::tcp::endpoint endpoint) :
	_socket(new tcp::socket(network::get_io_service())) {
	_socket->async_connect(endpoint, [this](const std::error_code& ec) {
		if (_callback) {
			_callback(ec, this);
		}
	});
}