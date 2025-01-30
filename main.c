#include <stdio.h>      // printf(), strcmp(), fopen()
#include <stdlib.h>     // malloc(), atoi()
#include <string.h>     // memset(), strtok()
#include <unistd.h>     // sleep()/usleep() and close(), open(), read(), write()
#include <dirent.h>     // For directory operation.
#include <time.h>       // time()
#include <stdbool.h>    // For bool operation.

#include "common.h"
#include "device.h"
#include "foperation.h"
#include "main.h"


#define FILE_NAME_SIZE  256
#define PATH_SIZE       128
#define CONFIG_FILE     "config"    // Setup file
#define LINE_SIZE       512  // Max. value of number of bytes in line in config file.

#define WORK_FILE       "work.csv"  // Upload file

/** Linux specific configuration.
 *  Adjust the following define to the device path of the sensor. */
#define I2C_DEVICE_PATH         "/dev/i2c-1"
#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
#define FAILED_START_SENSOR     30

static char configFileName[FILE_NAME_SIZE];
char currentPath[PATH_SIZE];
LOCATION Site;

/// @brief 2025.1.29 
static char sensorName[16][256];    //=> Sensor.name
static uint8_t sensorID[16];        //=> Sensor.id
static char sensorUnit[16][16];     //=> Sensor.unit
POINT Sensor;
/*
typedef struct{
    uint8_t id;    // sensor_ID
    char name[256];   // sensor_name
    char unit[8];   // sensor_unit    
}SENSOR;
*/

static char workFileName[FILE_NAME_SIZE];

uint8_t sensorStatus = 0;       // Sensor' status
/** Define DIR_PATH "/home/pi/works/upload_file" and work file name is testWork.csv **/
//char work_file[] = "testWork_test.csv";
char dir_path[] = "/home/pi/works/upload_file/";
//char fname[128];
unsigned char deviceMarking[32];
struct tm *local;

//LOCATION Place;     // Monitor site information, name and number of sensor point's.
SENSOR Sensor[16];  // Sensor's information whish is located at the monitor site.
/// Initialize Sensor information the sample monitor site.
Sensor_data result = {"ホルムアルデヒド濃度", "相対湿度", "周囲温度", 0.0, 0.0, 0.0};

int main(int argc, char *argv[]){  
    static uint8_t point_num = 0;
    int8_t i = 0;
    char str[LINE_SIZE];
    FILE *fp; //FILE structure.
    //SetupConfig(configFileName);

    /** Get the current path */
    
    if(getcwd(currentPath, PATH_SIZE) == NULL){
        printf("カレントディレクトリーが取得できません.\nプログラムを終了します.\n");
        return -1;    
    }
    strcpy(configFileName, currentPath);
    strcat(strcat(configFileName, "/"), CONFIG_FILE);

    fp = fopen(configFileName, "r");
    if (fp == NULL){
        printf("指定された \"%s\" ファイルがありません.\nプログラムを終了します.\n", configFileName);
        return -1;
    }
    while(fgets(str, LINE_SIZE, fp) != NULL){
        static int8_t id = 0;
        char *ptr;

        ptr = strtok(str, ",");     // First
        if(strcmp(ptr, "location") == 0){
            ptr = strtok(NULL, ",");    // Site.name
            //strcpy(company, ptr);
            strcpy(Site.name, ptr);
            
            ptr = strtok(NULL, ",");    // Number
            // sensorNum = atoi(ptr);
            Site.num = atoi(ptr);
            
        }else{
            ptr = strtok(NULL, ",");    // Sensor ID
            sensorID[id] = atoi(ptr);             

            ptr = strtok(NULL, ",");    // Sensor NAME
            strcpy(sensorName[id], ptr);

            ptr = strtok(NULL, ",");    // Sensor UNIT
            strcpy(sensorUnit[id], ptr);

            id++;
        }
    }
    fclose(fp);
    

    printf("main_#97 The company is %s\n", Site.name);
    printf("main_#98 The number is %d\n\n", Site.num);
    for(i = 0; i < Site.num; i++){
        printf("main_#100 The ID[%d] is %d\n", i, sensorID[i]);
        printf("main_#101 The sensor[%d] is %s\n", i, sensorName[i]);
        printf("main_#102 The unit[%d] is %s\n\n", i, sensorUnit[i]);
    }

    /** @2024.11.13 Open work folder which includes uploadfile. **/
    //DIR *dir = opendir(dir_path);
    DIR *dir = opendir(currentPath);
    if (!dir){ 
        printf("Missing \"up_load\" directory.\nTerminated!\n");
        return -1;
    }
    printf("Yes, I'm here\n");

    /** Normal operation. **/
    if(argc <= 1){
        /// Reset sensor board hardware.
        if (DeviceReset() != NO_ERROR){
            printf("Device reset failed,...\nTerminated!\n");
            return -1;
        }
        /// Obtain the device marking.
        if (GetDeviceMarking(&deviceMarking[0], sizeof(deviceMarking)) != NO_ERROR) {
            printf("Failed to get Device Marking\nTerminated!\n");
            return -1;
        }
        /// Opening message.
        printf("Welcome!\nSerial-code is %s.\n\n", deviceMarking);
        
        if (StartContinuousMeasurement() != NO_ERROR) {
            printf("Failed to set sensor continuous_measurement()\nTerminated!\n");
            return -1;
        }        
        /** At the beginning, makes a dummy read once. */
        usleep(500000);
        BlankRead();
    
        /** previous dir control and prepare working directory.**/
        strcat(strcat(workFileName, currentPath), WORK_FILE);

        /* main loop */
        for (;;) {
            static bool flag = 0, rept = 0;
            time_t timer;
            int year, month, day, hour, minute, second;
            char dateNow[32], timeNow[16], today[48];
            
            usleep(500000);
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
                    /// Display current time' information on console.
                    printf("%s @%s\n", dateNow, timeNow);
                    
                    /// Read sensor' data and display on the screen.
                    result = ReadMeasure(result);
                    // printf("%s\n", company);
                    printf("%s\n", Site.name);
                    printf("%s: %.1f ppb\n", result.gasName, result.gas);
                    printf("%s: %.2f %%RH\n", result.humid, result.humidity);
                    printf("%s: %.2f °C\n\n", result.temp, result.temperature);
                }
                if((int)second == 0 && flag == 0){
                    flag = 1;
                    FILE *fp = fopen(workFileName,"w");
                    if (fp == NULL){
                        printf("The file: %s is NOT able to open.\n", workFileName);
                        return -1;
                    }
                    fprintf(fp, "measured_date,measured_value,sensor,place\n");
                    fprintf(fp, "%s,%0.2f,%s,%s\n", today, result.gas, result.gasName, Site.name);
                    fprintf(fp, "%s,%.2f,%s,%s\n", today, result.humidity, result.humid, Site.name);
                    fprintf(fp, "%s,%.2f,%s,%s\n", today, result.temperature, result.temp, Site.name);        
                    fclose(fp);
                    printf("Save data was written in %s\n\n", workFileName);            
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
        char line[LINE_SIZE];
        char *mark, *str, *unit;
        uint8_t *id = 0;
        int8_t *num = 0;
        int i = 0;
        
        printf("main_#180 argc = %d, argv = %s\n", argc, argv[1]);
        
        // In case of specify the setup file name.
        if(argc == 3){
            char* ans;              
            char fl[128];

            ans = (char *)malloc(sizeof(char));
            printf("ファイル名\"%s\"を設定ファイルとします\n", argv[2]);
            printf("OKの場合は\"y\",変更する場合は\"n\"... ");
            scanf("%c", ans);
            
            if(!strcmp(ans, "y")){
                strcat(strcat(configFileName, dir_path), argv[2]);
                if(AddFile(configFileName)) printf("ファイル, \"%s\"を作成しました。\n", configFileName);
                else{
                    printf("ファイル名\"%s\"は既に存在します\n", configFileName);
                    //if(OverWriteFile(fname) == -1) printf("main_#201 Failed to make \"%s\".", fname);                    
                }
            }else if(!strcmp(ans, "n")){
                while(1U){
                    printf("ファイル名を入力してください\n");
                    scanf("%s", fl);
                    strcat(strcat(configFileName, dir_path), fl);
                    printf("ファイル名\"%s\"を作成します\n", configFileName);
                    printf("OKの場合はyを変更する場合はnを入力してください... ");
                    scanf("%s", ans);
                    if(!strcmp(ans, "y")){
                        OverWriteFile(configFileName);
                    }
                    strcpy(configFileName, "");
                }
            }
            free(ans);
        }             

        // Open configuration file.
        FILE *fp = fopen(configFileName,"r");
        if (fp == NULL){
            printf("Failed to open file: %s\n", configFileName);
            return -1;
        }
        
        // Read config.
        str = (char *)malloc(sizeof(char) * 256);
        num = (int8_t *)malloc(sizeof(int8_t));
        id = (uint8_t*)malloc(sizeof(uint8_t)); 
        mark = (char *)malloc(sizeof(char) * 8);
        unit = (char*)malloc(sizeof(char) * 8);

        while(fgets(line, LINE_SIZE, fp) != NULL){
            sscanf(line,"%[^,],%hhd,%[^,],%hhd,%[^,]",mark, id, str, num, unit);
            i = *num - 1;
            //if(!strcmp(mark,"header")) Place = SetLocationName(str, *num);
            //else if(!strcmp(mark,"point")){
            //    Sensor[i] = SetSensor(str, *id, unit);
            //}   
        }
        free(mark); 
        free(id); 
        free(num); 
        free(str);
        
        putchar('\n');
        //printf("main_#255 Location name is \"%s\" and there are %d sensor points.\nAs followings;\n", Place.name, Place.point_num);
        //for(i=0;i < Place.point_num;i++){
        //    printf("main_#257 Sensor[%d] is %s\n", i, Sensor[i].name);
        //}
        fclose(fp);
        putchar('\n');
        //printf("main_#257 Close %s file.\n", configFileName);
        //printf("main_#258 And terminate.\n");
    
    }else{
        /** Setup operation class 2.
        *  From creating a setup file. **/
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
                scanf("%s", Site.name);
                printf("your input is \"%s\"?\n\n", Site.name);
                printf("OKの場合は\"y\",変更する場合は\"n\"... ");
                scanf("%s", ans);
            }
            /** Initialize the variable char* ans with 0x00 code. **/
            ans[0] = 0x00;

            /** Prepare setup_file file. **/
            //strcat(strcat(fname, dir_path), configFileName);
            FILE *fp = fopen(configFileName,"w");
            if (fp == NULL){
                printf("The file: %s is NOT able to open.\n", configFileName);
                return -1;
            }
            //fprintf(fp, "Set_up data @%d-%2d-%2d %2d:%2d\n",year, month, day, hour, minute);
            fprintf(fp, "place,%s\n",Site.name);
            printf("\"%s\"をサイト名として登録しました.\n\n", Site.name);
                
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
/*
Site SetLocationName(char* name, int8_t num) {
    LOCATION p;
    
    strcpy(p.name, name);
    //p.name = name;
    p.point_num = num;
    return p;
}
 */
/*
SENSOR SetSensor(char* name, uint8_t id, char* unit) {
    SENSOR s;
    strcpy(s.name, name);
    s.id = id;
    strcpy(s.unit, unit);
    return s;
}
*/
