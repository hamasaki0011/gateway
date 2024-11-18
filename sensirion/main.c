#include <stdio.h>  // printf(), strcmp(), fopen()
#include <string.h> // memset()
#include <unistd.h>     // for sleep()/usleep(), for close
#include <time.h>

#include "sensirion_i2c_hal.h" 
#include "sfa3x_i2c.h"
#include "tcp_com.h"

/*
 * TO USE CONSOLE OUTPUT (PRINTF) YOU MAY NEED TO ADAPT THE INCLUDE ABOVE OR
 * DEFINE IT ACCORDING TO YOUR PLATFORM:
 * #define printf(...)
 */

//sensor error status -> sensErrStatus
#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
#define FAILED_START_SENSOR     30

uint8_t sensorStatus = 0;       // Sensor' statusoid
//extern uint8_t status;        // 
//uint8_t status = STOP;        // system status
unsigned char device_marking[32];

int dummyRead(){
    int8_t errCode = 0;
    float data1, data2, data3;
    Sensor_data result = {"ホルムアルデヒド濃度", "相対湿度", "周囲温度"};
        
    // it may adjust the measurement interval around for 500ms: sensirion_i2c_hal.c
    //sensirion_i2c_hal_sleep_usec(500000);
       
    /** read and store data: sfa3x_i2c.c **/
    errCode = sfa3x_read_measured_values(&data1, &data2, &data3);
    
    if(errCode != 0){
        printf("Failed to read measure data form Sensor\n");
        result.gas = 1.0;
        result.humidity = 1.0;
        result.temperature = 1.0;
    }
    else{
        result.gas = data1;
        result.humidity = data2;
        result.temperature = data3;
    }
    
    return 0;    
}

int whattimeisit(void){
    /*
    time_t timer;
    struct tm *local;
    int year, month, day, hour, minute, second;

    // Get the latest time
    timer = time(NULL);
    // Convert to localtime
    local = localtime(&timer);

    // 年月日と時分秒をtm構造体の各パラメタから変数に代入
    year = local->tm_year + 1900;   // 1900年からの年数が取得されるため
    month = local->tm_mon + 1;  // 0を1月としているため
    day = local->tm_mday;
    hour = local->tm_hour;
    minute = local->tm_min;
    second = local->tm_sec;
 
    // 現在の日時を表示
    printf("It's %d年%d月%d日 %d時%d分%d秒 o'clock now.\n", year, month, day, hour, minute, second);

    */
    return 0;
}

int main(void) {
    int8_t errorSensor = 0;
    int8_t waitPeriod = 60;
    time_t timer;
    struct tm *local;
    int year, month, day, hour, minute, second;
    //char resultData;
      
    /** Open i2c interface to connect sensirion formaldehyde sensor **/ 
    sensirion_i2c_hal_init();
    /** Make reset sensor hardware **/
    errorSensor = sfa3x_device_reset();
    if (errorSensor) {
        printf("Failed to initilize and rest Sensor: %i\n", errorSensor);
        sensorStatus = FAILED_MAKE_RESET;
        
    }
    
    /** Confirm device marking **/
    errorSensor = sfa3x_get_device_marking(&device_marking[0], sizeof(device_marking));
    if (errorSensor) {
        printf("Failed to get Device Marking: %i\n", errorSensor);
        sensorStatus = FAILED_GET_DEVICEMARK;
    }
    printf("main_#102 Welcome! Here is the serial code.: %s\n", device_marking);
    
    errorSensor = sfa3x_device_reset();
    if (errorSensor) {
        printf("Failed to initilize and rest Sensor: %i\n", errorSensor);
        sensorStatus = FAILED_MAKE_RESET;
    }
    
    errorSensor = sfa3x_start_continuous_measurement();
    
    if (errorSensor) {
        printf("Failed to execute sfa3x_start_continuous_measurement(): %i\n", errorSensor);
        //return 1;
    }else{
        printf("main_#116 Start sensing ... \n");
    }
    
    // wait around 200ms before starting to read data
    usleep(200000);
    dummyRead();

    /* main loop */
    for (;;) {
        int8_t errCode = 0;
        
        // wait around 10000ms before Sensor operating
        usleep(10000000);

        // 2023.11.24 Get the latest time
        timer = time(NULL);
        // Convert to localtime
        local = localtime(&timer);

        // 年月日と時分秒をtm構造体の各パラメタから変数に代入
        year = local->tm_year + 1900;   /* 1900年からの年数が取得されるため */
        month = local->tm_mon + 1;  /* 0を1月としているため */
        day = local->tm_mday;
        hour = local->tm_hour; 
        minute = local->tm_min;
        second = local->tm_sec;
 
        /* 現在の日時を表示 */
        printf("main_#144 %d年%d月%d日 %d時%d分%d秒\n", year, month, day, hour, minute, second);

        //int16_t send_size = 0;
        //int16_t error = 0;

        // 2023.11.24 Read measured data and display on terminal
        // 2024.11.12 ReadMeasure() fuction is in tcp_com.c
        ReadMeasure();
        
        FILE *fp = fopen("testWork.csv","a");
        if (fp == NULL){
            printf("It can NOT open.\n");
            return 1;
        }
        printf("main_#157 read_dummy read data1, read data2");
        //resultData = "Read data";
        fprintf(fp, "%s \n", "read_dummy read data1, read data2");
        
        fclose(fp);
        /*
         *2023.11.24
         This program does not use TCP/IP communication
        */
        
        /*
        // Generate a socket and connecting to server
        if(GenerateSocket() == 0){
            printf("Success to open socket.\n");
            // Transfer() loop
            do{
                // wait around 200ms before Sensor operating
                usleep(200000);
                // put here the command operation in transfer() function
                errCode = Transfer();
                // 次の接続要求の受け付けに移る //
            }while(errCode == 0);
            
            // (1)comStatus = FAILED_ACCEPT_REQUEST
            // (2)comStatus = FAILED_RECEIVE_REQUEST
            if(errCode == -1 || errCode == -3){
                printf("Acception or receiption error has occurred.. after a few minutes lator try again!!!\n");
                sleep((unsigned int)(waitPeriod));
            }
            // (3)comStatus = SHUT_DOWN_NETWORK it may cause by Server shut down
            else{
                // send stop command "0x01" to stop measurement to sfa30: sfa3x_i2c.c
                errSensor = sfa3x_stop_measurement();
                if(errSensor == 0){
                    printf("Stoped Sensing.\n");
                }
                else printf("Failed to stop Sensing.\n");
                
                printf("Try again after few minutes late.\n");
                sleep((unsigned int)(waitPeriod));
            }
        }else{
            printf("Failed to generate a socket.\n");
            printf("Try again after few minutes.\n");
            // send stop command "0x01" to stop measurement to sfa30: sfa3x_i2c.c
            errSensor = sfa3x_device_reset();
            if (errSensor) {
                printf("Failed to initilize and rest Sensor: %i\n", errSensor);
                sensorStatus = FAILED_MAKE_RESET;
            }
            printf("Waiting ... around 60s.\n");
            sleep((unsigned int)(waitPeriod));
            printf("Awake from waiting period.\n");
        }
        */
    }
     
    // To finish this application
    // What should we do at the next time to start application.
    
    /*
    // send stop command "0x01" to stop measurement to sfa30: sfa3x_i2c.c
    sens_error = sfa3x_stop_measurement();
    printf("センサーを停止します。\n");

    // ソケット通信をクローズ
    printf("ソケット通信をクローズします。"); 
    close(c_sock);
    */
    
    /** Exit from this application program **/
    return 0;
}

