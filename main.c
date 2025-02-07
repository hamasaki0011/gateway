#include <stdio.h>      // printf(), strcmp(), fopen(), perror()
#include <stdlib.h>     // malloc(), atoi()
#include <string.h>     // memset(), strtok()
#include <unistd.h>     // sleep()/usleep(), close(), open(), read(), write()
#include <time.h>       // time()
#include <stdbool.h>    // For bool operation, true and false.

#include "common.h"
#include "device.h"
#include "file_control.h"
#include "main.h"

/** Raspberry Pi specific configuration.
 *  Adjust the following define to the device path of the sensor. */
//#define I2C_DEVICE_PATH         "/dev/i2c-1"

#define FILE_NAME_SIZE  256
#define LINE_SIZE       512  // Max. value of number of bytes in line in config file.

#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
#define FAILED_START_SENSOR     30

/**
 * The main function reads configuration settings, initializes sensor devices, continuously reads
 * sensor data, and saves the data periodically to a file.
 * 
 * @param argc `argc` is the argument count, which represents the number of arguments passed to the
 * program when it is executed from the command line. It includes the name of the program itself as the
 * first argument.
 * @param argv In the provided code snippet, the `argv` parameter in the `main` function represents an
 * array of strings that contain the command-line arguments passed to the program when it is executed.
 * Each element of the `argv` array is a null-terminated C-string, where `argv[0]`
 * 
 * @return The `main` function is returning an integer value of 0, which typically indicates successful
 * execution of the program.
 */
int main(int argc, char *argv[]){
    /// Define DIR_PATH "/home/pi/works/upload_file" and work file name is testWork.csv
    static char uploadFile[FILE_NAME_SIZE], configFile[FILE_NAME_SIZE];
    char readLine[LINE_SIZE], ans[2];
    unsigned char deviceMarking[32];
    
    struct tm *local;
    LOCATION Site;
    /// Site_id:    Site.id 
    /// Site_nam  Site.name
    /// Site_num:   Site.num:
    POINT Sensor[16];   // Sensor's information whish is located at the monitor site.
    /// Sensor_id:    Sensor[id].id 
    /// Sensor_name:  Sensor[id].name
    /// Sensor_unit:   Sensor[id].unit

    /// Find current directory and Set config file' name and path.
    strcpy(configFile, GetConfig(configFile));

    /// Set uploadFile.
    strcpy(uploadFile, SetUploadFile(uploadFile));

    FILE *fp; //FILE structure.
    fp = fopen(configFile, "r");
    /// Normal operation.
    if(fp == NULL){
        if(argc <= 1){
            printf("設定ファイルがありません.\n作成しますか？\n");
            printf("作成する場合は ... \"y\"を\n中止する場合は(プログラムを終了します.) ... \"n\"を入力してください.\n");
            while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
                scanf("%s", ans);        
                if(strcmp(ans, "y") == 0){
                    fp = fopen(BuildConfigAll(configFile, Site, Sensor), "r");
                    break;
                }
                else if(strcmp(ans, "n") == 0) {
                    /* ［要検討］作成しない場合の対処 */
                    printf("プログラムを終了します.\n");
                    return -1;
                }
            }

        }else if(argc > 1 && (strcmp(argv[1], "setup") == 0)){
            printf("設定ファイルを作成します.\n");
            fp = fopen(BuildConfigAll(configFile, Site, Sensor), "r");
        }
    }else{
        if(argc > 1 && (strcmp(argv[1], "setup") == 0)){
            printf("設定ファイルが既に存在します.\n");
            DisplayConfig(configFile);
            printf("設定ファイルを作り直す場合は... \"y\"を\n中止する場合は... \"n\"を入力してください.\n");
            while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
                scanf("%s", ans);        
                if(strcmp(ans, "y") == 0){
                    fp = fopen(BuildConfigAll(configFile, Site, Sensor), "r");
                    break;
                }
                else if(strcmp(ans, "n") == 0) break;
            }
        }
    }        

    // if(argc <= 1){
    //     char ans[2];

    //     if (fp == NULL){
    //         printf("設定ファイルがありません.\n作成しますか？\n");
    //         printf("作成する場合は ... \"y\"を\n中止する場合は(プログラムを終了します.) ... \"n\"を入力してください.\n");
    //         while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
    //             scanf("%s", ans);        
    //             if(strcmp(ans, "y") == 0){
    //                 fp = fopen(BuildConfig(configFile, Site, Sensor), "r");
    //                 break;
    //             }
    //             else if(strcmp(ans, "n") == 0) {
    //                 /* ［要検討］作成しない場合の対処 */
    //                 printf("プログラムを終了します.\n");
    //                 return -1;
    //             }
    //         }
    //     }     

    // }else if(argc > 1 && (strcmp(argv[1], "setup") == 0)){
    //     char ans[2];

    //     if (fp == NULL){
    //         printf("設定ファイルを作成します.\n");
    //         fp = fopen(BuildConfig(configFile, Site, Sensor), "r");
    //     }else{
    //         printf("設定ファイルが既に存在します.\n");
    //         DisplayConfig(configFile);
    //         printf("設定ファイルを作り直す場合は... \"y\"を\n中止する場合は... \"n\"を入力してください.\n");
    //         while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
    //             scanf("%s", ans);        
    //             if(strcmp(ans, "y") == 0){
    //                 fp = fopen(BuildConfig(configFile, Site, Sensor), "r");
    //                 break;
    //             }
    //             else if(strcmp(ans, "n") == 0) break;
    //         }
    //     }
    // }

    /// Load config file.
    while(fgets(readLine, LINE_SIZE, fp) != NULL){
        static int8_t id = 0;
        char *ptr;

        ptr = strtok(readLine, ",");     // First
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
    printf("センサーの読み取りを開始します.\nセンサーのシリアルコードは %s です.\n\n", deviceMarking);
        
    if (StartContinuousMeasurement() != NO_ERROR) {
        perror("センサーを連続読み出しモードに設定できませんでした... プログラムを終了します.\n");
        exit(EXIT_FAILURE);
    }        
    /// At the beginning, makes a dummy read once.
    usleep(500000);
    BlankRead();

    /** main loop */
    for (;;) {
        static uint8_t flgRec = 0, flgDisp = 0;
        time_t timer;
        int year, month, day, hour, minute, second;
        char dateNow[32], timeNow[16], now[48];

        /** Case 0: Using a data logger connecting one or many sensors,
         * and it is connected to a lap-top computer via serial communication interface. */

        /** Case 1: Using a sensirion sensor and it embedded temperature and humidity sensor in it,
         * and it is connected with Raspberry Pi via I2C interface.  */

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
            if(flgDisp == 0){
                flgDisp = 1;
                float data1, data2, data3;
                int8_t i;

                /// Display current time' information on console.
                printf("%s\n", dateNow);
                
                /// Read Sensirion sensor' data and display on the screen.
                if(ReadMeasuredValues(&data1, &data2, &data3) != 0){
                    for(i = 0; i < Site.num; i++) Sensor[i].data = 0.0;
                    printf("Failed to read Sensor data.\n");

                }else{
                    Sensor[0].data = data1;
                    Sensor[1].data = data2;
                    Sensor[2].data = data3;
                }
                
                printf("%s  @%s\n", Site.name, timeNow);
                for(i = 0; i < Site.num; i++){
                    printf("%s: %.1f %s", Sensor[i].name, Sensor[i].data, Sensor[i].unit);
                }
                putchar('\n');
            }

            if(second == 0 && flgRec == 0){
                flgRec = 1;
                int8_t i;

                FILE *fp = fopen(uploadFile,"w");
                
                if (fp == NULL){
                    perror("ファイルにアクセスすることができません... プログラムを終了します.\n");
                    return -1;
                }

                /// Record header.
                fprintf(fp, "measured_date,measured_value,sensor,place\n");
                for(i = 0; i < Site.num; i++){
                    fprintf(fp, "%s,%0.1f,%s,%s\n", now, Sensor[i].data, Sensor[i].name, Site.name);
                }
                fclose(fp);
                printf("Saved into \"%s\"\n\n", uploadFile);

            }
        }else{
            flgDisp = 0;
            flgRec = 0;
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

/**
 * The function `BuildConfigAll` takes user input to create a configuration file with location and sensor
 * information.
 * 
 * @param f The `f` parameter in the `BuildConfigAll` function is a pointer to a character array, which is
 * used to store the file name where the configuration settings will be saved. The function reads user
 * input to set the location name, number of sensor points, sensor names, and units, and then
 * @param place The `place` parameter in the `BuildConfigAll` function represents the location information
 * for a measurement site. It includes the following fields:
 * @param sensor The `sensor` parameter in the `BuildConfigAll` function is a pointer to an array of
 * `POINT` structures. Each `POINT` structure represents a sensor point and contains the following
 * fields:
 * 
 * @return The function `BuildConfigAll` is returning a `char*` which is the file name `f` that was passed
 * as an argument to the function.
 */
char* BuildConfigAll(char *f, LOCATION place, POINT* sensor)
{
    char ans[2];
    int8_t i;

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
    // while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
    while(true){
        int8_t res = -1;

        for(i = 0; i < place.num; i++){
            printf("ポイント(センサー)名を入力してください... ");
            scanf("%s", sensor[i].name);
            printf("測定値の単位を入力してください... ");
            scanf("%s", sensor[i].unit);
            sensor[i].id = i + 1;
        }

        DisplaySetting(place, sensor);
        putchar('\n');
        
        printf("確認OKの場合は\"y\",\n変更する場合は\"n\"を入力してください.\n");     
        scanf("%s", ans);
        if(strcmp(ans, "y") == 0) break;
        else if(strcmp(ans, "n") == 0){
            // printf("プログラムを終了します.\n");
            printf("修正する項目の番号を入力してください\n");
            printf("0 ... すべての項目\n");
            printf("1 ... センサーポイント数\n");
            printf("2 ... センサー設定\n");
            scanf("%hhd", &res);
            while(true){
                switch(res){
                    case 0:
                        printf("すべて変更します\n");
                        /*これ以降の表示が重複して現れる*/
                        BuildConfigAll(f, place, sensor);
                        break;
                    case 1:
                        printf("センサーポイント数を変更します。\n");
                        /*少なく変えると、以前の値も有効となる
                        多く入れるとエラーになる?どこで*/
                        BuildConfigPointNum(f, place, sensor);
                        break;
                    case 2:
                        printf("センサー設定を変更します。\n");
                        // BuildConfig(f, place, sensor);
                        break;                    
                    default:
                        ;
                }
                break;
            }
            break;
        }
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
    printf("新しい設定ファイルでプログラムを実行します.\n");
    return f;
}

char* BuildConfigPointNum(char *f, LOCATION place, POINT* sensor)
{
    char ans[2];
    int8_t i;

    /** Set Site name */
    while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
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
    // while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
    while(true){
        int8_t res = -1;

        for(i = 0; i < place.num; i++){
            printf("ポイント(センサー)名を入力してください... ");
            scanf("%s", sensor[i].name);
            printf("測定値の単位を入力してください... ");
            scanf("%s", sensor[i].unit);
            sensor[i].id = i + 1;
        }

        DisplaySetting(place, sensor);
        putchar('\n');
        
        printf("確認OKの場合は\"y\",\n変更する場合は\"n\"を入力してください.\n");     
        scanf("%s", ans);
        if(strcmp(ans, "y") == 0) break;
        else if(strcmp(ans, "n") == 0){
            // printf("プログラムを終了します.\n");
            printf("修正する項目の番号を入力してください\n");
            printf("0 ... すべての項目\n");
            printf("1 ... センサーポイント数\n");
            printf("2 ... センサー設定\n");
            scanf("%hhd", &res);
            while(true){
                switch(res){
                    case 0:
                        printf("すべて変更します\n");
                        BuildConfigAll(f, place, sensor);

                        break;
                    case 1:
                        printf("センサーポイント数を変更します。\n");
                        // BuildConfig(f, place, sensor);
                        break;
                    case 2:
                        printf("センサー設定を変更します。\n");
                        // BuildConfig(f, place, sensor);
                        break;                    
                    default:
                        ;
                }
                break;
            }
            break;
        }
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
        printf("num is %d\", place.num);
        fprintf(fs, "point,%hhd,%s,%s\n",sensor[i].id, sensor[i].name, sensor[i].unit);
    }
    fclose(fs);
    printf("新しい設定ファイルでプログラムを実行します.\n");
    return f;
}

/**
 * The function `DisplayConfig` reads a configuration file, extracts location and sensor information,
 * and then displays the settings.
 * 
 * @param f The parameter `f` in the `DisplayConfig` function is a pointer to a character array, which
 * represents the file name of the configuration file that needs to be read and processed.
 * 
 * @return The function `DisplayConfig` is returning `void`, which means it does not return any value.
 */
void DisplayConfig(char *f)
{
    char readLine[LINE_SIZE];
    // int8_t i;
    LOCATION lo;
    POINT se[16];

    printf("\"%s\"の設定は ... \n", f);
    FILE *fp = fopen(f,"r");
    if (fp == NULL){
        perror("設定ファイルを開けません.\n");
        exit(EXIT_FAILURE);
    }
    /// Read config file.
    while(fgets(readLine, LINE_SIZE, fp) != NULL){
        static int8_t id = 0;
        char *ptr;

        ptr = strtok(readLine, ",");     // First
        if(strcmp(ptr, "location") == 0){
            ptr = strtok(NULL, ",");    // Site.name
            strcpy(lo.name, ptr);

            ptr = strtok(NULL, ",");    // Number
            lo.num = atoi(ptr);            
        }else{
            ptr = strtok(NULL, ",");    // Sensor ID
            se[id].id = atoi(ptr);             

            ptr = strtok(NULL, ",");    // Sensor NAME
            strcpy(se[id].name, ptr);

            ptr = strtok(NULL, ",");    // Sensor UNIT
            strcpy(se[id].unit, ptr);

            id++;
        }
    }
    fclose(fp);
    DisplaySetting(lo, se);
    putchar('\n');
    return;
}
