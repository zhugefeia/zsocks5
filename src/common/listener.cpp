#include <listener.h>
#include "network.h"

using namespace asio::ip;


listener::listener(int port) :
_acceptor(network::get_io_service(), tcp::endpoint(tcp::v4(), port)) {
}

void listener::do_accept() {
	_socket = std::make_unique<tcp::socket>(network::get_io_service());
	_acceptor.async_accept(*_socket, [this](std::error_code ec) {
		if (ec) {
			// error
		} else {
			if (_callback) {
				_socket->non_blocking(true);
				_callback(ec, _socket);
			}
		}
	});
}

void listener::start_accept() {
	do_accept();
}