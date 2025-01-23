/*
 * This is software which controls sfa3x board via i2c from Raspberry Pi. 
 */
#include <stdio.h>  // printf(), strcmp(), fopen()
#include <unistd.h> // for sleep()/usleep(), for close
#include "sfa3x.h"
#include "define.h"
#include "common.h"
#include "i2c.h"

#define SFA3X_I2C_ADDRESS 0x5D

// 2024.11.11 main_#67 call.
int16_t StartContinuousMeasurement(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    uint16_t command = 0x06;

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);
    
    error = i2c_hal_write(SFA3X_I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }
    usleep(1000);
    return NO_ERROR;    // NO_ERROR: 0 in common.h
}

int16_t StopMeasurement(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    uint16_t command = 0x104;

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);
    
    error = i2c_hal_write(SFA3X_I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }
    usleep(50000);
    return NO_ERROR;
}

int16_t ReadMeasuredValuesTicks(int16_t* hcho, int16_t* humidity, int16_t* temperature) {
    int16_t error;
    uint8_t buffer[9];
    uint16_t offset = 0;
    uint16_t command = 0x327;
    
    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);

    error = i2c_hal_write(SFA3X_I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }
    usleep(5000);
    error = ReadDataInplace(SFA3X_I2C_ADDRESS, &buffer[0], 6);
    if (error) {
        return error;
    }
    *hcho = bytes_to_int16_t(&buffer[0]);
    *humidity = bytes_to_int16_t(&buffer[2]);
    *temperature = bytes_to_int16_t(&buffer[4]);
    return NO_ERROR;
}

int16_t ReadMeasuredValues(float* hcho, float* humidity, float* temperature) {
    int16_t error;
    int16_t hcho_ticks;
    int16_t humidity_ticks;
    int16_t temperature_ticks;

    error = ReadMeasuredValuesTicks(&hcho_ticks, &humidity_ticks, &temperature_ticks);
    if (error) {
        return error;
    }

    *hcho = (float)hcho_ticks / 5.0f;
    *humidity = (float)humidity_ticks / 100.0f;
    *temperature = (float)temperature_ticks / 200.0f;

    return NO_ERROR;
}

int16_t GetDeviceMarking(unsigned char* device_marking, uint8_t device_marking_size) {
    int16_t error;
    uint8_t buffer[48];
    uint16_t offset = 0;
    uint16_t command = 0xD060;

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);

    error = i2c_hal_write(SFA3X_I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }

    usleep(2000);

    error = ReadDataInplace(SFA3X_I2C_ADDRESS, &buffer[0], 32);
    if (error) {
        return error;
    }
    copy_bytes(&buffer[0], device_marking, device_marking_size);
    return NO_ERROR;
}

int16_t DeviceReset(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    uint16_t command = 0xd304;
    
    i2c_hal_init();

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);

    error = i2c_hal_write(SFA3X_I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }
    usleep(100000);
    return NO_ERROR;
}

Sensor_data ReadMeasure(Sensor_data r){
    float data1, data2, data3;
    
    //usleep(500000);
    if(ReadMeasuredValues(&data1, &data2, &data3) != 0){
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

int16_t ReadDataInplace(uint8_t address, uint8_t* buffer, uint16_t expected_data_length) {
    int16_t error;
    uint16_t i, j;
    uint16_t size = (expected_data_length / WORD_SIZE) * (WORD_SIZE + CRC8_LEN);

    if (expected_data_length % WORD_SIZE != 0) {
        return BYTE_NUM_ERROR;
    }

    error = i2c_hal_read(address, buffer, size);
    if (error) {
        return error;
    }

    for (i = 0, j = 0; i < size; i += WORD_SIZE + CRC8_LEN) {

        error = i2c_check_crc(&buffer[i], WORD_SIZE, buffer[i + WORD_SIZE]);
        if (error) {
            return error;
        }
        buffer[j++] = buffer[i];
        buffer[j++] = buffer[i + 1];
    }

    return NO_ERROR;
}
