/*
 * Copyright (c) 2024
 */

#ifndef MAIN_H
#define MAIN_H

/** Typedef section for types commonly defined in <stdint.h>
 * If your system does not provide stdint headers, please define them
 * accordingly. Please make sure to define int64_t and uint64_t.*/
#include <stdint.h>

/** define structure **/
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

/** DummyRead function **/
int8_t ReadDummy(void);
LOCATION SetLocationName(char*, int8_t);
SENSOR SetSensor(char*, uint8_t, char*);

/*
 * Confirm the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists.
*/
int8_t AddFile(const char*);

/*
 * Overwrite the file exists or Not.
 * path:   file path.
 * return: 0: exist, 1: not exists.
*/
int8_t OverWriteFile(const char*);

#endif /* MAIN_H */
