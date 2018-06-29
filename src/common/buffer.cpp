#include "buffer.h"
#include "const.h"


/** 
XXTEA
*/

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

void btea(uint32_t *v, int n, uint32_t const key[4]) {
	uint32_t y, z, sum;
	unsigned p, rounds, e;
	if (n > 1) {          /* Coding Part */
		rounds = 6 + 52 / n;
		sum = 0;
		z = v[n - 1];
		do {
			sum += DELTA;
			e = (sum >> 2) & 3;
			for (p = 0; p < n - 1; p++) {
				y = v[p + 1];
				z = v[p] += MX;
			}
			y = v[0];
			z = v[n - 1] += MX;
		} while (--rounds);
	}
	else if (n < -1) {  /* Decoding Part */
		n = -n;
		rounds = 6 + 52 / n;
		sum = rounds*DELTA;
		y = v[0];
		do {
			e = (sum >> 2) & 3;
			for (p = n - 1; p > 0; p--) {
				z = v[p - 1];
				y = v[p] -= MX;
			}
			z = v[n - 1];
			y = v[0] -= MX;
			sum -= DELTA;
		} while (--rounds);
	}
}

////////////////////////////////////////////////////////


std::default_random_engine* random::_engine = nullptr;
std::uniform_int_distribution<uint32_t>* random::_dis = nullptr;

bool buffer::s_is_little_endian = false;
uint32_t buffer::s_key[4] = { 0xFU, 0xFU, 0xFU, 0xFU };
buffer_ptr buffer::empty_buff;


buffer_ptr buffer::create_package(int code, int len) {
	int size = 2 + 4 + len;
	

 	//int size = 2 + INT_LENGTH + len + 2 + INT_LENGTH; // head + code + data + len + rand
 	//int remainder = (len + 2) % INT_LENGTH;
 	//if (remainder > 0) {
 	//	size += INT_LENGTH - remainder;
 	//}
	buffer_ptr buff = buffer::create(size);
	buff->write_ushort(size - 2);
	buff->write_int(code);

	return std::move(buff);
}


void random::init() {
	if (_engine == nullptr) {
		std::random_device rd;
		_engine = new std::default_random_engine(rd());
		_dis = new std::uniform_int_distribution<uint32_t>(0, 0xFFFFFFFF);
	}
}

uint32_t random::gen() {
	return (*_dis)(*_engine);
}



void buffer::check_endian() {
	int v = 0x12345678;
	char a = *(char*)&v;
	s_is_little_endian = a == 0x78;
}

buffer::buffer(int size) :
_rpos(0), _wpos(0), _size(size) {
	if (size > 0) {
		_data = new unsigned char[size];
	} else {
		_data = nullptr;
	}
}

buffer::~buffer() {
	if (_data) {
		delete[] _data;
	}
}

void buffer::write_data(unsigned const char* data, int size) {
	if (size <= _size - _wpos) {
		memcpy(_data + _wpos, data, size);
		_wpos += size;
	}
}

void buffer::write_int(int v) {
	if (INT_LENGTH <= _size - _wpos) {
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
		*(int32_t*)(_data + _wpos) = v;
		_wpos += INT_LENGTH;
	}
}

void buffer::write_uint(uint32_t v) {
	if (INT_LENGTH <= _size - _wpos) {
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
		*(uint32_t*)(_data + _wpos) = v;
		_wpos += INT_LENGTH;
	}
}

void buffer::write_ushort(uint16_t v) {
	if (2 <= _size - _wpos) {
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
		*(uint16_t*)(_data + _wpos) = v;
		_wpos += 2;
	}
}

void buffer::write_string(unsigned const char* str, int size) {
	if (size + INT_LENGTH <= _size - _wpos) {
		write_int(size);
		memcpy(_data + _wpos, str, size);
		_wpos += size;
	}
}

void buffer::write_byte(unsigned char v) {
	if (_size - _wpos >= 1) {
		_data[_wpos] = v;
		_wpos += 1;
	}
}

void buffer::read_data(unsigned char* const data, int size) {
	if (size <= _wpos - _rpos) {
		memcpy(data, _data + _rpos, size);
		_rpos += size;
	}
}

int buffer::read_int() {
	int v = 0;
	if (INT_LENGTH <= _wpos - _rpos) {
		v = *(int32_t*)(_data + _rpos);
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
		_rpos += INT_LENGTH;
	}
	return v;
}

uint32_t buffer::read_uint() {
	int v = 0;
	if (INT_LENGTH <= _wpos - _rpos) {
		v = *(uint32_t*)(_data + _rpos);
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
		_rpos += INT_LENGTH;
	}
	return v;
}

void buffer::read_string(unsigned char* const str, int size) {
	if (INT_LENGTH < _wpos - _rpos) {
		int len = read_int();
		if (len <= size && len <= _size - _rpos) {
			memcpy(str, _data + _rpos, len);
			_rpos += len;
		}
	}
}

unsigned char buffer::read_byte() {
	unsigned char v = 0;
	if (_wpos - _rpos >= 1) {
		v = _data[_rpos];
		_rpos += 1;
	}
	return v;
}

uint16_t buffer::read_ushort() {
	uint16_t v = 0;
	if (2 <= _wpos - _rpos) {
		v = *(uint16_t*)(_data + _rpos);
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
		_rpos += 2;
	}
	return v;
}

int buffer::test_read_int() {
	int v = 0;
	if (INT_LENGTH <= _wpos - _rpos) {
		v = *(int32_t*)(_data + _rpos);
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
	}
	return v;
}

uint16_t buffer::test_read_ushort() {
	uint16_t v = 0;
	if (2 <= _wpos - _rpos) {
		v = *(uint16_t*)(_data + _rpos);
		if (s_is_little_endian) {
			v = switch_endian(v);
		}
	}
	return v;
}

void buffer::move_write_pos(int v) {
	_wpos += v;
	_wpos = _wpos < 0 ? 0 : (_wpos > _size ? _size : _wpos);
}

void buffer::move_read_pos(int v) {
	_rpos += v;
	_rpos = _rpos < 0 ? 0 : (_rpos > _wpos ? _wpos : _rpos);
}

void buffer::clear_read() {
	if (_wpos > _rpos) {
		memmove(_data, _data + _rpos, _wpos - _rpos);
		//int len = _wpos - _rpos;
		//for (int i = 0; i < len; i++) {
		//	_data[i] = _data[_rpos + i];
		//}
	}
	//printf("11 rd_pos = %d, w_pos = %d\n", _rpos, _wpos);
	move_write_pos(-_rpos);
	_rpos = 0;
	//printf("rd_pos = %d, w_pos = %d\n",_rpos,_wpos);
}

void buffer::encrypt_package() {
	return;
	uint16_t len = _wpos - _rpos - 2;
	_wpos = _size - 2;
	printf("x + x + x +  @@@@@@@  en len: %d\n", len);
	write_ushort(len);

	//uint32_t rand = random::gen();
	//uint32_t key[4];
	//for (int i = 0; i < 4; i++) {
	//	key[i] = s_key[i] ^ rand;
	//}
	//write_uint(rand);

	//int n = (_size - INT_LENGTH - 2) / 4;
// 	for (int i = 0; i < n * 4; i++) {
// 		printf("%c", _data[2 + i]);
// 	}
// 	printf("\n");
	//btea((uint32_t*)(_data + 2), n, key);

// 	for (int i = 0; i < n * 4; i++) {
// 		printf("%d", _data[2 + i]);
// 	}
// 	printf("\n");

	//btea((uint32_t*)(_data + 2), -n, key);
// 	for (int i = 0; i < n * 4; i++) {
// 		printf("%c", _data[2 + i]);
// 	}
// 	printf("\n");

	/*_wpos = _size - INT_LENGTH - 2;
	write()


	if (_size > INT_LENGTH * 3 && _size % 4 == 0 && _wpos <= _size - INT_LENGTH) {
		

		int n = _size / 4 - 2;
		btea((uint32_t*)(_data + INT_LENGTH), n, key);
		_wpos = _size - INT_LENGTH;
		write_uint(rand);
	}


	if (_wpos - _rpos > INT_LENGTH && _size - _wpos >= 1) {
		char rand = random::gen();
		char key[7];
		for (int i = 0; i < sizeof(s_key); i++) {
			key[i] = rand ^ s_key[i];
		}
		
		for (int i = _rpos + INT_LENGTH; i < _wpos; i++) {
			_data[i] ^= key[(i - INT_LENGTH - _rpos) % 7];
			char v = _data[i];
			char v1 = (char((v & 0xF)) << 8) | (char(v & 0xF0) >> 8);
			char v2 = (char(v1 & 0xF) << 8) | (char(v1 & 0xF0) >> 8);
			printf("%d", v2);
			_data[i] = v1 ^ key;
			key = v ^ key;
			//key = 
		}

		_data[_wpos] = rand;
		_wpos += 1;*/
	//}
}

void buffer::decrypt() {
	return;
	//_rpos = _size - INT_LENGTH;
	//uint32_t rand = read_uint();
	//uint32_t key[4];
	//for (int i = 0; i < 4; i++) {
	//	key[i] = s_key[i] ^ rand;
	//}

	//btea((uint32_t*)_data, -(_size - INT_LENGTH) / 4, key);
	
	_rpos = _size - 2;
	uint16_t len = read_ushort();
	printf("x + x + x +  ----------  de len: %d\n", len);

	_rpos = 0;
	_wpos = len;

	/*if (_wpos - _rpos >= 2 * INT_LENGTH) {
		_rpos = _size - INT_LENGTH;
		uint32_t rand = read_uint();
		_rpos = 0;

		uint32_t key[4];
		for (int i = 0; i < 4; i++) {
			key[i] = s_key[i] ^ rand;
		}

		btea((uint32_t*)_data, 1 - (_wpos - _rpos) / 4, key);
		_wpos = len;
	}


	if (_wpos - _rpos > 1) {
		char rand = _data[_wpos - 1];
		_wpos -= 1;

		char key[7];
		for (int i = 0; i < sizeof(s_key); i++) {
			key[i] = rand ^ s_key[i];
		}

		for (int i = _rpos; i < _wpos; i++) {
			_data[i] ^= key[(i - _rpos) % 7];
			char v = _data[i];
			v ^= key;
			v = ((v & 0xF) << 8) | ((v & 0xF0) >> 8);
			_data[i] = v;
			key = v ^ key;
		}
	}*/
}