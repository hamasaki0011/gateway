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
 * the slave device does not acknowledge any of the bytes, an error shall be returned. */
int8_t i2c_hal_write(uint8_t, const uint8_t*, uint16_t);

int8_t i2c_cmd_write(uint16_t);

/** DeviceReset() - Executes a reset on the device. */
int16_t DeviceReset(void);

/** GetDeviceMarking() - Read the device marking string from the device. */
int16_t GetDeviceMarking(unsigned char*, uint8_t);

/** StartContinuousMeasurement() - Starts continuous measurement in polling mode. */
int16_t StartContinuousMeasurement(void);

/** ReadMeasuredValues() - Returns the new measurement results as float. */
int16_t ReadMeasuredValues(float*, float*, float*);

int8_t BlankRead(void);

/** Stop_measurement() - Stops the measurement mode and returns to idle */
int16_t StopMeasurement(void);

/** Execute one read transaction on the I2C bus, reading a given number of bytes. */
int8_t i2c_hal_read(uint8_t, uint8_t*, uint16_t);

uint8_t i2c_generate_crc(const uint8_t*, uint16_t);

int8_t i2c_check_crc(const uint8_t*, uint16_t, uint8_t);

/** i2c_read_data_inplace() - Reads data from the Sensor. */
int16_t ReadDataInplace(uint8_t, uint8_t*, uint16_t);

#endif /* DEVICE_H */
