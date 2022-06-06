#pragma once

#include <vector>
#include <string>

#define CRC64_POLY 0x9A6C9329AC4BC9B5ULL

namespace crc64
{
  uint64_t _crc64_table[256] = {0};
  bool _crc64_table_generated = false;

  uint64_t calc(const void *data_ptr, size_t data_size, uint64_t crc = 0)
  {
    const unsigned char *_p = (const unsigned char *)data_ptr;
  	size_t i;

  	crc = ~crc;

  	for (i = 0; i < data_size; i++)
  		crc = (crc >> 8) ^ _crc64_table[(crc & 0xff) ^ *_p++];

  	return ~crc;
  }

  uint64_t calc(const std::string &str, uint64_t crc = 0)
  {
    return calc(str.c_str(), str.length(), crc);
  }

  // uint64_t calc(const std::vector &vec, uint64_t crc = 0)
  // {
  //   return calc(vec.data(), vec.size(), crc);
  // }

  void generate()
  {
    if (_crc64_table_generated) return;

    uint64_t i, j, c, crc;

  	for (i = 0; i < 256; i++) {
  		crc = 0ULL;
  		c = i;

  		for (j = 0; j < 8; j++) {
  			if ((crc ^ (c >> j)) & 1)
  				crc = (crc >> 1) ^ CRC64_POLY;
  			else
  				crc >>= 1;
  		}
  		_crc64_table[i] = crc;
  	}
  }
}
