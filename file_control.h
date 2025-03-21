/* Copyright (c) 2025 */

#ifndef FILE_CONTROL_H
#define FILE_CONTROL_H

#include "main.h"

#define FILE_NAME_SIZE  256
#define PATH_SIZE       128
<<<<<<< HEAD
#define LINE_SIZE       512
#define CONFIG_FILE     "config"            // Setup file
#define UPLOAD_FILE     "testWork.csv"          // Upload file
=======
#define CONFIG_FILE     "config"           // Setup file
#define UPLOAD_FILE     "testWork.csv"     // Upload file
>>>>>>> 1c170112868f9554a71e7d1a16e03042093437f8
#define UPLOAD_PATH     "../upload_file/"  // Upload file
#define LOG_FILE		"senseing.log"     // Log file

char* GetConfig(char*);
void LoadConfigSettings(char*, LOCATION, POINT*, char*);
char* SetUploadFile(char*);
<<<<<<< HEAD
void DisplayFormat(LOCATION, POINT*, char*);
void DisplayConfig(char*);
char* BuildConfig(char*, LOCATION, POINT*, char*);
char* ReadJsonFile(char*, char*);
=======
char* SetLogFile(char*);
int8_t Logging(char*, char*);
void DisplaySetting(LOCATION, POINT*);
>>>>>>> 1c170112868f9554a71e7d1a16e03042093437f8

#endif /* MAIN_H */
