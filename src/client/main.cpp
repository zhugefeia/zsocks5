#include <iostream>
#include "asio.hpp"
#include "client.h"
#include "network.h"


int main(int argc, char** argv) {
	random::init();
	buffer::check_endian();

	client cli("127.0.0.1", "21234", 22234);

	asio::io_service& io_service = network::get_io_service();
	while (true) {
		std::error_code ec;
		io_service.run(ec);
		if (ec) {
			printf("main loop: %s", ec.message().c_str());
			break;
		} else {
			Sleep(30);
		}
	}

	return 0;
}