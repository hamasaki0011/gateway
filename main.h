/*
 * Copyright (c) 2024
 */

#ifndef MAIN_H
#define MAIN_H

/** Typedef section for types commonly defined in <stdint.h>
 * If your system does not provide stdint headers, please define them
 * accordingly. Please make sure to define int64_t and uint64_t.*/
#include <stdint.h>

#include "foperation.h"

/** define structure **/
/** tm structure
* struct tm {
*  int tm_sec;      // 秒 [0-61] 最大2秒までのうるう秒を考慮
*  int tm_min;      // 分 [0-59] 
*  int tm_hour;     // 時 [0-23] 
*  int tm_mday;     // 日 [1-31]
*  int tm_mon;      // 月 [0-11] 0から始まることに注意
*  int tm_year;     // 年 [1900からの経過年数]
*  int tm_wday;     // 曜日 [0:日 1:月 ... 6:土]
*  int tm_yday;     // 年内の通し日数 [0-365] 0から始まることに注意
*  int tm_isdst;    // 夏時間が無効であれば 0 }; */

typedef struct{
    char name[128];     // location_name
    int8_t point_num;   // sensor count
}LOCATION;

typedef struct{
    uint8_t id;    // sensor_ID
    char name[256];   // sensor_name
    char unit[8];   // sensor_unit    
}SENSOR;

typedef struct{
    char *gasName;  // gasName
    char *humid;    // humidity
    char *temp;     // temperature
    
    float gas;
    float humidity;
    float temperature;
}Sensor_data;
LOCATION SetLocationName(char*, int8_t);
SENSOR SetSensor(char*, uint8_t, char*);

#endif /* MAIN_H */
