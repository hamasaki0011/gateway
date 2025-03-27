/*Copyright (c) 2024 */
#ifndef DEVICE_H
#define DEVICE_H

#include "main.h"
#include "common.h"

#define COMMAND_SIZE        2
#define WORD_SIZE           2
#define NUM_WORDS(x)        (sizeof(x) / WORD_SIZE)
#define MAX_BUFFER_WORDS    32

#define NO_ERROR            0
#define I2C_WRITE_FAILED    -1
#define I2C_READ_FAILED     -2

#define CRC_ERROR           1
#define I2C_BUS_ERROR       2
#define I2C_NACK_ERROR      3
#define BYTE_NUM_ERROR      4
#define CRC8_POLYNOMIAL     0x31
#define CRC8_INIT           0xFF
#define CRC8_LEN            1

/** Initialize all hard- and software components that are needed for the I2C communication. */
void i2c_hal_init(void);
/** Release all resources initialized by sensirion_i2c_hal_init(). */
void i2c_hal_free(void);

/** Execute one write transaction on the I2C bus, sending a given number of bytes.
 * The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be returned.
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise */
int8_t i2c_hal_write(uint8_t, const uint8_t*, uint16_t);
// int8_t i2c_hal_write(uint8_t address, const uint8_t* data, uint16_t count);

int8_t i2c_cmd_write(uint16_t);

/** DeviceReset() - Executes a reset on the device.
 * @return 0 on success, an error code otherwise */
int16_t DeviceReset(void);

/** GetDeviceMarking() - Read the device marking string from the device.
 * @param device_marking ASCII string containing the serial number. The string
 * has the null-termination character included and is at most 32 bytes long.
 * @return 0 on success, an error code otherwise */
int16_t GetDeviceMarking(unsigned char* device_marking, uint8_t device_marking_size);

/** StartContinuousMeasurement() - Starts continuous measurement in polling mode.
 * @note This command is only available in idle mode.
 * @return 0 on success, an error code otherwise */
int16_t StartContinuousMeasurement(void);

/** ReadMeasuredValues() - Returns the new measurement results as float.
 * @param hcho Formaldehyde concentration in ppb.
 * @param humidity Relative humidity in % RH.
 * @param temperature Temperature in degrees Celsius.
 * @return 0 on success, an error code otherwise */
int16_t ReadMeasuredValues(float* hcho, float* humidity, float* temperature);
int8_t BlankRead(void);
// SDATA ReadMeasure(SDATA r);

/**
 * sfa3x_stop_measurement() - Stops the measurement mode and returns to idle
 * mode.
 * @note This command is only available in measurement mode.
 * @return 0 on success, an error code otherwise */
int16_t StopMeasurement(void);

/** Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be returned.
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise */
int8_t i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count);

uint8_t i2c_generate_crc(const uint8_t* data, uint16_t count);

int8_t i2c_check_crc(const uint8_t* data, uint16_t count, uint8_t checksum);

/** i2c_read_data_inplace() - Reads data from the Sensor.
 * @param address              Sensor I2C address
 * @param buffer               Allocated buffer to store data as bytes. Needs
 *                             to be big enough to store the data including
 *                             CRC. Twice the size of data should always suffice.
 * @param expected_data_length Number of bytes to read (without CRC). Needs
 *                             to be a multiple of WORD_SIZE,
 *                             otherwise the function returns BYTE_NUM_ERROR.
 * @return            NO_ERROR on success, an error code otherwise */
int16_t ReadDataInplace(uint8_t address, uint8_t* buffer, uint16_t expected_data_length);



// /** i2c_fill_cmd_send_buf() - create the i2c send buffer for a command
//  * and a set of argument words. The output buffer interleaves argument words
//  * with their checksums.
//  * @buf:        The generated buffer to send over i2c. Then buffer length must
//  *              be at least COMMAND_LEN + num_args *
//  *              (WORD_SIZE + CRC8_LEN).
//  * @cmd:        The i2c command to send. It already includes a checksum.
//  * @args:       The arguments to the command. Can be NULL if none.
//  * @num_args:   The number of word arguments in args.
//  * @return      The number of bytes written to buf. */
// uint16_t i2c_fill_cmd_send_buf(uint8_t* buf, uint16_t cmd, const uint16_t* args, uint8_t num_args);

// /** i2c_read_words_as_bytes() - read data words as byte-stream from sensor
//  * Read bytes without adjusting values to the uP's word-order.
//  * @address:    Sensor i2c address
//  * @data:       Allocated buffer to store the read bytes. The buffer may also have been modified in case of an error.
//  * @num_words:  Number of data words(!) to read (without CRC bytes)
//  *              Since only word-chunks can be read from the sensor the size
//  *              is still specified in sensor-words (num_words = num_bytes *WORD_SIZE)
//  * @return      NO_ERROR on success, an error code otherwise */
// int16_t i2c_read_words_as_bytes(uint8_t address, uint8_t* data, uint16_t num_words);

#endif /* DEVICE_H */
