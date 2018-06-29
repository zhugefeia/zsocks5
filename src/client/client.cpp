#include "client.h"
#include "connector.h"
#include "buffer.h"
#include "const.h"
#include "globals.h"


client::client(const char* sever_ip, const char* server_port, int listen_port):
_server_ip(sever_ip), 
_server_port(server_port), 
_listen_port(listen_port) {
	connect_server();
}

void client::connect_server() {
	_connector = std::make_unique<connector>(_server_ip.c_str(), _server_port.c_str());
	_connector->set_callback([this](std::error_code ec, connector* conn) {
		if (ec) {
			printf("%s", ec.message().c_str());
			// connect_error
		} else {
			_tunnel = std::make_unique<tunnel>(conn->get_socket());
			//_tunnel.reset(new tunnel(conn->get_socket()));
			
			const char* password = "123456";

			const char* name = "zhugefei";
			int name_len = strlen(name) + 1;
			int password_len = strlen(password) + 1;
			int msg_len =  name_len + password_len + 8;
			buffer_ptr buff = buffer::create_package(MSG_AUTHOR, msg_len);
			buff->write_string((unsigned char*)password, password_len);

			buff->write_string((unsigned char*)name, name_len);
// 			buff->move_read_pos(2 + 4 + password_len + 4);
// 			char a[100];
// 			buff->read_string((unsigned char*)a, 100);
// 			printf("%s", a);


			/*int len = 4 + 4 + strlen(name) + 1 + strlen(password) + 1 + 4 + 4 + 1;
			buffer_ptr buff = std::make_unique<buffer>(len);
			buff->write_int(len - 4);
			buff->write_int(MSG_AUTHOR);
			buff->write_string((unsigned char*)name, strlen(name) + 1);
			buff->write_string((unsigned char*)password, strlen(password) + 1);*/
			_tunnel->write(buff);
			_tunnel->set_callback([this](std::error_code ec, buffer_ptr& buff) {
				if (ec) {
					// tunnel error
				} else {
					int msg_code = buff->read_int();
					if (msg_code == MSG_AUTHOR) {
						char ret = buff->read_byte();
						if (ret == 1) {
							printf("%s", "success");
							start_listener();
							_tunnel->set_callback(std::bind(&client::on_tunnel_msg, this, std::placeholders::_1, std::placeholders::_2));
						}
					}

				}
			});
			_connector.reset();
		}
	});
}


void client::start_listener() {
	_listener = listener::create(_listen_port);
	_listener->set_callback([this](std::error_code ec, socket_ptr& socket) {
		if (ec) {
			printf("listener error: %s\n", ec.message().c_str());
		} else {
			printf("accept listener\n");
			int id = session::gen_id();
			session_ptr session = session::create(socket, id);
			session->set_callback(std::bind(&client::on_session_msg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			_session_map[id] = std::move(session);

			int len = INT_LENGTH;
			buffer_ptr msg = buffer::create_package(MSG_NEW_SESSION, len);
			msg->write_int(id);

// 			int len = INT_LENGTH + INT_LENGTH + 1; // code + id + rand
// 			buffer_ptr msg = buffer::create(INT_LENGTH + len);
// 			msg->write_int(len);
// 			msg->write_int(MSG_NEW_SESSION);
// 			msg->write_int(id);
			_tunnel->write(msg);

			printf("new session %d\n", id);

			_listener->start_accept();
		}
	});
	_listener->start_accept();
}

void client::on_session_msg(std::error_code ec, buffer_ptr& buff, session* session) {
	if (buff) {
		//printf("session msg %d, len=%d\n", session->get_id(), buff->get_available_read_size());
		_tunnel->write(buff);
	}

	if (ec) {
		int id = session->get_id();

		if (session->is_closed()) {
			printf("session_real_close %d : %s\n", session->get_id(), ec.message().c_str());
			_session_map.erase(id);
		} else {
			printf("session_close %d : %s\n", session->get_id(), ec.message().c_str());

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

void client::on_tunnel_msg(std::error_code ec, buffer_ptr& buff) {
	printf(" -- -- session cout %d\n", _session_map.size());

	if (ec) {
		// tunnel error
		printf("tunnel close£º %s\n", ec.message().c_str());
	} else {
		int code = buff->read_int();
		printf("tunnel message code=%d \n", code);
		if (code < 0) {
			// control message
			switch (code) {
			case MSG_RM_SESSION:
 				int id = buff->read_int();
// 				printf("remove session id=%d\n", id);
// 				_session_map.erase(id);
				_session_map[id]->close();
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

/*
void client::do_accept() {
	asio::ip::tcp::acceptor _acceptor;
	_acceptor.async_accept(_socket, [this](std::error_code ec) {
		if (!ec) {
			printf("accept ...");
		}
		do_accept();
	});
}*/