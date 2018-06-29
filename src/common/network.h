#ifndef __NET_WORK_H__
#define __NET_WORK_H__

#include "asio.hpp"


class network
{
public:
	static asio::io_service& get_io_service() {
		return io_service;
	}

private:
	static asio::io_service io_service;
};


#endif // __NET_WORK_H__