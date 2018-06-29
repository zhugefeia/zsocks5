#include "server.h"
#include "const.h"


server::server(int port) :
	_port(port) {
	do_listener();
}

void server::do_listener() {
	_listener = std::make_unique<listener>(_port);
	_listener->start_accept();
	_listener->set_callback([this](std::error_code ec, socket_ptr& socket) {
		if (ec) {
			// error
		} else {
			_tunnel = std::make_unique<tunnel>(socket);
			_tunnel->set_callback([this](std::error_code ec, buffer_ptr& buff) {
				int msg_code = buff->read_int();
				if (msg_code == MSG_AUTHOR) {
					char name[20];
					char password[20];
					buff->read_string((unsigned char*)name, 20);
					buff->read_string((unsigned char*)password, 20);

					printf("name: %s, password: %s", name, password);
					int len = 1;
					buffer_ptr buff = buffer::create_package(MSG_AUTHOR, 1);
					buff->write_byte(1);

// 					int len = INT_LENGTH + 1;
// 					buffer_ptr buff = std::make_unique<buffer>(INT_LENGTH + len + 1);
// 					buff->write_int(len + 1);
// 					buff->write_int(MSG_AUTHOR);
// 					buff->write_byte(1);
					_tunnel->write(buff);

					_listener.reset();
					_tunnel->set_callback(std::bind(&server::on_tunnel_msg, this, std::placeholders::_1, std::placeholders::_2));
				}
			});
		}
	});
}

void server::on_session_msg(const std::error_code& ec, buffer_ptr& buff, socks5_session* session) {
	if (buff) {
		printf("session msg %d, len=%d\n", session->get_id(), buff->get_available_read_size());
		_tunnel->write(buff);
	}

	if (ec) {
		int id = session->get_id();
		if (session->is_closed()) {
			printf("session real close %d\n", id);
			_session_map.erase(id);
		} else {
			printf("session close %d\n", id);

			int len = INT_LENGTH;
			buffer_ptr msg = buffer::create_package(MSG_RM_SESSION, len);
			msg->write_int(id);

// 			int len = INT_LENGTH + INT_LENGTH + 1; // code + id + rand
// 			buffer_ptr msg = buffer::create(INT_LENGTH + len);
// 			msg->write_int(len);
// 			msg->write_int(MSG_RM_SESSION);
// 			msg->write_int(id);
			_tunnel->write(msg);

			session->close();
		}

	}
}

void server::on_tunnel_msg(const std::error_code& ec, buffer_ptr& buff) {
	printf(" -- -- session cout %d\n", _session_map.size());

	if (ec) {
		// tunnel error
	} else {
		int code = buff->read_int();
		printf("tunnel message id = %d\n", code);
		if (code < 0) {
			// control message
			switch (code) {
			case MSG_NEW_SESSION:
			{
				int id = buff->read_int();
				printf("new session id=%d\n", id);
				socks5_session_ptr session = socks5_session::create(id);
				session->set_callback(std::bind(&server::on_session_msg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
				_session_map[id] = std::move(session);
			}
				break;

			case MSG_RM_SESSION:
			{
				int id = buff->read_int();
				printf("remove session id=%d\n", id);
				//_session_map.erase(id);
				_session_map[id]->close();
			}
				break;
			}

		} else {
			auto& iter = _session_map.find(code);
			if (iter != _session_map.end()) {
				printf("tunnel session msg %d, len=%d\n", code, buff->get_available_read_size());
				iter->second->write(buff);
			}
		}
	}
}
