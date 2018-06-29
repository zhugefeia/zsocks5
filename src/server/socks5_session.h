#ifndef __SOCKS_5_SESSION_H__
#define __SOCKS_5_SESSION_H__

#include "globals.h"
#include "connector.h"
#include "buffer.h"
#include "session.h"


class socks5_session;
typedef std::unique_ptr<socks5_session> socks5_session_ptr;


class socks5_session
{
public:
	static socks5_session_ptr create(int id) { return std::make_unique<socks5_session>(id); }

	socks5_session(int id);
	~socks5_session();

	void set_callback(std::function<void(std::error_code, buffer_ptr&, socks5_session*)> callback) { _callback = callback; }
	int get_id() const { return _id; }

	void write(buffer_ptr& buff);

	void close();
	bool is_closed() const;

private:
	void call_close();
	void send_auth_response(unsigned char version, unsigned char method);
	void send_error(char ec, bool closed=true);
	
	void on_session_msg(std::error_code ec, buffer_ptr& buff, session* session);


private:
	int _id;
	connector_ptr _connector;
	socket_ptr _socket;
	std::function<void(std::error_code, buffer_ptr&, socks5_session*)> _callback;
	session_ptr _session;
	buffer_ptr _socks5_buff;

private:
	enum class STATUS : char {
		None, Authed, Forward
	};

	STATUS _status;
};

#endif // !__SOCKS_5_SESSION_H__