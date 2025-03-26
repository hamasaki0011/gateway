/** This is a Raspberry Pi program controls sensor board via i2c.  */
#include <stdio.h>  // printf(), strcmp(), fopen()
#include <unistd.h> // for sleep()/usleep(), for close
#include <stdio.h>      // printf(), strcmp(), fopen()
#include <fcntl.h>      // open()
#include <sys/ioctl.h>  // ioctl()
#include <unistd.h>     // read(), write(), usleep()

#include "common.h"
#include "device.h"
#include "main.h"

/** Linux specific configuration. Adjust the following define to the device path of sensor. */
#define I2C_DEVICE_PATH "/dev/i2c-1"
#define I2C_ADDRESS 0x5D
/** The following define was taken from i2c-dev.h. Alternatively the header file
 * can be included. The define was added in Linux v3.10 and never changed since then.  */
#define I2C_SLAVE 0x0703
#define I2C_WRITE_FAILED -1
#define I2C_READ_FAILED -1

static int8_t i2c_device = -1;
static uint8_t i2c_address = 0;

int16_t DeviceReset(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    uint16_t command = 0xd304;
    
    i2c_device = open(I2C_DEVICE_PATH, O_RDWR);

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);

    error = i2c_hal_write(I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }
    usleep(100000);
    return NO_ERROR;
}

int16_t GetDeviceMarking(unsigned char* deviceMarking, uint8_t deviceMarking_size) {
    int16_t error;
    uint8_t buffer[48];
    uint16_t offset = 0;
    uint16_t command = 0xD060;

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);

    error = i2c_hal_write(I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }

    usleep(2000);

    error = ReadDataInplace(I2C_ADDRESS, &buffer[0], 32);
    if (error) {
        return error;
    }
    copy_bytes(&buffer[0], deviceMarking, deviceMarking_size);
    return NO_ERROR;
}

int16_t StartContinuousMeasurement(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    uint16_t command = 0x06;

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);
    
    error = i2c_hal_write(I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }
    usleep(1000);
    return NO_ERROR;    // NO_ERROR: 0 in common.h
}

int16_t ReadMeasuredValues(float* hcho, float* humidity, float* temperature) {
    int16_t error;
    int16_t hcho_ticks;
    int16_t humidity_ticks;
    int16_t temperature_ticks;
    uint8_t buffer[9];
    uint16_t offset = 0;
    uint16_t command = 0x327;
    
    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);

    //error = i2c_hal_write(I2C_ADDRESS, &buffer[0], offset);    
    //if (error) {
    if (i2c_hal_write(I2C_ADDRESS, &buffer[0], offset)) {
        // return error;
        return I2C_WRITE_FAILED;
    }
    usleep(5000);
    error = ReadDataInplace(I2C_ADDRESS, &buffer[0], 6);
    if (error) {
        return error;
    }    

    hcho_ticks = (uint16_t)buffer[0] << 8 | (uint16_t)buffer[1];
    humidity_ticks = (uint16_t)buffer[2] << 8 | (uint16_t)buffer[3];
    temperature_ticks = (uint16_t)buffer[4] << 8 | (uint16_t)buffer[5];

    *hcho = (float)hcho_ticks / 5.0f;
    *humidity = (float)humidity_ticks / 100.0f;
    *temperature = (float)temperature_ticks / 200.0f;

    return NO_ERROR;
}


int8_t BlankRead(){
    float data1 = 0.0, data2 = 0.0, data3 = 0.0;

    // It may be adjust the measurement interval around for 500ms
    usleep(500000); // Original software settings.
    if(ReadMeasuredValues(&data1, &data2, &data3) != 0){
        printf("Error: Failed to read sensor data\n");
        return -1;
    }
    return 0;    
}

int16_t StopMeasurement(void) {
    int16_t error;
    uint8_t buffer[2];
    uint16_t offset = 0;
    uint16_t command = 0x104;

    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);
    
    error = i2c_hal_write(I2C_ADDRESS, &buffer[0], offset);    
    if (error) {
        return error;
    }
    
    usleep(50000);
    return NO_ERROR;
}

int16_t ReadDataInplace(uint8_t address, uint8_t* buffer, uint16_t expected_data_length) {
    int16_t error;
    uint16_t i, j;
    uint16_t size = (expected_data_length / WORD_SIZE) * (WORD_SIZE + CRC8_LEN);

    if (expected_data_length % WORD_SIZE != 0) {
        return BYTE_NUM_ERROR;
    }

    // error = i2c_hal_read(address, buffer, size);
    // if (error) {
    if (i2c_hal_read(address, buffer, size)) {
        // return error;
        return I2C_READ_FAILED;
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


/** Execute one write transaction on the I2C bus, sending a given number of bytes. 
 * The bytes in the supplied buffer must be sent to the given address. 
 * If the slave device does not acknowledge any of the bytes, an error shall be returned.
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise. */
int8_t i2c_hal_write(uint8_t address, const uint8_t* data, uint16_t count) {
    if (i2c_address != address) {
        ioctl(i2c_device, I2C_SLAVE, address);
        i2c_address = address;
    }
    if (write(i2c_device, data, count) != count) {
        return I2C_WRITE_FAILED;
    }
    return 0;
}

/** Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be returned.
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise. */
int8_t i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count) {
    if (i2c_address != address) {
        ioctl(i2c_device, I2C_SLAVE, address);
        i2c_address = address;
    }
    if (read(i2c_device, data, count) != count) {
        return I2C_READ_FAILED;
    }
    return 0;
}

// 7 times @2024.12.23
uint8_t i2c_generate_crc(const uint8_t* data, uint16_t count) {
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;

    // calculates 8-Bit checksum with given polynomial
    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

// 2 times @2024.12.23
int8_t i2c_check_crc(const uint8_t* data, uint16_t count, uint8_t checksum) {                              
    if (i2c_generate_crc(data, count) != checksum) return CRC_ERROR;    
    return NO_ERROR;
}

// 3 times @2024.12.23
uint16_t i2c_fill_cmd_send_buf(uint8_t* buf, uint16_t cmd, const uint16_t* args, uint8_t num_args){
    uint8_t i;
    uint16_t idx = 0;

    buf[idx++] = (uint8_t)((cmd & 0xFF00) >> 8);
    buf[idx++] = (uint8_t)((cmd & 0x00FF) >> 0);
    for (i = 0; i < num_args; ++i) {
        buf[idx++] = (uint8_t)((args[i] & 0xFF00) >> 8);
        buf[idx++] = (uint8_t)((args[i] & 0x00FF) >> 0);
        uint8_t crc = i2c_generate_crc((uint8_t*)&buf[idx - 2], WORD_SIZE);
        buf[idx++] = crc;
    }
    return idx;
}

// 1 time @2024.13.23
int16_t i2c_read_words_as_bytes(uint8_t address, uint8_t* data, uint16_t num_words) {
    int16_t ret;
    uint16_t i, j;
    uint16_t size = num_words * (WORD_SIZE + CRC8_LEN);
    uint16_t word_buf[MAX_BUFFER_WORDS];
    uint8_t* const buf8 = (uint8_t*)word_buf;

    ret = i2c_hal_read(address, buf8, size);
    if (ret != NO_ERROR)
        return ret;

    /* check the CRC for each word */
    for (i = 0, j = 0; i < size; i += WORD_SIZE + CRC8_LEN) {
        ret = i2c_check_crc(&buf8[i], WORD_SIZE, buf8[i + WORD_SIZE]);
        if (ret != NO_ERROR) return ret;
        data[j++] = buf8[i];
        data[j++] = buf8[i + 1];
    }
    return NO_ERROR;
}

// 1 time @2024.12.23
int16_t i2c_read_words(uint8_t address, uint16_t* data_words, uint16_t num_words) {
    int16_t ret;
    uint8_t i;

    ret = i2c_read_words_as_bytes(address, (uint8_t*)data_words, num_words);
    if (ret != NO_ERROR) return ret;

    for (i = 0; i < num_words; ++i) {
        const uint8_t* word_bytes = (uint8_t*)&data_words[i];
        data_words[i] = ((uint16_t)word_bytes[0] << 8) | word_bytes[1];
    }
    return NO_ERROR;
}


// 1 time @2024.12.23
int16_t i2c_delayed_read_cmd(uint8_t address, uint16_t cmd, uint32_t delay_us, uint16_t* data_words, uint16_t num_words) {
    int16_t ret;
    uint8_t buf[COMMAND_SIZE];

    i2c_fill_cmd_send_buf(buf, cmd, NULL, 0);
    ret = i2c_hal_write(address, buf, COMMAND_SIZE);
    if (ret != NO_ERROR) return ret;
    if (delay_us) usleep(delay_us);
    return i2c_read_words(address, data_words, num_words);
}

// 1 time @2024.12.23
int16_t i2c_read_cmd(uint8_t address, uint16_t cmd, uint16_t* data_words, uint16_t num_words) {
    return i2c_delayed_read_cmd(address, cmd, 0, data_words, num_words);
}

// 1 time @2024.12.23
uint16_t i2c_add_uint32_t_to_buffer(uint8_t* buffer, uint16_t offset, uint32_t data) {
    buffer[offset++] = (uint8_t)((data & 0xFF000000) >> 24);
    buffer[offset++] = (uint8_t)((data & 0x00FF0000) >> 16);
    buffer[offset] = i2c_generate_crc(&buffer[offset - WORD_SIZE], WORD_SIZE);
    offset++;
    buffer[offset++] = (uint8_t)((data & 0x0000FF00) >> 8);
    buffer[offset++] = (uint8_t)((data & 0x000000FF) >> 0);
    buffer[offset] = i2c_generate_crc(&buffer[offset - WORD_SIZE], WORD_SIZE);
    offset++;
    return offset;
}

