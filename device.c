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
#define I2C_ADDRESS     0x5D
/** The following define was taken from i2c-dev.h. Alternatively the header file
 * can be included. The define was added in Linux v3.10 and never changed since then.  */
#define I2C_SLAVE       0x0703

// Descriptor
static int8_t i2c_device = -1;
static uint8_t i2c_address = 0;

/** Initialize all hard- and software components that are needed for the I2C communication. */
void i2c_hal_init(void) {
    /// open i2c adapter
    i2c_device = open(I2C_DEVICE_PATH, O_RDWR);
    if (i2c_device == -1) return; // no error handling.
    return;
}

/** Release all resources initialized */
void i2c_hal_free(void) {
    /// close i2c adapter
    if (i2c_device >= 0) close(i2c_device);
    return;
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

    if (write(i2c_device, data, count) != count) return I2C_WRITE_FAILED;

    return NO_ERROR;
}

int8_t i2c_cmd_write(uint16_t cmd)
{
    uint8_t buffer[2];
    uint16_t offset = 0;
    
    buffer[offset++] = (uint8_t)((cmd & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((cmd & 0x00FF) >> 0);
    if (i2c_hal_write(I2C_ADDRESS, &buffer[0], offset)) return I2C_WRITE_FAILED;
    
    return NO_ERROR; 
}


int8_t DeviceReset(void) {
    uint16_t command = 0xd304;
    
    i2c_hal_init();

    if(i2c_cmd_write(command)) return I2C_WRITE_FAILED;

    usleep(100000);
    
    return NO_ERROR;
}

int8_t GetDeviceMarking(unsigned char* deviceMarking, uint8_t deviceMarking_size) {
    int16_t error = NO_ERROR;
    uint8_t buffer[48];
    uint16_t command = 0xD060;

    if(i2c_cmd_write(command)) return I2C_WRITE_FAILED;

    usleep(2000);

    error = ReadDataInplace(I2C_ADDRESS, &buffer[0], 32);
    if (error) {
        return error;
    }
    copy_bytes(&buffer[0], deviceMarking, deviceMarking_size);

    return error;
}

int8_t StartContinuousMeasurement(void) {
    uint16_t command = 0x06;

    if(i2c_cmd_write(command)) return I2C_WRITE_FAILED;

    usleep(1000);
    
    return NO_ERROR;
}

/** [MEMO] Read procedure for Sensirion sensor' data
 * For Sensirion's sensor
 * The structures of the Result
 * char *gasName;       // gas name
 * char *humid;         // humidity
 * char *temp;          // temperature
 * float gas;           // gas concentration value
 * float humidity;      // humidity value
 * float temperature;   // temperature value */
        
// SDATA ReadMeasure(SDATA r){
//     float data1, data2, data3;
//     if(ReadMeasuredValues(&data1, &data2, &data3) != 0){
//         r.gas = 0.0;
//         r.humidity = 0.0;
//         r.temperature = 0.0;
//         printf("Failed to read Sensor data.\n");
//     }else{
//         r.gas = data1;
//         r.humidity = data2;
//         r.temperature = data3;
//     }    
//     return r;
// }
int8_t ReadMeasuredValues(float* data1, float* data2, float* data3) {
    int8_t error = NO_ERROR;
    uint8_t buffer[9];
    uint16_t command = 0x327;
    
    if(i2c_cmd_write(command)) return I2C_WRITE_FAILED;

    usleep(5000);

    error = ReadDataInplace(I2C_ADDRESS, &buffer[0], 6);
    if (error) {
        return error;
    }    

    *data1 = (float)((uint16_t)buffer[0] << 8 | (uint16_t)buffer[1]) / 5.0f;
    *data2 = (float)((uint16_t)buffer[2] << 8 | (uint16_t)buffer[3]) / 100.0f;
    *data3 = (float)((uint16_t)buffer[4] << 8 | (uint16_t)buffer[5]) / 200.0f;

    return error;
}

int8_t BlankRead(){
    float data1 = 0.0, data2 = 0.0, data3 = 0.0;
    int8_t error = NO_ERROR;

    // It may need to adjust around 500ms as the wait time.
    usleep(500000); // Original software settings.

    error = ReadMeasuredValues(&data1, &data2, &data3);
    if(error){
        printf("Error: Failed to read sensor data\n");
        return error;
    }

    return error;
}

int8_t StopMeasurement(void) {
    uint16_t command = 0x104;

    if(i2c_cmd_write(command)) return I2C_WRITE_FAILED;
    
    usleep(50000);

    // And stop sensor operation.
    i2c_hal_free();

    return NO_ERROR;
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

    if (read(i2c_device, data, count) != count) return I2C_READ_FAILED;
    
    return NO_ERROR;

}

// uint8_t i2c_generate_crc(const uint8_t* data, uint16_t count) {
//     uint16_t current_byte;
//     uint8_t crc = CRC8_INIT;
//     uint8_t crc_bit;

//     // calculates 8-Bit checksum with given polynomial
//     for (current_byte = 0; current_byte < count; ++current_byte) {
//         crc ^= (data[current_byte]);
//         // printf("device_#211 original is %02x\n", data[current_byte]);
//         // printf("device_#212 crc_before is %02x\n", crc);
//         for (crc_bit = 8; crc_bit > 0; --crc_bit) {
//             if (crc & 0x80){
//                 crc = (crc << 1) ^ CRC8_POLYNOMIAL;
//                 // printf("device_#216 crc1 is %02x\n", crc);
//             }
//             else{
//                 crc = (crc << 1);
//                 // printf("device_#220 crc0 is %02x\n", crc);
//             }
//             //printf("device_#218 crc_n is %02x\n", crc);
//         }
//         // printf("device_#219 crc is %02x\n", crc);
//     }
//     return crc;
// }

// 2 times @2024.12.23
int8_t i2c_check_crc(const uint8_t* data, uint16_t count, uint8_t checksum) {                              
    // if (i2c_generate_crc(data, count) != checksum) return CRC_ERROR;
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;

    printf("device_#235 data: %hhn, count: %d, checksum: %02x\n", data, count, checksum);
    // calculates 8-Bit checksum with given polynomial
    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= (data[current_byte]);
        printf("device_#239 original is %02x\n", data[current_byte]);
        printf("device_#240 crc_before is %02x\n\n", crc);
        
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if (crc & 0x80){
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
                printf("device_#245 crc1 is %02x\n", crc);
            }
            else{
                crc = (crc << 1);
                printf("device_#249 crc0 is %02x\n", crc);
            }
            printf("device_#251 crc_n is %02x\n\n", crc);
        }
        printf("device_#253 crc is %02x\n", crc);
    }
    if(crc != checksum) return CRC_ERROR;

    return NO_ERROR;
}

int8_t ReadDataInplace(uint8_t address, uint8_t* buffer, uint16_t expected_data_length) {
    uint16_t i, j;
    uint16_t size = (expected_data_length / WORD_SIZE) * (WORD_SIZE + CRC8_LEN);

    if (expected_data_length % WORD_SIZE != 0) return BYTE_NUM_ERROR;
    if (i2c_hal_read(address, buffer, size)) return I2C_READ_FAILED;

    for (i = 0, j = 0; i < size; i += WORD_SIZE + CRC8_LEN) {

        if (i2c_check_crc(&buffer[i], WORD_SIZE, buffer[i + WORD_SIZE])) return CRC_ERROR;

        buffer[j++] = buffer[i];
        buffer[j++] = buffer[i + 1];

    }

    return NO_ERROR;
}




