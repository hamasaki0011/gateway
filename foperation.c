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

#define CONFIG_LINE             512  // Max. value of number of bytes in line in config file.

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
