#include "socks5_session.h"
#include "const.h"


enum authmethod {
	AM_NO_AUTH = 0,
	AM_GSSAPI = 1,
	AM_USERNAME = 2,
	AM_INVALID = 0xFF
};


enum errorcode {
	EC_SUCCESS = 0,
	EC_GENERAL_FAILURE = 1,
	EC_NOT_ALLOWED = 2,
	EC_NET_UNREACHABLE = 3,
	EC_HOST_UNREACHABLE = 4,
	EC_CONN_REFUSED = 5,
	EC_TTL_EXPIRED = 6,
	EC_COMMAND_NOT_SUPPORTED = 7,
	EC_ADDRESSTYPE_NOT_SUPPORTED = 8,
};


socks5_session::socks5_session(int id) :
	_id(id), _status(STATUS::None) {
	_socks5_buff = buffer::create(1024);
}

socks5_session::~socks5_session() {
	if (_session) {
		_session.reset();
	}
	_callback = nullptr;
}

void socks5_session::write(buffer_ptr& buff) {
	switch (_status) {
	case STATUS::None:
	{
		//_socks5_buff->write_data(buff->get_read_data(), buff->get_available_read_size());
		int size = buff->get_available_read_size();

		unsigned char ver = buff->read_byte();
		if (ver != 5 || size <= 1) {
			call_close();
			return;
		}

		unsigned char method = buff->read_byte();
		int idx = 2;
		while (idx < size && method > 0) {
			unsigned char auth = buff->read_byte();
			if (auth == AM_NO_AUTH) {
				send_auth_response(5, auth);
				_status = STATUS::Authed;
				return;
			}
			idx++;
			method--;
		}
		call_close();
	}
		break;

	case STATUS::Authed:
	{
		int size = buff->get_available_read_size();
		if (size < 5) {
			send_error(-EC_GENERAL_FAILURE);
			return;
		}

		unsigned char ver = buff->read_byte();
		if (ver != 5) {
			send_error(-EC_GENERAL_FAILURE);
			return;
		}

		unsigned char cmd = buff->read_byte();
		if (cmd != 1) {
			send_error(-EC_COMMAND_NOT_SUPPORTED); // only support CONNECT
			return;
		}

		if (buff->read_byte() != 0) {
			send_error(-EC_GENERAL_FAILURE); // malformed packet
			return;
		}

		std::string host_name;
		unsigned char address_type = buff->read_byte();
		switch (address_type) {
		case 1: // ipv4
		{
			if (buff->get_available_read_size() < 4 + 2) {
				send_error(-EC_GENERAL_FAILURE);
				return;
			}

			std::array<unsigned char, 4> ipaddr;
			for (int i = 0; i < 4; i++) {
				ipaddr[i] = buff->read_byte();
			}
			asio::ip::address_v4 addr(ipaddr);
			std::error_code ec;
			host_name = addr.to_string(ec);
			if (ec) {
				send_error(-EC_GENERAL_FAILURE); // malformed or too long addr
				return;
			}
		}
			break;

		case 4: // ipv6
		{
			if (buff->get_available_read_size() < 16 + 2) {
				send_error(-EC_GENERAL_FAILURE);
				return;
			}

			std::array<unsigned char, 16> ipaddr;
			for (int i = 0; i < 16; i++) {
				ipaddr[i] = buff->read_byte();
			}

			asio::ip::address_v6 addr(ipaddr);
			std::error_code ec;
			host_name = addr.to_string(ec);
			if (ec) {
				send_error(-EC_GENERAL_FAILURE); // malformed or too long addr
				return;
			}
		}
			break;

		case 3: // dns name
		{
			unsigned char l = buff->read_byte();
			if (buff->get_available_read_size() < l + 2) {
				send_error(-EC_GENERAL_FAILURE);
				return;
			}

			char data[256];
			buff->read_data((unsigned char*)data, l);
			data[l] = 0;
			host_name = data;
		}
			break;

		default:
			send_error(-EC_ADDRESSTYPE_NOT_SUPPORTED);
			return;
		}

		unsigned short port = buff->read_ushort();
		_connector = connector::create(host_name.c_str(), std::to_string(port).c_str());
		_connector->set_callback([this](std::error_code ec, connector* conn) {
			if (ec) {
				send_error(-EC_HOST_UNREACHABLE);
			} else {
				send_error(EC_SUCCESS, false);
				_status = STATUS::Forward;

				_session = session::create(conn->get_socket(), _id);
				_session->set_callback(std::bind(&socks5_session::on_session_msg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			}
		});
	}
		break;

	case STATUS::Forward:
		printf("socks5 session write %d len=%d\n", _id, buff->get_available_read_size());
		_session->write(buff);
		break;
	}
}

void socks5_session::call_close() {
	if (_callback) {
		_callback(std::error_code(1, std::generic_category()), buffer::empty_buff, this);
	}
}

void socks5_session::send_auth_response(unsigned char version, unsigned char method) {
	printf("auth response %d\n", _id);
	int len = 2;
	buffer_ptr buff = buffer::create_package(_id, len);
	buff->write_byte(version);
	buff->write_byte(method);

// 	int len = INT_LENGTH + 2 + 1; // id + data + rand
// 	buffer_ptr buff = buffer::create(INT_LENGTH + len);
// 	buff->write_int(len);
// 	buff->write_int(_id);
// 	buff->write_byte(version);
// 	buff->write_byte(method);
	if (_callback) {
		_callback(std::error_code(), buff, this);
	}
}

void socks5_session::send_error(char ec, bool closed/* =true */) {
	printf("send error %d id=%d\n", ec, _id);

	char data[10] = { 5, ec, 0, 1 /*AT_IPV4*/, 0,0,0,0, 0,0 };
	int len = 10;
	buffer_ptr buff = buffer::create_package(_id, len);
	buff->write_data((unsigned char*)data, 10);

// 	int len = INT_LENGTH + 10 + 1; // id + data + rand
// 	buffer_ptr buff = buffer::create(INT_LENGTH + len);
// 	buff->write_int(len);
// 	buff->write_int(_id);
// 	buff->write_data((unsigned char*)data, 10);
	if (_callback) {
		if (closed) {
			_callback(std::error_code(1, std::generic_category()), buff, this);
		} else {
			_callback(std::error_code(), buff, this);
		}
	}
}

void socks5_session::on_session_msg(std::error_code ec, buffer_ptr& buff, session* session) {
	printf("socks5 sesson msg %d len=%d \n", _id, buff ? buff->get_available_read_size() : 0);
	if (_callback) {
		_callback(ec, buff, this);
	}
}

void socks5_session::close() {
	if (_session) {
		_session->close();
	} else {
		call_close();
	}
}

bool socks5_session::is_closed() const {
	return _session ? _session->is_closed() : true;
}