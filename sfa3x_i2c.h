/*
 * Copyright (c) 2024
 */

#ifndef SFA3X_I2C_H
#define SFA3X_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

/*
typedef struct{
    char *gasName;  // gasName
    char *humid;    // humidity
    char *temp;     // temperature
    
    float gas;
    float humidity;
    float temperature;
}Sensor_data;
*/
/**
 * sfa3x_start_continuous_measurement() - Starts continuous measurement in
 * polling mode.
 *
 * @note This command is only available in idle mode.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sfa3x_start_continuous_measurement(void);

/**
 * sfa3x_stop_measurement() - Stops the measurement mode and returns to idle
 * mode.
 *
 * @note This command is only available in measurement mode.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sfa3x_stop_measurement(void);

/**
 * sfa3x_read_measured_values_ticks() - Returns the new measurement results as
 * integers.
 *
 * @param hcho Formaldehyde concentration in ppb with a scaling of 5.
 *
 * @param humidity Relative humidity in % RH with a scaling of 100.
 *
 * @param temperature Temperature in degrees Celsius with a scaling of 200.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sfa3x_read_measured_values_ticks(int16_t* hcho, int16_t* humidity,
                                         int16_t* temperature);

/**
 * sfa3x_read_measured_values() - Returns the new measurement results as
 * float.
 *
 * @param hcho Formaldehyde concentration in ppb.
 *
 * @param humidity Relative humidity in % RH.
 *
 * @param temperature Temperature in degrees Celsius.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sfa3x_read_measured_values(float* hcho, float* humidity,
                                   float* temperature);

/**
 * sfa3x_get_device_marking() - Read the device marking string from the device.
 *
 * @param device_marking ASCII string containing the serial number. The string
 * has the null-termination character included and is at most 32 bytes long.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sfa3x_get_device_marking(unsigned char* device_marking,
                                 uint8_t device_marking_size);
                                 
int8_t SendData(int16_t sock, Sensor_data h);

/**
 * sfa3x_device_reset() - Executes a reset on the device.
 *
 * @return 0 on success, an error code otherwise
 */
int16_t sfa3x_device_reset(void);

Sensor_data ReadMeasure(Sensor_data r);

#ifdef __cplusplus
}
#endif

#endif /* SFA3X_I2C_H */
