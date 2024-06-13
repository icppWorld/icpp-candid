#include <cstdint>
#include "icpp_hash_library_utils.h"


// Utility function to check if we are running on little-endian
bool icpp_hash_library_utils::is_little_endian() {
  // c++17 version
  uint16_t number = 0x1;
  uint8_t *byte_ptr = reinterpret_cast<uint8_t*>(&number);
  return byte_ptr[0] == 0x1;
}

// Utility function to check if we are running on big-endian
bool icpp_hash_library_utils::is_big_endian() {
  // c++17 version
  uint16_t number = 0x1;
  uint8_t *byte_ptr = reinterpret_cast<uint8_t*>(&number);
  return byte_ptr[0] == 0x0;
}