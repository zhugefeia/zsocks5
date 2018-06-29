#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <string>
#include <cstdint>
#include <memory>

#include <random>


class random
{
public:
	static void init();
	static uint32_t gen();

private:
	static std::default_random_engine* _engine;
	static std::uniform_int_distribution<uint32_t>* _dis;
};


class buffer;
typedef std::unique_ptr<buffer> buffer_ptr;


class buffer
{
public:
	static buffer_ptr create(int size) { return std::make_unique<buffer>(size); };
	static buffer_ptr create_package(int code, int len);
	static buffer_ptr empty_buff;


	buffer(int size);
	~buffer();

public:
	void write_data(const unsigned char* data, int size);
	void write_int(int v);
	void write_uint(uint32_t v);
	void write_ushort(uint16_t v);
	void write_string(const unsigned char* str, int size);
	void write_byte(unsigned char v);

	void read_data(unsigned char* const data, int size);
	int read_int();
	uint32_t read_uint();
	uint16_t read_ushort();
	void read_string(unsigned char* const str, int size);
	unsigned char read_byte();

	int test_read_int();
	uint16_t test_read_ushort();

	unsigned char* get_data() const { return _data; }
	unsigned char* get_read_data() const { return _data + _rpos; }
	unsigned char* get_write_data() const { return _data + _wpos; }

	int get_size() const { return _size; }

	void move_write_pos(int v);
	void move_read_pos(int v);
	int get_available_write_size() { return _size - _wpos; }
	int get_available_read_size() { return _wpos >= _rpos ? (_wpos - _rpos) : 0; }

	void clear_read();
	void clear() { _wpos = _rpos = 0; }

	void encrypt_package();
	void decrypt();

	bool is_empty() { return _data == nullptr; }

	static void check_endian();
	static int switch_endian(int v) {
		return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
	}
	static uint32_t switch_endian(uint32_t v) {
		return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
	}
	static uint16_t switch_endian(uint16_t v) {
		return ((v & 0xFF) << 8) | ((v & 0xFF00) >> 8);
	}

private:
	unsigned char* _data;
	int _rpos, _wpos;
	int _size;
	static bool s_is_little_endian;
	static uint32_t s_key[4];
};

#endif // !__BUFFER_H__
