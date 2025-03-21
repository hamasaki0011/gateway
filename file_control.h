/* Copyright (c) 2025 */

#ifndef FILE_CONTROL_H
#define FILE_CONTROL_H

#include "main.h"

#define FILE_NAME_SIZE  256
#define PATH_SIZE       128
#define CONFIG_FILE     "config"           // Setup file
#define UPLOAD_FILE     "testWork.csv"     // Upload file
#define UPLOAD_PATH     "../upload_file/"  // Upload file
#define LOG_FILE		"senseing.log"     // Log file

char* GetConfig(char*);
char* SetUploadFile(char*);
char* SetLogFile(char*);
int8_t Logging(char*, char*);
void DisplaySetting(LOCATION, POINT*);

#endif /* MAIN_H */
