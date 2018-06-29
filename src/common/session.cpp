#include "session.h"
#include "const.h"


int session::s_id = 0;

int session::gen_id() {
	return s_id = (s_id + 1) % 100000;
}


session::session(socket_ptr& socket, int id) :
_socket(std::move(socket)), _id(id), _is_closed(false), _is_read_stop(false) {
	_read_buff = buffer::create(MAX_LENGTH);
	do_read();
}

session::~session() {
// 	if (_socket) {
// 		_socket->cancel();
// 		_socket->shutdown(asio::ip::tcp::socket::shutdown_both);
// 		_socket->close();
// 		_socket.reset();
// 		_callback = nullptr;
// 	}
}

void session::write(buffer_ptr& buff) {
	if (_is_closed) return;

	_buff_queue.push(std::move(buff));
	if (_buff_queue.size() == 1) {
		do_write();
	}
}

void session::do_read() {
	if (_is_closed) return;

	printf("raw session read %d\n", _read_buff->get_available_write_size());
	_socket->async_read_some(asio::buffer(_read_buff->get_write_data(), _read_buff->get_available_write_size()), [this](std::error_code ec, std::size_t bytes_transferred) {
		if (_is_closed) {
			_is_read_stop = true;
			try_close();
			return;
		}

		printf("raw session on read %d\n", bytes_transferred);

		if (bytes_transferred > 0) {
			_read_buff->move_write_pos(bytes_transferred);
			int read_size = _read_buff->get_available_read_size();
			/*int msg_len = INT_LENGTH + read_size + 1; // id + data + rand
			buffer_ptr buff = buffer::create(INT_LENGTH + msg_len);
			buff->write_int(msg_len);
			buff->write_int(_id);
			_read_buff->read_data(buff->get_write_data(), read_size);
			buff->move_write_pos(read_size);*/

			int msg_len = read_size;
			buffer_ptr buff = buffer::create_package(_id, read_size);
			_read_buff->read_data(buff->get_write_data(), read_size);
			buff->move_write_pos(read_size);

			if (_callback) {
				_callback(ec, buff, this);
			}

			_read_buff->clear_read();
		}
		else if (bytes_transferred == 0) {
			_callback(std::error_code(1, std::generic_category()), buffer::empty_buff, this);
		}
		else if (ec && _callback) {
			_callback(ec, buffer::empty_buff, this);
		}

		if (_is_closed) {
			_is_read_stop = true;
			try_close();
			return;
		}

		do_read();
	});
}

void session::do_write() {
	if (_buff_queue.size() > 0) {
		buffer_ptr& buff = _buff_queue.front();
		_socket->async_write_some(asio::buffer(buff->get_read_data(), buff->get_available_read_size()), [this](std::error_code ec, std::size_t bytes_transferred) {
			if (ec) {
				std::queue<buffer_ptr> empty;
				_buff_queue.swap(empty);
				if (_is_closed) {
					shutdown();
					try_close();
					return;
				} else {
					if (_callback) {
						_callback(ec, buffer::empty_buff, this);
					}
				}
				
			} else {
				buffer_ptr& buff = _buff_queue.front();
				buff->move_read_pos(bytes_transferred);
				if (buff->get_available_read_size() == 0) {
					_buff_queue.pop();
					if (_is_closed && _buff_queue.size() == 0) {
						shutdown();
						try_close();
						return;
					}

				}
				do_write();
			}
		});
	}
}

void session::close() {
	if (_is_closed) return;
	_is_closed = true;

	if (_buff_queue.size() == 0) {
		shutdown();
		try_close();
	}
}

void session::try_close() {
	if (_is_closed && _is_read_stop && _buff_queue.size() == 0) {
		if (_callback) {
			_callback(std::error_code(1, std::generic_category()), buffer::empty_buff, this);
		}
	}
}

void session::shutdown() {
	if (_socket) {
		_socket->cancel();
		_socket->shutdown(asio::socket_base::shutdown_both);
		_socket->close();
	}
}