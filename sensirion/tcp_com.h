#include <stdint.h>

#define SERVER_ADDR "10.10.210.1"
#define SERVER_PORT 8080    //8080
#define BUF_SIZE 1024
// network status => comStatus
#define DISCONNECT              0
#define SUCCESS_OPEN_SOCKET     10
#define SUCCESS_BIND_SOCKET     11
#define SUCCESS_START_LISTEN    12
#define ACCEPT_REQUEST          13
#define RECEIVE_REQUEST         14
#define CONNECT                 1
//netwok error status -> netErrStatus
#define FAILED_GEN_SOCKET       10
#define FAILED_BIND_SOCKET      20
#define FAILED_START_LISTEN     30
#define FAILED_ACCEPT_REQUEST   40
#define FAILED_RECEIVE_REQUEST  50
#define SHUT_DOWN_NETWORK       60
//Sensor error status
#define FAILED_MAKE_RESET       10

typedef struct{
    char *gasName;     // gasName name
    char *humid;    // humidity
    char *temp;     // temperature
    
    float gas;
    float humidity;
    float temperature;
}Sensor_data;

//転送用構造体
/*
typedef struct {
    float prm_pc_to_soc[20];    //PC→SoCへの転送パラメータ
    float prm_soc_to_pc[20];    //SoC→PChへの転送パラメータ
} Transfer_data;
*/

int8_t GenerateSocket(void);
int8_t Transfer(void);  /** it should return command buffer pointer **/
void CmdAnalysis(int16_t, char*);
int8_t Connect(int16_t);
int8_t StartMeasure(int16_t);
int8_t StopSensing(void);
int8_t SendData0(int16_t);
int8_t SendData(int16_t, Sensor_data);
int8_t SendStatus(int16_t, char[]);
int8_t SendDM(int16_t);
int8_t FinishProcess(int16_t);
int8_t OtherCMD(int16_t);
int8_t ReadMeasure0(void);
Sensor_data ReadMeasure();
