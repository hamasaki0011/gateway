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





/** Confirm "config" file is exist or not.
 *  path:   currentPath
    return value: Not 0 means exist, 0 means Not exist. */

/*
void SetupConfig(const char fname)
{
    char str[LINE_SIZE];
    FILE *fp; //FILE structure.

    if(getcwd(currentPath, PATH_SIZE) == NULL){
        printf("カレントディレクトリーが取得できません.\nプログラムを終了します.\n");
        return -1;    
    }
    // strcpy(configFileName, currentPath);
    strcpy(fname, currentPath);
    // strcat(strcat(configFileName, "/"), CONFIG_FILE);
    strcat(strcat(fname, "/"), CONFIG_FILE);

    // fp = fopen(configFileName, "r");
    fp = fopen(fname, "r"); 
    if (fp == NULL){
        // printf("指定された \"%s\" ファイルがありません.\nプログラムを終了します.\n", configFileName);
        printf("指定された \"%s\" ファイルがありません.\nプログラムを終了します.\n", fname);
        return -1;
    }
    while(fgets(str, LINE_SIZE, fp) != NULL){
        static int8_t id = 0;
        char *ptr;

        ptr = strtok(str, ",");     // First
        if(strcmp(ptr, "Site") == 0){
            ptr = strtok(NULL, ",");    // Site.name
            strcpy(Site.name, ptr);
            
            ptr = strtok(NULL, ",");    // Number
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
}
*/

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
