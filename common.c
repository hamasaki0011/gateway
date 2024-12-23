/*
 * Copyright (c) 2024
 */

#include "common.h"

// 2024.12.23 Done.
uint16_t common_bytes_to_uint16_t(const uint8_t* bytes) {
    return (uint16_t)bytes[0] << 8 | (uint16_t)bytes[1];
}

int16_t common_bytes_to_int16_t(const uint8_t* bytes) {
    return (int16_t)common_bytes_to_uint16_t(bytes);
}

uint32_t common_bytes_to_uint32_t(const uint8_t* bytes) {
    return (uint32_t)bytes[0] << 24 | (uint32_t)bytes[1] << 16 |
           (uint32_t)bytes[2] << 8 | (uint32_t)bytes[3];
}

int32_t common_bytes_to_int32_t(const uint8_t* bytes) {
    return (int32_t)common_bytes_to_uint32_t(bytes);
}

float common_bytes_to_float(const uint8_t* bytes) {
    union {
        uint32_t u32_value;
        float float32;
    } tmp;

    tmp.u32_value = common_bytes_to_uint32_t(bytes);
    return tmp.float32;
}

void common_uint32_t_to_bytes(const uint32_t value, uint8_t* bytes) {
    bytes[0] = value >> 24;
    bytes[1] = value >> 16;
    bytes[2] = value >> 8;
    bytes[3] = value;
}

void common_uint16_t_to_bytes(const uint16_t value, uint8_t* bytes) {
    bytes[0] = value >> 8;
    bytes[1] = value;
}

void common_int32_t_to_bytes(const int32_t value, uint8_t* bytes) {
    bytes[0] = value >> 24;
    bytes[1] = value >> 16;
    bytes[2] = value >> 8;
    bytes[3] = value;
}

void common_int16_t_to_bytes(const int16_t value, uint8_t* bytes) {
    bytes[0] = value >> 8;
    bytes[1] = value;
}

void common_float_to_bytes(const float value, uint8_t* bytes) {
    union {
        uint32_t u32_value;
        float float32;
    } tmp;
    tmp.float32 = value;
    common_uint32_t_to_bytes(tmp.u32_value, bytes);
}

void common_copy_bytes(const uint8_t* source, uint8_t* destination,
                                 uint16_t data_length) {
    uint16_t i;
    for (i = 0; i < data_length; i++) {
        destination[i] = source[i];
    }
}
