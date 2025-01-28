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
int8_t IsExistFile(const char* path)
{
    FILE* f = fopen(path, "r");
    if (f == NULL) return 0;
    fclose(f);
    return 1;
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
    printf("OverWrite_#402 \"%s\" is overwerwritten.\n", path);
    return res;
}
