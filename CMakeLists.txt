cmake_minimum_required (VERSION 2.6)
project (zsocks5)

include_directories("lib/asio")
include_directories("src/common")

add_definitions(-DASIO_STANDALONE)

if (WIN32)
	add_definitions(-D_WIN32_WINNT=0x0601)

else()
	add_definitions(-DASIO_HAS_STD_ADDRESSOF)
	add_definitions(-DASIO_HAS_STD_ARRAY)
	add_definitions(-DASIO_HAS_CSTDINT)
	add_definitions(-DASIO_HAS_STD_SHARED_PTR)
	add_definitions(-DASIO_HAS_STD_TYPE_TRAITS)

	add_definitions(-DASIO_HAS_VARIADIC_TEMPLATES)
	add_definitions(-DASIO_HAS_STD_FUNCTION)
	add_definitions(-DASIO_HAS_STD_CHRONO)
	add_definitions(-DASIO_HAS_STD_ATOMIC)

	add_compile_options(-std=c++11)

endif()

aux_source_directory("src/common" COMMON_DIR_SRCS)
aux_source_directory("src/client" CLIENT_DIR_SRCS)
aux_source_directory("src/server" SERVER_DIR_SRCS)

add_executable(client ${CLIENT_DIR_SRCS} ${COMMON_DIR_SRCS})
add_executable(server ${SERVER_DIR_SRCS} ${COMMON_DIR_SRCS})

if (UNIX)
	target_link_libraries(server pthread)
endif()