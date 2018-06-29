#include "tunnel.h"
#include "const.h"


tunnel::tunnel(std::unique_ptr<asio::ip::tcp::socket>& socket) :
	_socket(std::move(socket)) {
	_read_buff = std::make_unique<buffer>(MAX_LENGTH * 2);
	do_read();
}

void tunnel::write(std::unique_ptr<buffer>& buff) {
	buff->encrypt_package();
	_buff_queue.push(std::move(buff));
	if (_buff_queue.size() == 1) {
		do_write();
	}
}

void tunnel::do_read() {
	_socket->async_read_some(asio::buffer(_read_buff->get_write_data(), _read_buff->get_available_write_size()), [this](std::error_code ec, std::size_t bytes_transferred) {
		printf("aa tunnel read = %d\n", bytes_transferred);
		if (ec) {
			_read_buff->move_write_pos(bytes_transferred);
			int read_size = _read_buff->get_available_read_size();
			if (read_size >= 2) {
				uint16_t msg_len = _read_buff->test_read_ushort();
				printf("session errror msg_len =%d\n", msg_len);
			}

			if (_callback) {
				_callback(ec, buffer::empty_buff);
			}
		} else {
			_read_buff->move_write_pos(bytes_transferred);
			int read_size = _read_buff->get_available_read_size();
			if (read_size >= 2) {
				uint16_t msg_len = _read_buff->test_read_ushort();
				printf("=============================================== tunnel msg len=%d\n", msg_len);
				if (msg_len + 2 <= read_size) {
					buffer_ptr msg = buffer::create(msg_len);
					_read_buff->read_ushort();
					_read_buff->read_data(msg->get_data(), msg_len);
					msg->move_write_pos(msg_len);
					msg->decrypt();

					_read_buff->clear_read();
					do_read();

					if (_callback) {
						_callback(ec, msg);
					}
					return;
				}
			}
			do_read();
		}
	});
}

void tunnel::do_write() {
	if (_buff_queue.size() > 0) {
		printf("tbb tunne writeing\n");
		buffer_ptr& buff = _buff_queue.front();
		_socket->async_write_some(asio::buffer(buff->get_read_data(), buff->get_available_read_size()), [this](std::error_code ec, std::size_t bytes_transferred) {
			if (ec) {
				// error
			} else {
				buffer_ptr& buff = _buff_queue.front();
				buff->move_read_pos(bytes_transferred);
				if (buff->get_available_read_size() == 0) {
					_buff_queue.pop();
				}
				do_write();
			}
		});

	}
	else {
		printf("bb tunnel write stop\n");
	}
}

