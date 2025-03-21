#include <stdio.h>      // printf(), strcmp(), fopen(), perror()
#include <stdlib.h>     // malloc(), atoi()
#include <string.h>     // memset(), strtok()
#include <unistd.h>     // sleep()/usleep(), close(), open(), read(), write()
#include <dirent.h>     // For directory operation.
#include <sys/stat.h>   // mkdir()

#include "file_control.h"
#include "main.h"

char* GetConfig(char* file){
    char currentPath[FILE_NAME_SIZE];
    
    /// Find current directory and Set config file' name and path.
    if(getcwd(currentPath, PATH_SIZE) == NULL){
        perror("カレントディレクトリーを取得できません.\n");
        exit(EXIT_FAILURE);
    }
    strcat(strcat(strcpy(file, currentPath), "/"), CONFIG_FILE);

    return file;
}

char* SetUploadFile(char* uploadFile){

    DIR *dir = opendir(UPLOAD_PATH);
    
    if (!dir){
        if(mkdir(UPLOAD_PATH, 0755)){   /// Make work folder.
            perror("ワークフォルダーの作成に失敗しました.\n");
            exit(EXIT_FAILURE);
        }
    }
    strcat(strcat(uploadFile, UPLOAD_PATH),UPLOAD_FILE);

    return uploadFile;
}

char* SetLogFile(char* logFile){

    DIR *dir = opendir(UPLOAD_PATH);
    
    if (!dir){
        if(mkdir(UPLOAD_PATH, 0755)){   /// Make work folder.
            perror("ワークフォルダーの作成に失敗しました.\n");
            exit(EXIT_FAILURE);
        }
    }
    strcat(strcat(logFile, UPLOAD_PATH),LOG_FILE);

    return logFile;
}

int8_t Logging(char* log, char* logMessage)
{
    int8_t res = 0;
    FILE *fw; //FILE structure.
    
    fw = fopen(log, "w");
    printf("%s", logMessage);
    
    fclose(fw);
    printf("file_control #60 close log and res is %d\n", res);
    return res;
}

void DisplaySetting(LOCATION lo, POINT* se){
    int8_t i;

    printf("測定サイト: \"%s\" (測定ポイント数 %d)\n\n", lo.name, lo.num);

    printf("センサーID,センサー名称,測定単位\n");
    printf("-------------------------------------\n");
    for(i = 0; i < lo.num; i++){
        printf("%hhd,%s,%s\n", se[i].id, se[i].name, se[i].unit); 
    }
    return;
}
