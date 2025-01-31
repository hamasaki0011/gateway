#include <stdio.h>      // printf(), strcmp(), fopen()
#include <stdlib.h>     // malloc() 
#include <string.h>     // memset()
#include <unistd.h>     // sleep()/usleep() and close(), open(), read(), write()
#include <time.h>       // time()
#include <dirent.h>     // For directory operation.
#include <stdbool.h>    // For bool operation.

#include "common.h"
#include "foperation.h"
#include "device.h"
#include "main.h"

/** Build "config" file.
 *  path:   currentPath
    return value: Not 0 means exist, 0 means Not exist. */
void BuildConfig(char *file)
{
    /** Setup operation class 2. From creating a setup file. **/
    char point[128];
    char ans[2];
    char fname[512];

    printf("foperation_#23 config is %s\n", file);
    ans[0] = '\0';
    printf("設定ファイルを用意している場合は... \"y\"を\n");
    printf("設定ファイルを作成する場合は... \"n\"を入力してください.\n");
    while(strcmp(ans, "y") != 0 || strcmp(ans, "n") != 0){
        scanf("%s", ans);
        printf("Your input is %s\n\n", ans);
        if(strcmp(ans, "y") == 0){
            printf("設定ファイル名をディレクトリーから指定してください.\n");
            scanf("%s", fname);
            printf("ファイル名は,\"%s\"ですね?\n\n", fname);
            break;
        }
    }
    printf("OKの場合は\"y\"を入力してください... ");
    while(strcmp(ans, "y") != 0){
        scanf("%s", ans);
        if(strcmp(ans, "y") == 0) break;
    }
    printf("Hi\n");

    printf("foperation_#24 設定ファイルを指定してください\n");
    // if(strcmp(argv[1], "setup") == 0){
    //     printf("main_#303 Let's make Sensor settings.\n\n");
            // 2023.11.24 Get the latest time
            // timer = time(NULL);
            // // Convert to localtime
            // local = localtime(&timer);
            // // 年月日と時分秒をtm構造体の各パラメタから変数に代入
            // year = local->tm_year + 1900;   // Because year count starts at 1900
            // month = local->tm_mon + 1;      // Because 0 indicates January.
            // day = local->tm_mday;
            // hour = local->tm_hour;
            // minute = local->tm_min;
            // second = local->tm_sec;
    //     /** Set place **/
    //     while(strcmp(ans, "y") != 0){
    //         printf("サイト名を設定してください... ");
    //         scanf("%s", Site.name);
    //         printf("your input is \"%s\"?\n\n", Site.name);
    //         printf("OKの場合は\"y\",変更する場合は\"n\"... ");
    //         scanf("%s", ans);
    //     }
    //     /** Initialize the variable char* ans with 0x00 code. **/
    //     ans[0] = 0x00;

    //     /** Prepare setup_file file. **/
    //     //strcat(strcat(fname, dir_path), configFile);
    //     FILE *fp = fopen(configFile,"w");
    //     if (fp == NULL){
    //         printf("The file: %s is NOT able to open.\n", configFile);
    //         return -1;
    //     }
    //     //fprintf(fp, "Set_up data @%d-%2d-%2d %2d:%2d\n",year, month, day, hour, minute);
    //     fprintf(fp, "place,%s\n",Site.name);
    //     printf("\"%s\"をサイト名として登録しました.\n\n", Site.name);
                
    //     /** Set Sensor Points **/
    //     while(strcmp(ans, "q") != 0){
    //         point_num++;
    //         while(strcmp(ans, "y") != 0){
    //             printf("次にセンサーポイントを設定してください...\n");
    //             scanf("%s", point);
    //             printf("your input is ""%s""?\n", point);
    //             printf("OKの場合はyを変更する場合はnを...");
    //             scanf("%s", ans);
    //         }
    //         fprintf(fp, "%d,%s\n",point_num, point);
    //         printf("\"%s\"をセンサーポイント名として登録しました.\n\n", point);
    //         printf("センサーポイント設定を継続する場合は\"r\",終了する場合は\"q\"を入力してください... ");
    //         scanf("%s", ans);
    //     }
    //     printf("main_#214 point_num is %d\n", point_num);
    //     printf("your work was terminated %s\n", ans);
    //     fclose(fp);
    // }else{
    //     printf("You have set a wrong parameter\nTherefore Terminate!");
    //     return -1;
    // }
}

/** Confirm the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists. */
int8_t AddFile(const char* path)
{
    FILE* fp = fopen(path, "a+");
    if (fp == NULL) {
        return 1;
    }
    fclose(fp);
    return 0;
}

/** Overwrite the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists. */
int8_t OverWriteFile(const char* path)
{
    int8_t res = 0;

    FILE* fp = fopen(path, "w+");
    if (fp == NULL) {
        printf("OverWrite_#398 Failed overwrite \"%s\".\n", path);
        res = -1;
    }
    fclose(fp);
    printf("OverWrite_#402 \"%s\" is overwritten.\n", path);
    return res;
}
