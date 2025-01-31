#include <stdio.h>      // printf(), strcmp(), fopen(), perror()
#include <stdlib.h>     // malloc(), atoi()
#include <sys/stat.h>   // mkdir()
#include <string.h>     // memset(), strtok()
#include <unistd.h>     // sleep()/usleep() and close(), open(), read(), write()
#include <dirent.h>     // For directory operation.
#include <time.h>       // time()
#include <stdbool.h>    // For bool operation.

#include "common.h"
#include "device.h"
#include "foperation.h"
#include "main.h"

/** Raspberry Pi specific configuration.
 *  Adjust the following define to the device path of the sensor. */
#define I2C_DEVICE_PATH         "/dev/i2c-1"

#define FILE_NAME_SIZE  256
#define PATH_SIZE       128
#define LINE_SIZE       512  // Max. value of number of bytes in line in config file.
#define CONFIG_FILE     "config"    // Setup file
#define UPLOAD_FILE     "work.csv"  // Upload file
#define UPLOAD_PATH     "upload_file/"  // Upload file

#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
#define FAILED_START_SENSOR     30
//#define EXIT_FAILURE     -1

static char configFile[FILE_NAME_SIZE];
static char uploadFile[FILE_NAME_SIZE];
char currentPath[PATH_SIZE];
LOCATION Site;
//Site_id:    Site.id 
//Site_name:  Site.name
//Site_num:   Site.num

POINT Sensor[16];   // Sensor's information whish is located at the monitor site.
//Sensor_id:    Sensor[id].id 
//Sensor_name:  Sensor[id].name
//Sensor_unit:   Sensor[id].unit

uint8_t sensorStatus = 0;       // Sensor' status

/** Define DIR_PATH "/home/pi/works/upload_file" and work file name is testWork.csv **/
unsigned char deviceMarking[32];
struct tm *local;

int main(int argc, char *argv[]){  
    static uint8_t point_num = 0;
    int8_t i;
    char str[LINE_SIZE];
    FILE *fp; //FILE structure.
    //SetupConfig(configFile);

    /** Open the config file */
    if(getcwd(currentPath, PATH_SIZE) == NULL){
        printf("カレントディレクトリーが取得できません.\nプログラムを終了します.\n");
        return -1;    
    }
    strcpy(configFile, currentPath);
    strcat(strcat(configFile, "/"), CONFIG_FILE);
    fp = fopen(configFile, "r");
    if (fp == NULL){
        printf("指定された \"%s\" ファイルがありません.\nプログラムを終了します.\n", configFile);
        return -1;
    }
    while(fgets(str, LINE_SIZE, fp) != NULL){
        static int8_t id = 0;
        char *ptr;

        ptr = strtok(str, ",");     // First
        if(strcmp(ptr, "location") == 0){
            ptr = strtok(NULL, ",");    // Site.name
            strcpy(Site.name, ptr);

            ptr = strtok(NULL, ",");    // Number
            Site.num = atoi(ptr);            
        }else{
            ptr = strtok(NULL, ",");    // Sensor ID
            Sensor[id].id = atoi(ptr);             

            ptr = strtok(NULL, ",");    // Sensor NAME
            strcpy(Sensor[id].name, ptr);

            ptr = strtok(NULL, ",");    // Sensor UNIT
            strcpy(Sensor[id].unit, ptr);

            id++;
        }
    }
    fclose(fp);

    /** @2025.1.31 Open upload folder which includes upload_file. **/
    DIR *work_dir = opendir(UPLOAD_PATH);
    if (!work_dir){ 
        if(mkdir(UPLOAD_PATH, 0755)){
            //error(argv[0]);
            perror("指定したフォルダーの作成に失敗しました.");
            exit(EXIT_FAILURE);
        }
    }
    printf("main_#114 I got the upload file path as \"%s.\"\n", UPLOAD_PATH);

    /** Normal operation. **/
    if(argc <= 1){
        /// Reset sensor board hardware.
        if (DeviceReset() != NO_ERROR){
            perror("センサーデバイスの初期化に失敗しました... プログラムを終了します.\n");
            return -1;
            //abort();
            //exit(EXIT_FAILURE);
        }
        /// Obtain the device marking.
        if (GetDeviceMarking(&deviceMarking[0], sizeof(deviceMarking)) != NO_ERROR) {
            perror("センサーのシリアルコード取得に失敗しました... プログラムを終了します.\n");
            return -1;
        }
        /// Opening message.
        printf("センサーの読み取りを開始します.\nセンサーのシリアルコードは, %s です.\n\n", deviceMarking);
        
        if (StartContinuousMeasurement() != NO_ERROR) {
            perror("センサーを連続読み出しモードに設定できませんでした... プログラムを終了します.\n");
            return -1;
        }        
        /// At the beginning, makes a dummy read once.
        usleep(500000);
        BlankRead();

        /** previous dir control and prepare working directory.**/
        strcat(strcat(strcat(strcat(uploadFile, currentPath), "/"), UPLOAD_PATH), UPLOAD_FILE);
        printf("main_#153 file name is %s\n", uploadFile);

        /* main loop */
        for (;;) {
            static bool flag = 0, rept = 0;
            time_t timer;
            int year, month, day, hour, minute, second;
            char dateNow[32], timeNow[16], now[48];
            /** Case 0: Using a data logger connecting one or many sensors,
             * and it is connected to a lap-top computer via serial communication interface. */

            /** Case 1: Using a sensirion sensor and it embedded temperature and humidity sensor in it,
             * and it is connected with Raspberry Pi via I2C interface.  */
            SDATA Result = {
                Sensor[0].name, Sensor[1].name, Sensor[2].name, 
                Sensor[0].data, Sensor[1].data, Sensor[2].data
            };

            usleep(500000);
            // 2023.11.24 Get the latest tim
            timer = time(NULL);  // For measurement system base timer.
            local = localtime(&timer);  // Convert to localtime
            // Read year, month, day, hour, minute, second to current variables.
            year = local->tm_year + 1900;   // Because year count starts at 1900
            month = local->tm_mon + 1;      // Because 0 indicates January.
            day = local->tm_mday;
            hour = local->tm_hour;
            minute = local->tm_min;
            second = local->tm_sec;            
            sprintf(dateNow, "%d-%d-%d", year, month, day);
            sprintf(timeNow, "%d:%d:%d", hour, minute, second);
            sprintf(now, "%s %s", dateNow, timeNow);

            if(second % 5 == 0){
                if(rept == 0){
                    rept = 1;

                    /// Display current time' information on console.
                    printf("%s @%s\n", dateNow, timeNow);
                    /// Read sensor' data and display on the screen.
                    Result = ReadMeasure(Result);
                    /** The structures of the Result
                     * char *gasName;       // gas name
                     * char *humid;         // humidity
                     * char *temp;          // temperature
                     * float gas;           // gas concentration value
                     * float humidity;      // humidity value
                     * float temperature;   // temperature value */
                    printf("%s\n", Site.name);
                    printf("%s: %.1f %s\n", Result.gasName, Result.gas, "ppb");
                    printf("%s: %.2f %s\n", Result.humid, Result.humidity, "\%RH");
                    printf("%s: %.2f %s\n\n", Result.temp, Result.temperature, "°C");
                }

                if(second == 0 && flag == 0){
                    flag = 1;
                    FILE *fp = fopen(uploadFile,"w");
                    if (fp == NULL){
                        perror("ファイルにアクセスすることができません... プログラムを終了します.\n");
                        return -1;
                    }
                    fprintf(fp, "measured_date,measured_value,sensor,place\n");
                    fprintf(fp, "%s,%0.1f,%s,%s\n", now, Result.gas, Result.gasName, Site.name);
                    fprintf(fp, "%s,%.2f,%s,%s\n", now, Result.humidity, Result.humid, Site.name);
                    fprintf(fp, "%s,%.2f,%s,%s\n", now, Result.temperature, Result.temp, Site.name);        

                    fclose(fp);
                    printf("Save data was written in %s\n\n", uploadFile);

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
        //int i = 0;
        
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
//                strcat(strcat(configFile, dir_path), argv[2]);
                if(AddFile(configFile)) printf("ファイル, \"%s\"を作成しました。\n", configFile);
                else{
                    printf("ファイル名\"%s\"は既に存在します\n", configFile);
                    //if(OverWriteFile(fname) == -1) printf("main_#201 Failed to make \"%s\".", fname);                    
                }
            }else if(!strcmp(ans, "n")){
                while(1U){
                    printf("ファイル名を入力してください\n");
                    scanf("%s", fl);
//                    strcat(strcat(configFile, dir_path), fl);
                    printf("ファイル名\"%s\"を作成します\n", configFile);
                    printf("OKの場合はyを変更する場合はnを入力してください... ");
                    scanf("%s", ans);
                    if(!strcmp(ans, "y")){
                        OverWriteFile(configFile);
                    }
                    strcpy(configFile, "");
                }
            }
            free(ans);
        }             

        // Open configuration file.
        FILE *fp = fopen(configFile,"r");
        if (fp == NULL){
            printf("Failed to open file: %s\n", configFile);
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
        //printf("main_#257 Close %s file.\n", configFile);
        //printf("main_#258 And terminate.\n");
    
    }else{
        /** Setup operation class 2.
        *  From creating a setup file. **/
        char point[128];
        char ans[5];

        if(strcmp(argv[1], "setup") == 0){
            printf("main_#303 Let's make Sensor settings.\n\n");
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
            //strcat(strcat(fname, dir_path), configFile);
            FILE *fp = fopen(configFile,"w");
            if (fp == NULL){
                printf("The file: %s is NOT able to open.\n", configFile);
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
