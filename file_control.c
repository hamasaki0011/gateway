#include <stdio.h>      // printf(), strcmp(), fopen(), perror()
#include <stdlib.h>     // malloc(), atoi()
#include <string.h>     // memset(), strtok()
#include <unistd.h>     // sleep()/usleep(), close(), open(), read(), write()
#include <dirent.h>     // For directory operation.
#include <sys/stat.h>   // mkdir()
#include <stdbool.h>    // For bool operation, true and false.
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

char* SetUploadFile(char* uploadFile, char* fileName){
    DIR *dir = opendir(UPLOAD_PATH);

    if (!dir){
        if(mkdir(UPLOAD_PATH, 0755)){   /// Make work folder.
            perror("ワークフォルダーの作成に失敗しました.\n");
            exit(EXIT_FAILURE);
        }
    }
    // uploadFile[0] = '0';
    strcat(strcat(uploadFile, UPLOAD_PATH), fileName);

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

int8_t Logging(char* log, char* msg, char* timeStamp)
{
    int8_t res = 0;

    FILE *fl; //FILE structure.
    fl = fopen(log, "a");
    if(fl == NULL){    
        res = -1;
        printf("file_control #60 Could Not open log file and res is%d\n", res);
        return res;
    }
    
    fprintf(fl, "%s: %s", timeStamp, msg);
    fclose(fl);
    printf("file_control #66 close log and res is %d\n", res);
    return res;
}

/** Display the upload format */
void DisplayUploadFormat(LOCATION lo, POINT* se, char* uf){
    int8_t i;

    printf("測定サイト: \"%s\" (測定ポイント数 %d)\n\n", lo.name, lo.num);

    printf("センサーID,センサー名称,測定単位\n");
    printf("-------------------------------------\n");
    for(i = 0; i < lo.num; i++){
        printf("%hhd,%s,%s\n", se[i].id, se[i].name, se[i].unit); 
    }
    putchar('\n');
    printf("アップロードファイル名: %s\n", uf);
    return;
}

/** Build "config" file.
 *  path:   currentPath
 *  return value: Not 0 means exist, 0 means Not exist. */
char* BuildConfig(char *f, LOCATION place, POINT* sensor, char* uf)
{
    char ans[2];
    int8_t i;

    /// Set Site name
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
    /// Set Sensor points
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

        DisplayUploadFormat(place, sensor, uf);
        putchar('\n');
        
        printf("確認OKの場合は\"y\",\n変更する場合は\"n\"を入力してください.\n");     
        scanf("%s", ans);
        if(strcmp(ans, "y") == 0) break;
        else if(strcmp(ans, "n") == 0){
            // printf("プログラムを終了します.\n");
            printf("修正する項目の番号を入力してください\n");
            printf("0 ... 測定サイト名\n");
            printf("1 ... センサーポイント数\n");
            printf("2 ... センサー設定\n");
            scanf("%hhd", &res);
            while(res < 0 || res > 2){
                switch(res){
                    case 0:
                        printf("すべて変更します\n");
                        BuildConfig(f, place, sensor, uf);
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
        fprintf(fs, "point,%hhd,%s,%s\n",sensor[i].id, sensor[i].name, sensor[i].unit);
    }
    fclose(fs);
    
    return f;
}

/** Display "config" file on screen.
 *  f:   configFile
 *  return value: None. */
void DisplayConfig(char *f)
{
    char readLine[LINE_SIZE];
    LOCATION lo;
    POINT se[16];
    char uf[256];

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

        }else if(strcmp(ptr, "point") == 0){
            ptr = strtok(NULL, ",");    // Sensor ID
            se[id].id = atoi(ptr);             

            ptr = strtok(NULL, ",");    // Sensor NAME
            strcpy(se[id].name, ptr);

            ptr = strtok(NULL, ",");    // Sensor UNIT
            strcpy(se[id].unit, ptr);

            id++;

        }else{
            ptr = strtok(NULL, ",");    // file name
            strcpy(uf, ptr);
        }
    }
    fclose(fp);
    DisplayUploadFormat(lo, se, uf);
    putchar('\n');
    return;
}

int8_t CreateUploadFile(char* uploadFile, LOCATION Site, POINT* Sensor, char* now)
{
    uint8_t i;
    FILE *f;

    f = fopen(uploadFile,"w");
    if (f == NULL){
        fclose(f);
        perror("ファイルにアクセスすることができません... プログラムを終了します.\n");
        return -1;
    }
    /// Record header.
    fprintf(f, "measured_date,measured_value,sensor,place\n");
    for(i = 0; i < Site.num; i++){
        fprintf(f, "%s,%0.1f,%s,%s\n", now, Sensor[i].data, Sensor[i].name, Site.name);
    }
    fclose(f);
    printf("\"%s\" saved\n\n", uploadFile);
    
    return 0;
}

/*
char* ReadJsonFile(char* f, char* str)
{
    char line[512];
    char *qtr;
    char s[1024];
    char buf[1024];
    int x;

    SETUP json;
    
    FILE *fj = fopen(f, "r");

    str[0] = '\0';
    while(fgets(line, LINE_SIZE, fj) != NULL){
        char *ptr;

        ptr = strtok(line, " \n");
        strcat(str, ptr);

    }
    fclose(fj);

    strcat(s, str);
    // 最初にbuf がどうなっているのか整数でみてみる
    for (int i = 0 ; i < 20 ; i++){
        printf("%d\n", buf[i]);
    }
    //qtr = strtok(s, "{");
    x = fgetc(s);

    printf("file_#260 str is %s\n", str);
    printf("file_#261 qtr is %s\n", qtr);

    return str;

}
*/
