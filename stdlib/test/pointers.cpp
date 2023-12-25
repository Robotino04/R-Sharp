#include <stdint.h>

extern "C" void set_i8_at_address(int8_t* address, int8_t value) {
    *address = value;
}

extern "C" void set_i16_at_address(int16_t* address, int16_t value) {
    *address = value;
}

extern "C" void set_i32_at_address(int32_t* address, int32_t value) {
    *address = value;
}

extern "C" void set_i64_at_address(int64_t* address, int64_t value) {
    *address = value;
}