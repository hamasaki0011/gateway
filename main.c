#include <stdio.h>      // printf(), strcmp(), fopen(), perror()
#include <stdlib.h>     // malloc(), atoi()
#include <sys/stat.h>   // mkdir()
#include <string.h>     // memset(), strtok()
#include <unistd.h>     // sleep()/usleep(), close(), open(), read(), write()
#include <dirent.h>     // For directory operation.
#include <time.h>       // time()
#include <stdbool.h>    // For bool operation.

#include "common.h"
#include "device.h"
#include "main.h"

/** Raspberry Pi specific configuration.
 *  Adjust the following define to the device path of the sensor. */
//#define I2C_DEVICE_PATH         "/dev/i2c-1"

#define FILE_NAME_SIZE  256
#define PATH_SIZE       128
#define LINE_SIZE       512  // Max. value of number of bytes in line in config file.
#define CONFIG_FILE     "config"    // Setup file
#define UPLOAD_FILE     "work.csv"  // Upload file
#define UPLOAD_PATH     "upload_file/"  // Upload file

#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
#define FAILED_START_SENSOR     30

LOCATION Site;
//Site_id:    Site.id 
//Site_nam  Site.name
//Site_num:   Site.num:

POINT Sensor[16];   // Sensor's information whish is located at the monitor site.
//Sensor_id:    Sensor[id].id 
//Sensor_name:  Sensor[id].name
//Sensor_unit:   Sensor[id].unit

struct tm *local;

int main(int argc, char *argv[]){
    /// Define DIR_PATH "/home/pi/works/upload_file" and work file name is testWork.csv
    static char uploadFile[FILE_NAME_SIZE];
    char configFile[FILE_NAME_SIZE];
    char currentPath[PATH_SIZE];
    char lineString[LINE_SIZE];
    unsigned char deviceMarking[32];

    /// Find current directory and Set config file name and path.
    if(getcwd(currentPath, PATH_SIZE) == NULL){
        perror("カレントディレクトリーを取得できません.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(configFile, currentPath);
    strcat(strcat(configFile, "/"), CONFIG_FILE);

    /// Open upload folder which includes upload_file.
    DIR *work_dir = opendir(UPLOAD_PATH);
    if (!work_dir){ 
        if(mkdir(UPLOAD_PATH, 0755)){
            perror("指定したフォルダーの作成に失敗しました.\n");
            exit(EXIT_FAILURE);
        }
    }

    FILE *fp; //FILE structure.

    /// Normal operation.
    if(argc <= 1){
        fp = fopen(configFile, "r");
        if (fp == NULL){
            printf("設定ファイルがありません.\n作成しますか？\n");
            /* ［要検討］作成しない場合の対処 */
            fp = fopen(BuildConfig(configFile, Site, Sensor), "r");
        }     

    }else if(argc > 1 && (strcmp(argv[1], "setup") == 0)){
        printf("設定ファイルを作成します.\n");
        fp = fopen(BuildConfig(configFile, Site, Sensor), "r");        
    }

    /// Load config file.
    while(fgets(lineString, LINE_SIZE, fp) != NULL){
        static int8_t id = 0;
        char *ptr;

        ptr = strtok(lineString, ",");     // First
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

    /// Reset sensor board hardware.
    if (DeviceReset() != NO_ERROR){
        perror("センサーデバイスの初期化に失敗しました... プログラムを終了します.\n");
        exit(EXIT_FAILURE);
    }
    /// Obtain the device marking.
    if (GetDeviceMarking(&deviceMarking[0], sizeof(deviceMarking)) != NO_ERROR) {
        perror("センサーのシリアルコード取得に失敗しました... プログラムを終了します.\n");
        exit(EXIT_FAILURE);
    }
    /// Opening message.
    printf("センサーの読み取りを開始します.\nセンサーのシリアルコードは, %s です.\n\n", deviceMarking);
        
    if (StartContinuousMeasurement() != NO_ERROR) {
        perror("センサーを連続読み出しモードに設定できませんでした... プログラムを終了します.\n");
        exit(EXIT_FAILURE);
    }        
    /// At the beginning, makes a dummy read once.
    usleep(500000);
    BlankRead();

    /// previous dir control and prepare working directory.**/
    strcat(strcat(strcat(strcat(uploadFile, currentPath), "/"), UPLOAD_PATH), UPLOAD_FILE);

    /** main loop */
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
                printf("Saved into \"%s\"\n\n", uploadFile);

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

    // send stop command "0x01" to stop measurement to sfa30: sfa3x_i2c.c
    // sens_error = sfa3x_stop_measurement();
    // printf("センサーを停止します。\n");

    /** Exit from this application program **/
    return 0;
}


/** Build "config" file.
 *  path:   currentPath
 *  return value: Not 0 means exist, 0 means Not exist. */
char* BuildConfig(char *f, LOCATION place, POINT* sensor)
{
    char ans[2];
    int8_t i;

    printf("設定ファイル\"%s\"を作成します.\n", f);
    printf("okであれば \"y\"を, 中止する場合は... \"n\"を入力してください.\n");
    while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
        scanf("%s", ans);        
        if(strcmp(ans, "y") == 0) break;
        else if(strcmp(ans, "n") == 0) exit(-1);
    }
    ans[0] = '\0';

    /** Set Site name */
    while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
        printf("測定サイト名(Location)を入力してください... ");
        scanf("%s", place.name);
        printf("測定ポイント数(センサー数)を入力してください... ");
        scanf("%hhd", &place.num);
        putchar('\n');
        printf("測定サイト名は... \"%s\"\n測定ポイント数は... \"%d\"\n", place.name, place.num);
        putchar('\n');
        printf("確認OKの場合は\"y\", 変更する場合は\"n\"を入力してください.\n");
        scanf("%s", ans);
        if(strcmp(ans, "y") == 0) break;
    }
    ans[0] = '\0';
    putchar('\n');
    /** Set Sensor points */
    while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
        for(i = 0; i < place.num; i++){
            printf("ポイント(センサー)名を入力してください... ");
            scanf("%s", sensor[i].name);
            printf("測定値の単位を入力してください... ");
            scanf("%s", sensor[i].unit);
            sensor[i].id = i + 1;
        }
        putchar('\n');
        printf("測定サイト: \"%s\" (測定ポイント数 %d)の測定ポイントは\n", place.name, place.num);
        printf("Sensor ID,Sensor,unit\n");
        for(i = 0; i < place.num; i++){
            printf("%2hhd, %-8s, %2s\n", sensor[i].id, sensor[i].name, sensor[i].unit); 
        }
        putchar('\n');
        printf("確認OKの場合は\"y\",\n変更する場合は\"n\"を入力してください.\n");
        
        scanf("%s", ans);
        if(strcmp(ans, "y") == 0) break;
    }
    ans[0] = '\0';
    putchar('\n');
    
    FILE *fs = fopen(f,"w");

    if (fs == NULL){
        perror("設定ファイルを開けません.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fs, "location,%s,%d\n",place.name,place.num);
    for(i =0; i < place.num; i++){
        fprintf(fs, "point,%hhd,%s,%s\n",sensor[i].id, sensor[i].name, sensor[i].unit);
    }
    fclose(fs);
    
    return f;
}

