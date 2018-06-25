cmake_minimum_required (VERSION 2.6)
project (zsocks5)

include_directories("lib/asio")

add_definitions(-DASIO_STANDALONE)

if (WIN32)
	add_definitions(-D_WIN32_WINNT=0x0601)
endif()

aux_source_directory("src/client" CLIENT_DIR_SRCS)
aux_source_directory("src/server" SERVER_DIR_SRCS)

add_executable(client ${CLIENT_DIR_SRCS})
add_executable(server ${SERVER_DIR_SRCS})