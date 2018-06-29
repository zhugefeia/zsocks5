#ifndef __TUNNEL_H__
#define __TUNNEL_H__

#include "asio.hpp"
#include <memory>
#include <queue>
#include <functional>
#include "buffer.h"

class tunnel;
typedef std::unique_ptr<tunnel> tunnel_ptr;


class tunnel
{
public:
	tunnel(std::unique_ptr<asio::ip::tcp::socket>& socket);

public:
	void write(std::unique_ptr<buffer>& buff);

	void set_callback(std::function<void(std::error_code, buffer_ptr&)> callback) { _callback = callback; }

private:
	void do_read();
	void do_write();

private:
	std::unique_ptr<asio::ip::tcp::socket> _socket;
	std::queue<buffer_ptr> _buff_queue;
	buffer_ptr _read_buff;
	std::function<void(std::error_code, buffer_ptr&)> _callback;

};

#endif // !__TUNNEL_H__