/* Copyright (c) 2024 */

#ifndef MAIN_H
#define MAIN_H

/* `#include <stdint.h>` is a preprocessor directive that includes 
the standard C header file `stdint.h` in your code. 
This header file defines a set of typedefs for integer types with specified widths, 
which are commonly used in C programming to ensure portability and consistency of 
integer types across different platforms. 
By including `stdint.h`, you can use types like `int8_t`, `uint8_t`, `int16_t`, 
`uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, etc., with known sizes 
regardless of the underlying system architecture. */
#include <stdint.h>

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
    char key[64];
    char c_value[64];
    int i_value;
    float f_value;
}SETUP;
typedef struct{
    char name[128]; // location_name
    int8_t num;     // sensor count
}LOCATION;
typedef struct{
    uint8_t id;     // sensor_ID
    char name[256]; // sensor_name
    char unit[8];   // sensor_unit
    
    float data;     // measured data
}POINT;

/** The function `BuildConfigAll` takes three parameters:
 a `char*` representing a configuration string,
 a `LOCATION` struct containing location information, 
 and an array of `POINT` structs representing sensor data. */
char* BuildConfig(char*, LOCATION, POINT*, char*);
char* BuildConfigPointNum(char*, LOCATION, POINT*);

/*The `void DisplayConfig(char*);` function declaration
 in the `main.h` header file is defining a function named `DisplayConfig` 
 that takes a single parameter of type `char*`. 
 This function is expected to display or output some configuration information 
 based on the provided `char*` parameter.
 The actual implementation of this function is not provided in the header file, 
 so it would need to be defined in a corresponding source file (.c file) 
 where the functionality of displaying the configuration information is implemented.*/
void DisplayConfig(char*);

#endif /* MAIN_H */
