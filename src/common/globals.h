#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "asio.hpp"
#include <memory>


typedef std::unique_ptr<asio::ip::tcp::socket> socket_ptr;


#endif // !__GLOBALS_H__