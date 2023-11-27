/**
 * This program is for text file read and/or write ptorgam
 * Memo for mode attribute
 * r :読み込み。ファイルがない時は失敗。
 * r+:読み書き。ファイルがない時は失敗。
 * w :書き込み。ファイルがあっても空のファイルを作る。
 * w+:読み書き。ファイルがあっても空のファイルを作る。
 * a :追加書き込み。ファイルがない時は作る。
 * a+:追加読み書き。ファイルがない時は作る
 * 
 * behave fgets()
 * 読み込んだ文字列の終端には、自動的にヌル文字「\0」が付与される。
 * ヌル文字を含めた読み込みサイズがファイルから読み込まれる。
 * 改行が途中に登場した場合は、後ろにヌル文字を付与し読み込みを止める。
 * 全ての文字を読み込んだ次の呼び出し時に「NULLポインタ」が取得される。
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

int8_t WriteLogAll(char* name, uint8_t code, char* msg, int32_t cntr, int16_t sock){
  static uint8_t repeat = 0;
  time_t t = time(NULL);
  const char *s = ctime(&t);
  char logData[256] = "";
  char *temp = (char*)s;
  
  temp = strtok(temp, "\r\n");
  sprintf(logData, "%s, %d, %s, %d, %d\n", temp, code, msg, cntr, sock);
  
  //strcat(strcat(strcpy(logData, temp), ", "), msg);
    
  FILE *fp = NULL;
  fp = fopen(name, "a+");
  repeat++;
  if (fp == NULL){
    printf("Failed to write file : %d\n", repeat);
    return -1;
  }
  fputs(logData, fp);
  //fprintf(file, logData);
  fclose(fp);

  return 0;
  }

int8_t WriteLog(char* name, char* str){
  time_t t = time(NULL);
  const char *s = ctime(&t);
  char logData[128] = "";
  char *temp = (char*)s;
  /*
  while(*(temp++) != '\0');
  temp -= 1;
  */
  temp = strtok(temp, "\r\n");
  strcat(strcat(strcpy(logData, temp), ", "), str);
    
  FILE *fp = NULL;
  fp = fopen(name, "a+");
  if (fp == NULL){
    printf("file can't write\n");
    return -1;
  }
  fputs(logData, fp);
  //fprintf(file, logData);
  fclose(fp);

  return 0;
  }
  
/**read every 1 line
 * char* fgets(char *buf, int size, FILE *fp) **/
//* int8_t ReadLog(uint8_t* name){
int8_t ReadLog(char* name){
  char logData[128];
  FILE *fp = NULL;
  //fopen_s(&file, name, "r");
  fp = fopen(name, "r");
  //fscanf(&file, "%s", &s);
  
  if (fp == NULL){
    printf("file open error\n");
    return -1;
  }
  
  while(fgets(logData, 128, fp) != NULL){
    printf("%s", logData);
  }
  fclose(fp);
  //printf("%s\n", &s);
  return 0;
}
