/*
 * Copyright (c) 2024
 */

#ifndef SFA3X_H
#define SFA3X_H

#include "define.h"
#include "common.h"

/**
 * sfa3x_start_continuous_measurement() - Starts continuous measurement in
 * polling mode.
 * @note This command is only available in idle mode.
 * @return 0 on success, an error code otherwise */
int16_t StartContinuousMeasurement(void);

/**
 * sfa3x_stop_measurement() - Stops the measurement mode and returns to idle
 * mode.
 * @note This command is only available in measurement mode.
 * @return 0 on success, an error code otherwise */
int16_t StopMeasurement(void);

/**
 * sfa3x_read_measured_values_ticks() - Returns the new measurement results as integers.
 * @param hcho Formaldehyde concentration in ppb with a scaling of 5.
 * @param humidity Relative humidity in % RH with a scaling of 100.
 * @param temperature Temperature in degrees Celsius with a scaling of 200.
 * @return 0 on success, an error code otherwise */
int16_t ReadMeasuredValuesTicks(int16_t* hcho, int16_t* humidity, int16_t* temperature);

/**
 * sfa3x_read_measured_values() - Returns the new measurement results as float.
 * @param hcho Formaldehyde concentration in ppb.
 * @param humidity Relative humidity in % RH.
 * @param temperature Temperature in degrees Celsius.
 * @return 0 on success, an error code otherwise */
int16_t ReadMeasuredValues(float* hcho, float* humidity, float* temperature);

/**
 * sfa3x_get_device_marking() - Read the device marking string from the device.
 * @param device_marking ASCII string containing the serial number. The string
 * has the null-termination character included and is at most 32 bytes long.
 * @return 0 on success, an error code otherwise */
int16_t GetDeviceMarking(unsigned char* device_marking, uint8_t device_marking_size);
                                 
int8_t SendData(int16_t sock, Sensor_data h);

/**
 * sfa3x_device_reset() - Executes a reset on the device.
 * @return 0 on success, an error code otherwise */
int16_t DeviceReset(void);

Sensor_data ReadMeasure(Sensor_data r);


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

#endif /* SFA3X_H */
