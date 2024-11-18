#include <stdio.h>  // printf(), strcmp(), fopen()
#include <string.h> // memset()
#include <unistd.h> // for sleep()/usleep(), for close
#include <time.h>
#include <dirent.h> // directory

#include <fcntl.h>
#include <sys/ioctl.h>

#include "common.h"
#include "i2c_hal.h" 
#include "sfa3x_i2c.h"

/*
 * TO USE CONSOLE OUTPUT (PRINTF) YOU MAY NEED TO ADAPT THE INCLUDE ABOVE OR
 * DEFINE IT ACCORDING TO YOUR PLATFORM:
 * #define printf(...)
 */        
/**
 * Linux specific configuration. Adjust the following define to the device path
 * of your sensor.
 */
#define I2C_DEVICE_PATH "/dev/i2c-1"
//sensor error status -> sensErrStatus
#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
#define FAILED_START_SENSOR     30
/** Define support functions **/
int dummyRead(void);
int wahttimeisit(void);

uint8_t sensorStatus = 0;       // Sensor' status
//extern uint8_t status;        // 
//uint8_t status = STOP;        // system status
unsigned char device_marking[32];
// 2024.11.14 Initialize the result.
char ans[1];
/** Define DIR_PATH "/home/pi/works/upload_file". And worrk file is testWork.csv **/
char file_name[] = "testWork.csv";
char dir_path[] = "/home/pi/works/upload_file/";
char fname[128];
Sensor_data result = {"ホルムアルデヒド濃度", "相対湿度", "周囲温度", 0.0, 0.0, 0.0};

int main(int argc, char *argv[]){
//int main(void) {    
    //int8_t waitPeriod = 60;
    time_t timer;
    struct tm *local;
    int year, month, day, hour, minute, second;
    //char resultData;
    
    if(argc <= 1){
        /** Open i2c interface to connect sensirion formaldehyde sensor **/
        i2c_hal_init();
        /** 2024.11.15 以下はうまく動作するがその後のIO操作ではNGとなる
        i2c_device = open(I2C_DEVICE_PATH, O_RDWR);
        if(i2c_device < 0){ 
            printf("Failed to open device with %d.\nTerminated processing.\n", i2c_device);
            return -1;
        }
        */
        /** Make reset sensor hardware #139 in sfa3x_i2c.c **/
        if (sfa3x_device_reset() != NO_ERROR){
            printf("Failed to make Sensor reset.\nTerminated!\n");
            sensorStatus = FAILED_MAKE_RESET;
            return -1;
        }
        /** Confirm device marking function is #116 in sfa3x_i2c.c**/
        if (sfa3x_get_device_marking(&device_marking[0], sizeof(device_marking)) != NO_ERROR) {
            printf("Failed to get Device Marking\n\n");
            sensorStatus = FAILED_GET_DEVICEMARK;
            return -1;
        }
        printf("Welcome! Sensor which serial code is %s.\n\n", device_marking);
        if (sfa3x_start_continuous_measurement() != NO_ERROR) {
            printf("Failed to set senseor continuous_measurement()\n");
            return -1;
        }
        /** wait around 500ms before starting to read data */
        usleep(500000);
        // At once dummy read.
        dummyRead();
    
        /** @2024.11.13 Open work folder which names uploadfile. **/
        DIR *dir = opendir(dir_path);
        if (!dir){ 
            printf("Can't find up_load directory.\nTerminated!\n");
            return -1;
        }
        /** Prepare save file. **/
        strcat(strcat(fname, dir_path), file_name);
    
        /* Start program control.
        while(ans[0] != 0x79){
            printf("Is it ok? If it is ok, please enter ""y""\n\n");
            scanf("%c: ", &ans);
            printf("your input is %c\n", ans[0]);
        }
        */

        /* main loop */
        for (;;) {
            static bool flag = 0;
            static bool rept = 0;

            // 2023.11.24 Get the latest time
            timer = time(NULL);
            // Convert to localtime
            local = localtime(&timer);
            // 年月日と時分秒をtm構造体の各パラメタから変数に代入
            year = local->tm_year + 1900;   // Because year count starts at 1900
            month = local->tm_mon + 1;      // Because 0 indicates January.
            day = local->tm_mday;
            hour = local->tm_hour;
            minute = local->tm_min;
            second = local->tm_sec;
        
            // Original is a wait around 10000ms before Sensor operating
            // wait around 5s before Sensor operatin
            if(second % 5 == 0){
                if(rept == 0){
                    rept = 1;
                    // Display current time's information on console.
                    printf("%d-%d-%d @%d:%d:%d\n", year, month, day, hour, minute, second);
                    /** Read Measure function is #134 **/
                    result = ReadMeasure(result);
                    printf("%s: %.1f ppb\n", result.gasName, result.gas);
                    printf("%s: %.2f %%RH\n", result.humid, result.humidity);
                    printf("%s: %.2f °C\n\n", result.temp, result.temperature);
                }
                if(second == 0 && flag == 0){
                    flag = 1;
                    printf("Save file:~ %s\nSave data is %d-%d-%d %d:%d:%d\n", fname, year, month, day, hour, minute, second);
                    FILE *fp = fopen(fname,"a");
                    if (fp == NULL){
                        printf("The file: %s is NOT able to open.\n", fname);
                        return -1;
                    }
                    fprintf(fp, "measured_date,measured_value,point_id,place_id\n");
                    fprintf(fp, "%d-%d-%d %d:%d:%d,%0.2f,%s\n", year, month, day, hour, minute, second, result.gas, result.gasName);
                    fprintf(fp, "%d-%d-%d %d:%d:%d,%.2f,%s\n", year, month, day, hour, minute, second, result.humidity, result.humid);
                    fprintf(fp, "%d-%d-%d %d:%d:%d,%.2f,%s\n", year, month, day, hour, minute, second, result.temperature, result.temp);        
                    fclose(fp);            
                }
            }
            else{
                rept = 0;
                flag = 0;
            }
            // 2023.11.24 Read measured data and display on terminal
            // 2024.11.12 #233 in tcp_com.c
            //if (closedir(dir)) printf("Failed to close directory.\n");
            //printf("main_#125 Close directory upload_file\n");
        }
     
        // To finish this application
        // What should we do at the next time to start application.
        /*
        // send stop command "0x01" to stop measurement to sfa30: sfa3x_i2c.c
        sens_error = sfa3x_stop_measurement();
        printf("センサーを停止します。\n");
        */
        /** Exit from this application program **/
        return 0;

        }
        else{
            printf("Your request parameter is %s\n", argv[1]);
            if(srgv[1} == "setup"){
                printf("Welcopme to setup program\n"); 
            }
            
        }
}

int dummyRead(){
    float data1 = 0.0, data2 = 0.0, data3 = 0.0;
        
    // it may adjust the measurement interval around for 500ms
    usleep(500000); // Original software settings.

    if(sfa3x_read_measured_values(&data1, &data2, &data3) != 0){
        printf("Error: Failed to read sensor data\n");
        return -1;
    }
    printf("Success: to make a dummy read\n\n");
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




