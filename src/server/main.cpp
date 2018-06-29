#include <iostream>
#include "asio.hpp"
#include "server.h"
#include "network.h"


int main(int argc, char** argv) {

	random::init();

	buffer::check_endian();

	server svr(21234);

	asio::io_service& io_service = network::get_io_service();
	while (true) {
		std::error_code ec;
		io_service.run(ec);
		if (ec) {
			printf("main loop: %s", ec.message().c_str());
			break;
		}
		else {
			Sleep(30);
		}
	}
}