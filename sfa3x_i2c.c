/*
 * This is software which controls sfa3x board via i2c from Raspberry Pi. 
 */
#include <stdio.h>  // printf(), strcmp(), fopen()
#include <unistd.h> // for sleep()/usleep(), for close
#include "sfa3x_i2c.h"
#include "common.h"
#include "i2c.h"
//#include "i2c_hal.h"

#define SFA3X_I2C_ADDRESS 0x5D

// 2024.11.11 main_#67 call.
int16_t sfa3x_start_continuous_measurement(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    offset = i2c_add_command_to_buffer(&buffer[0], offset, 0x06);
    //offset = sensirion_i2c_add_command_to_buffer(&buffer[0], offset, 0x06);
    error = i2c_write_data(SFA3X_I2C_ADDRESS, &buffer[0], offset);
    //error = sensirion_i2c_write_data(SFA3X_I2C_ADDRESS, &buffer[0], offset);
    if (error) {
        return error;
    }
    //sensirion_i2c_hal_sleep_usec(1000);
    usleep(1000);
    return NO_ERROR;    // NO_ERROR: 0 in common.h
}

int16_t sfa3x_stop_measurement(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    //offset = sensirion_i2c_add_command_to_buffer(&buffer[0], offset, 0x104);
    offset = i2c_add_command_to_buffer(&buffer[0], offset, 0x104);
    //error = sensirion_i2c_write_data(SFA3X_I2C_ADDRESS, &buffer[0], offset);
    error = i2c_write_data(SFA3X_I2C_ADDRESS, &buffer[0], offset);
    if (error) {
        return error;
    }
    //sensirion_i2c_hal_sleep_usec(50000);
    usleep(50000);
    return NO_ERROR;
}

// 2024.11.14 sfa3x_i2c_#99 call.
int16_t sfa3x_read_measured_values_ticks(int16_t* hcho, int16_t* humidity, int16_t* temperature) {
    int16_t error;
    uint8_t buffer[9];
    uint16_t offset = 0;
    
    offset = i2c_add_command_to_buffer(&buffer[0], offset, 0x327);
    error = i2c_write_data(SFA3X_I2C_ADDRESS, &buffer[0], offset);
    if (error) {
        return error;
    }
    usleep(5000);
    //error = sensirion_i2c_read_data_inplace(SFA3X_I2C_ADDRESS, &buffer[0], 6);
    error = i2c_read_data_inplace(SFA3X_I2C_ADDRESS, &buffer[0], 6);
    if (error) {
        return error;
    }
    *hcho = sensirion_common_bytes_to_int16_t(&buffer[0]);
    *humidity = sensirion_common_bytes_to_int16_t(&buffer[2]);
    *temperature = sensirion_common_bytes_to_int16_t(&buffer[4]);
    return NO_ERROR;
}

// 2024.11.14 sfa3x_i2c_#158 call.
int16_t sfa3x_read_measured_values(float* hcho, float* humidity, float* temperature) {
    int16_t error;
    int16_t hcho_ticks;
    int16_t humidity_ticks;
    int16_t temperature_ticks;

    error = sfa3x_read_measured_values_ticks(&hcho_ticks, &humidity_ticks, &temperature_ticks);
    if (error) {
        return error;
    }

    *hcho = (float)hcho_ticks / 5.0f;
    *humidity = (float)humidity_ticks / 100.0f;
    *temperature = (float)temperature_ticks / 200.0f;

    return NO_ERROR;
}

// 2024.11.11 main_#61 call.
int16_t sfa3x_get_device_marking(unsigned char* device_marking, uint8_t device_marking_size) {
    int16_t error;
    uint8_t buffer[48];
    uint16_t offset = 0;
    /** i2c_add_command_to_buffer is #10 in i2c.c  **/
    offset = i2c_add_command_to_buffer(&buffer[0], offset, 0xD060);
    error = i2c_write_data(SFA3X_I2C_ADDRESS, &buffer[0], offset);
    if (error) {
        return error;
    }

    usleep(2000);

    //error = sensirion_i2c_read_data_inplace(SFA3X_I2C_ADDRESS, &buffer[0], 32);
    error = i2c_read_data_inplace(SFA3X_I2C_ADDRESS, &buffer[0], 32);
    if (error) {
        return error;
    }
    sensirion_common_copy_bytes(&buffer[0], device_marking,
                                device_marking_size);
    return NO_ERROR;
}

// 2024.11.15 main_#54 call.
int16_t sfa3x_device_reset(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    uint16_t command = 0xd304;
    
    // i2c.c_#175 i2c_write_data()
    offset = i2c_add_command_to_buffer(&buffer[0], offset, command);
    // i2c.c_# i2c_write_data()
    error = i2c_write_data(SFA3X_I2C_ADDRESS, &buffer[0], offset);
    if (error) {
        return error;
    }
    usleep(100000);
    return NO_ERROR;
}

// 2024.11.14 Almost done main_#102 call.
Sensor_data ReadMeasure(Sensor_data r){
    float data1, data2, data3;
    
    usleep(500000);
    // read and store data: sfa3x_i2c.c
    if(sfa3x_read_measured_values(&data1, &data2, &data3) != 0){
        r.gas = 0.0;
        r.humidity = 0.0;
        r.temperature = 0.0;
        printf("Failed to read measure data form Sensor\n");
    }
    else{
        r.gas = data1;
        r.humidity = data2;
        r.temperature = data3;
    }    
    return r;
}


