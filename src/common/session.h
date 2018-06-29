#ifndef _SESSION_H__
#define _SESSION_H__

#include "globals.h"
#include <memory>
#include "buffer.h"
#include <functional>
#include <queue>
#include "connector.h"


class session;
typedef std::unique_ptr<session> session_ptr;


class session {
public:
	static session_ptr create(socket_ptr& socket, int id) { return std::make_unique<session>(socket, id); }

	session(socket_ptr& socket, int id);
	~session();

	void set_callback(std::function<void(std::error_code, buffer_ptr&, session*)> callback) { _callback = callback; }

	void write(buffer_ptr& buff);
	int get_id() const { return _id; }
	bool is_closed() const { return _is_closed; }

	void close();

private:
	void do_read();
	void do_write();
	void try_close();
	void shutdown();

private:
	int _id;
	socket_ptr _socket;
	buffer_ptr _read_buff;
	std::queue<buffer_ptr> _buff_queue;
	std::function<void(std::error_code, buffer_ptr&, session*)> _callback;
	connector_ptr _connector;
	bool _is_closed;
	bool _is_read_stop;

public:
	static int gen_id();

private:
	static int s_id;
};

#endif // _SESSION_H__