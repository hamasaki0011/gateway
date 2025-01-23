/*
 * Copyright (c) 2024
 */

#ifndef COMMON_H
#define COMMON_H

//#include <stdlib.h>
/**
 * #ifndef NULL
 * #define NULL ((void *)0)
 * #endif
 */
 
#include <stdint.h>
/** Typedef section for types commonly defined in <stdint.h>
 * If your system does not provide stdint headers, please define them
 * accordingly. Please make sure to define int64_t and uint64_t. */
//typedef unsigned int sizet;
/**
 * typedef unsigned long long int uint64_t;
 * typedef long long int int64_t;
 * typedef long int32_t;
 * typedef unsigned long uint32_t;
 * typedef short int16_t;
 * typedef unsigned short uint16_t;
 * typedef char int8_t;
 * typedef unsigned char uint8_t; */

#define NO_ERROR 0
#define NOT_IMPLEMENTED_ERROR 31

/*
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif
*/
/** bytes_to_uint16_t() - Convert an array of bytes to an uint16_t
 * Convert an array of bytes received from the sensor in big-endian/MSB-first 
 * format to an uint16_t value in the correct system-endianness.
 * @param bytes An array of at least two bytes (MSB first)
 * @return      The byte array represented as uint16_t */
uint16_t bytes_to_uint16_t(const uint8_t* bytes);

/** bytes_to_int16_t() - Convert an array of bytes to an int16_t
 * Convert an array of bytes received from the sensor in big-endian/MSB-first
 * format to an int16_t value in the correct system-endianness.
 * @param bytes An array of at least two bytes (MSB first)
 * @return      The byte array represented as int16_t */
int16_t bytes_to_int16_t(const uint8_t* bytes);

/** sensirion_common_bytes_to_uint32_t() - Convert an array of bytes to an uint32_t
 * Convert an array of bytes received from the sensor in big-endian/MSB-first
 * format to an uint32_t value in the correct system-endianness.
 * @param bytes An array of at least four bytes (MSB first)
 * @return      The byte array represented as uint32_t */
uint32_t bytes_to_uint32_t(const uint8_t* bytes);

/** sensirion_common_bytes _to_int32_t() - Convert an array of bytes to an int32_t
 * Convert an array of bytes received from the sensor in big-endian/MSB-first
 * format to an int32_t value in the correct system-endianness.
 * @param bytes An array of at least four bytes (MSB first)
 * @return      The byte array represented as int32_t */
int32_t bytes_to_int32_t(const uint8_t* bytes);

/** sensirion_common_bytes_to_float() - Convert an array of bytes to a float
 * Convert an array of bytes received from the sensor in big-endian/MSB-first
 * format to an float value in the correct system-endianness.
 * @param bytes An array of at least four bytes (MSB first)
 * @return      The byte array represented as float */
float bytes_to_float(const uint8_t* bytes);

/** sensirion_common_uint32_t_to_bytes() - Convert an uint32_t to an array of bytes
 * Convert an uint32_t value in system-endianness to big-endian/MBS-first
 * format to send to the sensor.
 * @param value Value to convert
 * @param bytes An array of at least four bytes */
void uint32_t_to_bytes(const uint32_t value, uint8_t* bytes);

/** sensirion_common_int32_t_to_bytes() - Convert an int32_t to an array of bytes
 * Convert an int32_t value in system-endianness to big-endian/MBS-first
 * format to send to the sensor.
 * @param value Value to convert
 * @param bytes An array of at least four bytes */
void int32_t_to_bytes(const int32_t value, uint8_t* bytes);

/** sensirion_common_uint16_t_to_bytes() - Convert an uint16_t to an array of bytes
 * Convert an uint16_t value in system-endianness to big-endian/MBS-first
 * format to send to the sensor.
 * @param value Value to convert
 * @param bytes An array of at least two bytes */
void uint16_t_to_bytes(const uint16_t value, uint8_t* bytes);

/** sensirion_common_int16_t_to_bytes() - Convert an int16_t to an array of bytes
 * Convert an int16_t value in system-endianness to big-endian/MBS-first
 * format to send to the sensor.
 * @param value Value to convert
 * @param bytes An array of at least two bytes */
void int16_t_to_bytes(const int16_t value, uint8_t* bytes);

/** sensirion_common_float_to_bytes() - Convert an float to an array of bytes
 * Convert an float value in system-endianness to big-endian/MBS-first
 * format to send to the sensor.
 * @param value Value to convert
 * @param bytes An array of at least four bytes */
void float_to_bytes(const float value, uint8_t* bytes);

/** sensirion_common_copy_bytes() - Copy bytes from one array to the other.
 * @param source      Array of bytes to be copied.
 * @param destination Array of bytes to be copied to.
 * @param data_length Number of bytes to copy. */
void copy_bytes(const uint8_t* source, uint8_t* destination, uint16_t data_length);

#endif /* COMMON_H */
