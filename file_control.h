/* Copyright (c) 2025 */

#ifndef FILE_CONTROL_H
#define FILE_CONTROL_H

#include "main.h"

#define FILE_NAME_SIZE  256
#define PATH_SIZE       128
#define LINE_SIZE       512
#define CONFIG_FILE     "config"            // Setup file
#define UPLOAD_FILE     "testWork.csv"          // Upload file
#define UPLOAD_PATH     "../upload_file/"  // Upload file

char* GetConfig(char*);
void LoadConfigSettings(char*, LOCATION, POINT*, char*);
char* SetUploadFile(char*);
void DisplayFormat(LOCATION, POINT*, char*);
void DisplayConfig(char*);
char* BuildConfig(char*, LOCATION, POINT*, char*);
char* ReadJsonFile(char*, char*);

#endif /* MAIN_H */