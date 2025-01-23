#include <stdio.h>      // For standard operation such as printf(), strcmp(), fopen()
#include <stdlib.h>     // For library like a malloc() which is memory opearation.
#include <string.h>     // memset()
#include <unistd.h>     // For sleep()/usleep(), for close
#include <time.h>
#include <dirent.h>     // For directory operation.
#include <stdbool.h>    // For bool operation.
#include <fcntl.h>
#include <sys/ioctl.h>

#include "define.h"
#include "common.h"
#include "i2c.h" 
#include "sfa3x.h"

/*
 * TO USE CONSOLE OUTPUT (PRINTF) YOU MAY NEED TO ADAPT THE INCLUDE ABOVE OR
 * DEFINE IT ACCORDING TO YOUR PLATFORM:
 * #define printf(...)
 */ 
        
/** Linux specific configuration. Adjust the following define to the device path of the sensor. */
#define I2C_DEVICE_PATH "/dev/i2c-1"
#define CONFIG_LINE             512  // Max. value of number of bytes in line in config file.
#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
#define FAILED_START_SENSOR     30

uint8_t sensorStatus = 0;       // Sensor' status
unsigned char device_marking[32];
// 2024.11.14 Initialize the result.
/** Define DIR_PATH "/home/pi/works/upload_file" and worrk file name is testWork.csv **/
char work_file[] = "testWork_test.csv";
char setup_file[] = "config";
char dir_path[] = "/home/pi/works/upload_file/";
char location_name[] = "株式会社A";
char fname[128];

/** About tm structure
* struct tm {
*  int tm_sec;      // 秒 [0-61] 最大2秒までのうるう秒を考慮
*  int tm_min;      // 分 [0-59] 
*  int tm_hour;     // 時 [0-23] 
*  int tm_mday;     // 日 [1-31]
*  int tm_mon;      // 月 [0-11] 0から始まることに注意
*  int tm_year;     // 年 [1900からの経過年数]
*  int tm_wday;     // 曜日 [0:日 1:月 ... 6:土]
*  int tm_yday;     // 年内の通し日数 [0-365] 0から始まることに注意
*  int tm_isdst;    // 夏時間が無効であれば 0
* };
**/
struct tm *local;

LOCATION Place;         // Monitor site information, name and number of sensor point's.
SENSOR Sensor[16];      // Sensor's information whish is located at the monitor site.

/// Initialize Sensor information the sample monitor site.
Sensor_data result = {"ホルムアルデヒド濃度", "相対湿度", "周囲温度", 0.0, 0.0, 0.0};


int main(int argc, char *argv[]){  
    static uint8_t point_num = 0;
    
    /** @2024.11.13 Open work folder which names uploadfile. **/
    DIR *dir = opendir(dir_path);
    if (!dir){ 
        printf("Can't find \"up_load\" directory.\nTerminated!\n");
        return -1;
    }
    /** Normal operation. **/
    if(argc <= 1){
        /** Open i2c interface to connect sensirion formaldehyde sensor **/
        i2c_hal_init();
        /** Make reset sensor hardware #139 in sfa3x_i2c.c **/
        if (DeviceReset() != NO_ERROR){
            printf("Failed to make Sensor reset.\nTerminated!\n");
            sensorStatus = FAILED_MAKE_RESET;
            return -1;
        }
        /** Confirm device marking function is #116 in sfa3x_i2c.c**/
        if (GetDeviceMarking(&device_marking[0], sizeof(device_marking)) != NO_ERROR) {
            printf("Failed to get Device Marking\n\n");
            sensorStatus = FAILED_GET_DEVICEMARK;
            return -1;
        }
        /** Opening message!! **/
        printf("Welcome!\nSensor serial-code is %s.\n\n", device_marking);
        
        if (StartContinuousMeasurement() != NO_ERROR) {
            printf("Failed to set senseor continuous_measurement()\n");
            return -1;
        }
        /** wait around 500ms before starting to read data */
        usleep(500000);
        /** At the beginning, makes a dummy read once. **/
        ReadDummy();
    
        /** previous dir control and prepare working directory.**/
        strcat(strcat(fname, dir_path), work_file);

        /* main loop */
        for (;;) {
            static bool flag = 0, rept = 0;
            time_t timer;
            int year, month, day, hour, minute, second;
            char dateNow[32], timeNow[16], today[48];
            
            sleep(1);
            // 2023.11.24 Get the latest tim
            timer = time(NULL);  // For measurement system base timer.
            local = localtime(&timer);  // Convert to localtime
            // 年月日と時分秒をtm構造体の各パラメタから変数に代入
            year = local->tm_year + 1900;   // Because year count starts at 1900
            month = local->tm_mon + 1;      // Because 0 indicates January.
            day = local->tm_mday;
            hour = local->tm_hour;
            minute = local->tm_min;
            second = local->tm_sec;            
            sprintf(dateNow, "%d-%d-%d", year, month, day);
            sprintf(timeNow, "%d:%d:%d", hour, minute, second);
            sprintf(today, "%s %s", dateNow, timeNow);

            if(second % 5 == 0){
                if(rept == 0){
                    rept = 1;
                    // Display current time's information on console.
                    printf("%s @%s\n", dateNow, timeNow);
                    // Read Measure function is #126 in sfa3x_i2c.c
                    result = ReadMeasure(result);
                    printf("%s\n", location_name);
                    printf("%s: %.1f ppb\n", result.gasName, result.gas);
                    printf("%s: %.2f %%RH\n", result.humid, result.humidity);
                    printf("%s: %.2f °C\n\n", result.temp, result.temperature);
                }
                if((int)second == 0 && flag == 0){
                    flag = 1;
                    FILE *fp = fopen(fname,"w");
                    if (fp == NULL){
                        printf("The file: %s is NOT able to open.\n", fname);
                        return -1;
                    }
                    fprintf(fp, "measured_date,measured_value,sensor,place\n");
                    fprintf(fp, "%s,%0.2f,%s,%s\n", today, result.gas, result.gasName, location_name);
                    fprintf(fp, "%s,%.2f,%s,%s\n", today, result.humidity, result.humid, location_name);
                    fprintf(fp, "%s,%.2f,%s,%s\n", today, result.temperature, result.temp, location_name);        
                    fclose(fp);
                    printf("Save data was written in %s\n\n", fname);            
                }
            }else{
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

    }else if(argc >= 3 && (strcmp(argv[1], "setup") == 0)){
        /** Setup operation class 1.
        *  From choosing a configuration file **/
        char line[CONFIG_LINE];
        char *mark, *str, *unit;
        uint8_t *id = 0;
        int8_t *num = 0;
        int i = 0;
        
        printf("main_#180 argc = %d, argv = %s\n", argc, argv[1]);
        
        // In case of specifiy the setup file name.
        if(argc == 3){
            char* ans;              
            char fl[128];

            ans = (char *)malloc(sizeof(char));
            printf("ファイル名\"%s\"を設定ファイルとします\n", argv[2]);
            printf("OKの場合は\"y\",変更する場合は\"n\"... ");
            scanf("%s", ans);
            
            if(!strcmp(ans, "y")){
                strcat(strcat(fname, dir_path), argv[2]);
                if(AddFile(fname)) printf("ファイル, \"%s\"を作成しました。\n", fname);
                else{
                    printf("ファイル名\"%s\"は既に存在します\n", fname);
                    //if(OverWriteFile(fname) == -1) printf("main_#201 Failed to make \"%s\".", fname);                    
                }
            }else if(!strcmp(ans, "n")){
                while(1U){
                    printf("ファイル名を入力してください\n");
                    scanf("%s", fl);
                    strcat(strcat(fname, dir_path), fl);
                    printf("ファイル名\"%s\"を作成します\n", fname);
                    printf("OKの場合はyを変更する場合はnを入力してください... ");
                    scanf("%s", ans);
                    if(!strcmp(ans, "y")){
                        OverWriteFile(fname);
                    }
                    strcpy(fname, "");
                }
            }
            free(ans);
        }             

        // Open configuration file.
        FILE *fp = fopen(fname,"r");
        if (fp == NULL){
            printf("Failed to open file: %s\n", fname);
            return -1;
        }
        
        // Read config.
        str = (char *)malloc(sizeof(char) * 256);
        num = (int8_t *)malloc(sizeof(int8_t));
        id = (uint8_t*)malloc(sizeof(uint8_t)); 
        mark = (char *)malloc(sizeof(char) * 8);
        unit = (char*)malloc(sizeof(char) * 8);

        while(fgets(line, CONFIG_LINE, fp) != NULL){
            sscanf(line,"%[^,],%hhd,%[^,],%hhd,%[^,]",mark, id, str, num, unit);
            i = *num - 1;
            if(!strcmp(mark,"header")) Place = SetLocationName(str, *num);
            else if(!strcmp(mark,"point")){
                Sensor[i] = SetSensor(str, *id, unit);
            }   
        }
        free(mark); 
        free(id); 
        free(num); 
        free(str);
        
        putchar('\n');
        printf("main_#255 Location name is \"%s\" and there are %d sensor points.\nAs followings;\n", Place.name, Place.point_num);
        for(i=0;i < Place.point_num;i++){
            printf("main_#257 Sensor[%d] is %s\n", i, Sensor[i].name);
        }
        fclose(fp);
        putchar('\n');
        printf("main_#257 Close %s file.\n", fname);
        printf("main_#258 And terminate.\n");
    
    }else{
        /** Setup operation class 2.
        *  Frpm creating a setup file. **/
        char point[128];
        char ans[5];

        if(strcmp(argv[1], "setup") == 0){
            printf("Let's make Sensor settings.\n\n");
            /*
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
            */
            /** Set place **/
            while(strcmp(ans, "y") != 0){
                printf("サイト名を設定してください... ");
                scanf("%s", Place.name);
                printf("your input is \"%s\"?\n\n", Place.name);
                printf("OKの場合は\"y\",変更する場合は\"n\"... ");
                scanf("%s", ans);
            }
            /** Initialize the variable char* ans with 0x00 code. **/
            ans[0] = 0x00;

            /** Prepare setup_file file. **/
            strcat(strcat(fname, dir_path), setup_file);
            FILE *fp = fopen(fname,"w");
            if (fp == NULL){
                printf("The file: %s is NOT able to open.\n", fname);
                return -1;
            }
            //fprintf(fp, "Set_up data @%d-%2d-%2d %2d:%2d\n",year, month, day, hour, minute);
            fprintf(fp, "place,%s\n",Place.name);
            printf("\"%s\"をサイト名として登録しました.\n\n", Place.name);
                
            /** Set Sensor Points **/
            while(strcmp(ans, "q") != 0){
                point_num++;
                while(strcmp(ans, "y") != 0){
                    printf("次にセンサーポイントを設定してください...\n");
                    scanf("%s", point);
                    printf("your input is ""%s""?\n", point);
                    printf("OKの場合はyを変更する場合はnを...");
                    scanf("%s", ans);
                }
                fprintf(fp, "%d,%s\n",point_num, point);
                printf("\"%s\"をセンサーポイント名として登録しました.\n\n", point);
                printf("センサーポイント設定を継続する場合は\"r\",終了する場合は\"q\"を入力してください... ");
                scanf("%s", ans);
            }
            printf("main_#214 point_num is %d\n", point_num);
            printf("your work was terminated %s\n", ans);
            fclose(fp);
        }else{
            printf("You have set a wrong parameter\nTherefore Terminate!");
            return -1;
        }
    }
    return 0;
}

int8_t ReadDummy(){
    float data1 = 0.0, data2 = 0.0, data3 = 0.0;

    // It may be adjust the measurement interval around for 500ms
    usleep(500000); // Original software settings.
    if(ReadMeasuredValues(&data1, &data2, &data3) != 0){
        printf("Error: Failed to read sensor data\n");
        return -1;
    }
    return 0;    
}

LOCATION SetLocationName(char* name, int8_t num) {
    LOCATION p;
    
    strcpy(p.name, name);
    //p.name = name;
    p.point_num = num;
    return p;
}

SENSOR SetSensor(char* name, uint8_t id, char* unit) {
    SENSOR s;
    strcpy(s.name, name);
    s.id = id;
    strcpy(s.unit, unit);
    return s;
}

/*
 * Confirm the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists.
*/
int8_t AddFile(const char* path)
{
    FILE* fp = fopen(path, "a+");
    if (fp == NULL) {
        return 1;
    }
    fclose(fp);
    return 0;
}

/*
 * Overwrite the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists.
*/
int8_t OverWriteFile(const char* path)
{
    int8_t res = 0;

    FILE* fp = fopen(path, "w+");
    if (fp == NULL) {
        printf("OverWrite_#398 Failed overwrite \"%s\".\n", path);
        res = -1;
    }
    fclose(fp);
    printf("OverWrite_#402 \"%s\" is overwerwritten.\n", path);
    return res;
}
