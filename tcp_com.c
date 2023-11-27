#include <stdio.h>  // printf(), strcmp()
#include <string.h> // memset()
//#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>     // for close
#include <arpa/inet.h>
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h" 
#include "sfa3x_i2c.h"
#include "tcp_com.h"

/*
 * TO USE CONSOLE OUTPUT (PRINTF) YOU MAY NEED TO ADAPT THE INCLUDE ABOVE OR
 * DEFINE IT ACCORDING TO YOUR PLATFORM:
 * #define printf(...)
 */

/*2022/7/6 ~*/
//転送用構造体
/*
typedef struct {
    float prm_pc_to_soc[20];    //PC→SoCへの転送パラメータ
    float prm_soc_to_pc[20];    //SoC→PChへの転送パラメータ
} Transfer_data;
*/
// command status => status
#define STOP        0   // Stop Measurment
//#define D_INFO  10  // Device Marking requested 
#define START       20  // Start Measurement
#define READ        30  // Read data at once
#define SENSING     40  // Sensing
#define NOTSENSING  50  // Sensing
#define FINISH      60  // Finish application
#define OTHERS      90  // INVALID command
#define ERROR       127 // ERROR
/*
//sensor error status -> sensErrStatus
#define FAILED_MAKE_RESET       10
#define FAILED_GET_DEVICEMARK   20
*/
#define FAILED_START_SENSOR     30

extern uint8_t sensorStatus;    // Sensor' status
extern unsigned char device_marking[32];

char recv_buf[BUF_SIZE] = "";
char send_buf[BUF_SIZE] = "";
const char send_DM[] ="Device Marking: ";
//const char send_finish[] ="Disconnect\n";
const char send_sense[] ="read measurement data\n";
const char send_greeting[] ="Please send a command\n";
 
static uint8_t status = STOP;   //System status
uint8_t comStatus = 0;          // communication status
int16_t w_addr;
int16_t c_sock;
/** Server' addr structure **/
struct sockaddr_in a_addr;
/** Client' addr structure **/
struct sockaddr_in client;
/** Generate a hcho structure **/

int8_t GenerateSocket(){
    /** Generate a socket
     * 1st argument: address domain, select one of AF_INET, AF_INET6, AF_UNIX and AF_RAW
     * 2nd argument: socket type, one of SOCK_STREAM, SOCK_DGRAM and SOCK_RAW 
     * 3rd argument: protocol, one of 0, IPPROTO_UDP and IPPPROT_TCP **/
    w_addr = socket(AF_INET, SOCK_STREAM, 0);
    if (w_addr == -1) {
        char *msg = "Failed to generate socket.\n";
        printf(msg);
        comStatus = FAILED_GEN_SOCKET;
        
        // Return to main loop and try agin from the beginning
        return -1;
    }else{
        comStatus = SUCCESS_OPEN_SOCKET;
    }
      
    /** Initialize sckaddre structure **/
    memset(&a_addr, 0, sizeof(struct sockaddr_in));
    
    /** Set IP address and port number of Server' **/
    a_addr.sin_family = AF_INET;                            /** AF_INET: IPv4 communication **/
    a_addr.sin_port = htons((unsigned short)SERVER_PORT);   /** Port number is converted to network-byte-order **/ 
    a_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);        /** IP address is converted to network-byte-order **/     

    /** Bind Server' socket
     * 1st argument : socket descriptor
     * 2nd argument : pointer for socket addr structure
     * 3rd argument : size of address (byte) **/
    if (bind(w_addr, (const struct sockaddr *)&a_addr, sizeof(a_addr)) == -1) {
        char *msg = "Failed to bind to server' port.\n";
        printf(msg);
        comStatus = FAILED_BIND_SOCKET;
        /** close the socket **/
        //close(w_addr);
        
        // Return to main loop and try agin from the beginning
        return -1;
        
    }else{
        comStatus = SUCCESS_BIND_SOCKET;
    }
    /** ソケットを接続待ちに設定 =>listen()
     * 1st argument : socket descriptor
     * 2nd argument : backlog, maximaum size of queue **/
    if (listen(w_addr, 1) == -1) {
        char *msg = "Failed to set ""Listen"" state.\n";
        printf(msg);
        /** close the socket **/
        //close(w_addr);
        comStatus = FAILED_START_LISTEN;
        
        // Return to main loop and try agin from the beginning 
        return -1;
        
    }else{
        comStatus = SUCCESS_START_LISTEN;
    }
    /** Normal completion **/
    return 0;
}

/** 
 * MEMO: IP address and port information at client side
    //client.sin_family = AF_INET;
    Port number of client PC     
    //client.sin_port;
    IP address is converted to network-byte-order   
    //cient.sin_addr.s_addr; **/
int8_t Transfer(void){
    int8_t comStat;
    uint len = sizeof(client);
        
    /** 接続要求の受け付け（接続要求くるまで待ち） =>accept()
    * It would be executed every main loop
    * 1st argument : socket descriptor
    * 2nd argument : address of client PC
    * 3rd argument : address length
    * Clientから接続要求がくるまで終了しない。関数のなかでプログラムが待たされることになる。**/
    printf("Waiting connect... \n");

    c_sock = accept(w_addr, (struct sockaddr *)&client, &len);
    if (c_sock == -1) {
        /** in case of Failed to accept **/
        printf("Failed to accept Client' request.\n");
        //close(w_addr);
        comStatus = FAILED_ACCEPT_REQUEST;
                    
        // (1)Failed to accept Client' request then to exit form transfer loop.
        return comStat = -3;
    }else{
        /** in case of accepting the request from Client  **/
        int8_t errRecv = 0;
        printf("Accept connection from %s, (port= %d)\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        comStatus = ACCEPT_REQUEST;
        //printf("... c_sock = %d\n...netCntr = %d\n", c_sock, netCntr++);
        
        /** 接続済のソケットでデータのやり取り =>recv() **/
        memset(recv_buf, 0, BUF_SIZE);  /** Clear read buffer **/
        /** クライアントから文字列を受信: recv()
         * 1st argument : socket descriptor
         * 2nd argument : receive buffer pointer
         * 3rd argument : buffer length
         * 4th argument : flags one or more in MSG_CONNTERM, MSG_OOB, MSG_PEEK and MSG_WAITALL
         * later, need study how to use the 4th argument **/
         errRecv = recv(c_sock, recv_buf, BUF_SIZE, 0);
         if(errRecv == -1){
             printf("Receiption error has occurred\n");
             comStatus = FAILED_RECEIVE_REQUEST;
             //(a)write an error log
             //WriteLogAll(filename, comStatus, "Receiption error has occurred", c_sock, w_addr);
             //(b)repeatly this kind of error has occurred, we shuold give it up to continue
             
             //close(w_addr);
             
             // (2)Failed to receive request then to exit form transfer loop.
             return errRecv;
             
        }else if(errRecv == -2){
            printf("Shut down socket because Client is silent.\n");
            comStatus = SHUT_DOWN_NETWORK;
            
            //close(w_addr);
            //(3)Because server side was shut down, exit form transfer loop.
            return errRecv;
            
        }else{
            //char *msg = "Success to receive request";
            comStatus = RECEIVE_REQUEST;
            /** Analize a request from Client **/
            CmdAnalysis(c_sock, recv_buf);
        }
    }
    // Analize a request from Client 
    //CmdAnalysis(c_sock, recv_buf);
    
    // ソケット通信をクローズ
    close(c_sock);
    
    return comStat = 0;    
}

int8_t ReadMeasure0(void){
    int16_t errCode = 0;
    /*    
    // it may adjust the measurement interval around for 500ms: sensirion_i2c_hal.c
    sensirion_i2c_hal_sleep_usec(500000);
    */   
    /** read and store data: sfa3x_i2c.c **/    
    /*
    errCode = sfa3x_read_measured_values(&hcho.gas, &hcho.humidity, &hcho.temperature); 

    if (errCode) {
        printf("Error executing sfa3x_read_measured_values(): %i\n", errCode);

        return errCode = -1;
            
    } else {
        printf("%s: %.1f ppb\n", hcho.gasName, hcho.gas);
        printf("%s: %.2f %%RH\n", hcho.humid, hcho.humidity);
        printf("%s: %.2f °C\n", hcho.temp, hcho.temperature);
        putchar('\n');         
    }
    */
    return errCode = 0;
}

Sensor_data ReadMeasure(){
    int8_t errCode = 0;
    float data1, data2, data3;
    Sensor_data result = {"ホルムアルデヒド濃度", "相対湿度", "周囲温度"};
        
    // it may adjust the measurement interval around for 500ms: sensirion_i2c_hal.c
    sensirion_i2c_hal_sleep_usec(500000);
       
    /** read and store data: sfa3x_i2c.c **/
    errCode = sfa3x_read_measured_values(&data1, &data2, &data3);
    
    if(errCode != 0){
        printf("Failed to read measure data form Sensor\n");
        result.gas = 1.0;
        result.humidity = 1.0;
        result.temperature = 1.0;
    }
    else{
        result.gas = data1;
        result.humidity = data2;
        result.temperature = data3;
    }

    printf("errCode: %d\n", errCode);
    printf("%s: %.1f ppb\n", result.gasName, result.gas);
    printf("%s: %.2f %%RH\n", result.humid, result.humidity);
    printf("%s: %.2f °C\n", result.temp, result.temperature);
    putchar('\n');
    
    return result;
}


/** command processing **/
void CmdAnalysis(int16_t sock, char* str){
        
    if(strcmp(str, "connect\n") == 0){
        /** connecting processing **/
        if(comStatus != CONNECT){
            if(Connect(sock) == 0){ 
                printf("Connecting...\n");
                comStatus = CONNECT;
            }
            else{
                 printf("Failed to connect.\n");
                 comStatus = DISCONNECT; 
            }
        }else{
             printf("Already conected.\n");
             comStatus = CONNECT;
        }        
        //return CONNECT;
        
    }else if (strcmp(str, "finish\n") == 0) {
        /** disconnecting processiing **/
        if(FinishProcess(sock) == 0){
            printf("Close socket\n");
            // Close socket communication
            close(c_sock);
        }else{
            printf("Failed to close socket\n");
        }
        // close(c_sock);
        // return FINISH;
        
    }else if(strcmp(str, "reset\n") == 0){
        /** Initialize Sensor **/
        if(status == STOP){
            /** Make reset sensor hardware **/
            if (sfa3x_device_reset() != 0) {
                  printf("Failed to reset Sensor:\n");
                  sensorStatus = FAILED_MAKE_RESET;
                  char *msg = "Failed to reset Sensor\r\nPlease try again later\r\nor check Sensor' hardware";
                  SendStatus(sock, msg);
                  status = ERROR;
            }else{
                printf("Success to reset Sensor:\n");
                //sensorStatus = FAILED_MAKE_RESET;
                char *msg = "Success to reset Sensor\r\n";
                SendStatus(sock, msg);
                status = ERROR;
            }
        }else{
            char *msg = "Stopped to reset, because it is sensing\r\n";
            SendStatus(sock, msg);
            //status = START; // overwrite the status
        }
        /** wait around 200ms before Sensor operating **/
        //usleep(200000);
                
    }else if(strcmp(str, "start\n") == 0){
        /** Start command processing **/
        if(status == STOP){
            if(StartMeasure(sock) == 0){
                char *msg = "Start to measure...\r\n";
                SendStatus(sock, msg);
                status = START;
            }else{
                char *msg = "Failed to execute start command for sensor\r\nPlease try again\r\n";
                SendStatus(sock, msg);
                sensorStatus = FAILED_START_SENSOR;
                status = ERROR;
            }
        }else{
            char *msg = "Already started!\r\n";
            SendStatus(sock, msg);
            status = START; // overwrite the status
        }

    }else if(strcmp(str, "startsense\n") == 0){
        /** Start sensing command procedure **/
        if(status == START || status == STOP){
            if(StartMeasure(sock) == 0){
                char *msg = "Start continuous sensing...\r\n";
                SendStatus(sock, msg);
                status = SENSING;
            }else{
                char *msg = "Failed to execute start command for sensor\r\nPlease try again\r\n";
                SendStatus(sock, msg);
                sensorStatus = FAILED_START_SENSOR;
                status = ERROR;
            }
        }else if(status == SENSING){
            char *msg = "Already be sensing ...\r\n";
            SendStatus(sock, msg);
            status = SENSING; // overwrite the status
        }else{
            char *msg = "Already be sensing do you want to stop sensing?\r\n";
            SendStatus(sock, msg);
            status = SENSING; // overwrite the status
        }

    }else if(strcmp(str, "readstatus\n") == 0){
        /** request read operation status **/
        if(status == SENSING){
            char *msg = "Senseing data\r\n";
            SendStatus(sock, msg);
            status = SENSING;
        }else if(status == START){
            char *msg = "Started Measurement\r\n";
            SendStatus(sock, msg);
            status = START; // overwrite the status
        }else{
            char *msg = "Stopped Measurement\r\n";
            SendStatus(sock, msg);
            status = STOP; // overwrite the status            
        }

    }else if(strcmp(str, "stop\n") == 0){
        // stop measurement processing
        if(status != STOP){
            if(StopSensing() == 0){
                char *msg = "Stop measurement\r\n";
                SendStatus(sock, msg);
                status = STOP;
            }else{
                char *msg = "Failed to execute stop command for sensor\r\nPlease try again\r\n";
                SendStatus(sock, msg);
                status = ERROR;                    
            }                            
        }else{
            char *msg = "Already stopped!\r\n";
            SendStatus(sock, msg);
            status = STOP; // Is it ok?                
        }

        
    }else if(strcmp(str, "stopsense\n") == 0){
        // stop measurement processing
        if(status == SENSING){
            char *msg = "Stop continuous sensing\r\n";
            SendStatus(sock, msg);
            status = START;
            /*
            if(StopSensing(sock) == 0){
                char *msg = "Stop continupus sense\r\n";
                SendStatus(sock, msg);
                status = start;
            }else{
                char *msg = "Failed to execute stop command for sensor\r\nPlease try again\r\n";
                SendStatus(sock, msg);
                status = ERROR;                    
            }
            */                            
        }else{
            char *msg = "Already stopped continuous sense!\r\n";
            SendStatus(sock, msg);
            //status = STOP; // Is it ok?                
        }

        
    }else if(strcmp(str, "read\n") == 0){
        Sensor_data hcho;

        if(status != START && status != SENSING){
            char *msg = "Sensor is NOT started!\r\nIt should be started as first\r\n";  
            SendStatus(sock, msg);
            
        }else{
            /** read measurement data **/
            hcho = ReadMeasure();
            
            if(SendData(sock, hcho) == 0){    
                printf("Send data for client\n");
            }else{
                // if data send error has occurred
                printf("Failed to send data\n");
            }                 
        }
    }else if(strcmp(str, "sense\n") == 0){
        if(status != START){
            char *msg = "Sensor is NOT started!\r\nIt should be started as first\r\n";  
            SendStatus(sock, msg);
        }else{
                
            if(SendData0(sock) == 0){
                printf("Send data for client\n");
            }else{
                // if data send error has occurred
                printf("Failed to send data\n");
            }                 
        }
        //return SENSE;
        
    }else if(strcmp(str, "inf\n") == 0){
        // send device marking
        if(SendDM(sock) != 0){
            printf("Failed to send Device Marking\nWait a while and try again!");
        }
        //return D_INFO;
        
    }else {
        // why c_sock?
        if(0 == OtherCMD(c_sock)){
            printf("INVALID COMMAND!\n");
        }
        //return OTHERS;
    }
    //return 0;
}

int8_t Connect(int16_t sock){
    int8_t result = 0;
    
    /** Clear send_buf **/
    memset(send_buf, 0, 1024);
    strcat(send_buf, "Welcome!\n");
            
    result = send(sock, send_buf, sizeof(send_buf)/sizeof(send_buf[0]), 0);
    if (result == -1) printf("Failed to reply message.\n");
    
    return result;    
}

/*
 * Start Measurement
 * Send start command "0x00" to start measurement to sfa30
 * with vendor function in sfa3x_i2c.c
 */
int8_t StartMeasure(int16_t sock){
    //int16_t send_size = 0;
    int16_t error = 0;
    
    error = sfa3x_start_continuous_measurement();
        
    if (error) {
        printf("Failed to execute sfa3x_start_continuous_measurement(): %i\n", error);
        return 1;
    }
    // normal termination on start command
    return 0;
}

/**  Stop Measurement
 * Send stop command "0x01" to stop measurement to sfa30
 * with vendor function in sfa3x_i2c.c **/
int8_t StopSensing(){
    int8_t result = 0;

    if (sfa3x_stop_measurement()) {
        printf("Failed to execute sfa3x_stop_measurement()\n");
        result = -1;
    }else{
        printf("Stopped Sensor sensing\n");
        result = 0;
    }
    return result;
}

int8_t SendData0(int16_t sock){
    /*
    int16_t send_size = 0;
    
    if(ReadMeasure0() == 0){
        // Clear send_buf
        memset(send_buf, 0, 1024);
        // Send data with format
        sprintf(send_buf, "mData,%s,%.1f\r\n%s,%.1f\r\n%s,%.1f\r\n", hcho.gasName, hcho.gas, hcho.humid, hcho.humidity, hcho.temp, hcho.temperature);        
        send_size = send(sock, send_buf, sizeof(send_buf)/sizeof(send_buf[0]), 0);
        
        if (send_size == -1) {
            printf("送信エラー\n");
            return -1;
        }        
    }
    */  
    /** normal terminaion on stop command **/
    return 0;
}

int8_t SendData(int16_t sock, Sensor_data h){
    int8_t errCode = 0;
    
    /** Clear send_buf **/
    memset(send_buf, 0, 1024);
    
    /** Prepare send data with format **/
    sprintf(send_buf, "mData,%s,%.1f\r\n%s,%.1f\r\n%s,%.1f\r\n", h.gasName, h.gas, h.humid, h.humidity, h.temp, h.temperature);        
    
    errCode = send(sock, send_buf, sizeof(send_buf)/sizeof(send_buf[0]), 0);    
    if (errCode == -1) return -1;
            
    /** normal terminaion on stop command **/
    return 0;
}
int8_t SendStatus(int16_t sock, char reply[]){
    int16_t send_size = 0;

    memset(send_buf, 0, 1024);
    strcat(send_buf, reply);
    send_size = send(sock, send_buf, sizeof(send_buf)/sizeof(send_buf[0]), 0);

    if (send_size == -1) {
        printf("送信エラー\n");
        return -1;    
    }
    // normal terminaion on stop command
    return 0;
}

int8_t FinishProcess(int16_t sock){
    int8_t result = 0;
        
    /** Clear send_buf **/
    memset(send_buf, 0, 1024);
    strcat(send_buf, "Disconnect\n");
    
    result = send(sock, send_buf, sizeof(send_buf)/sizeof(send_buf[0]), 0);
    if (result == -1) printf("Failed to send ""Finish command""\n");
    else{
        // Stop Sensor sensing;
        StopSensing();
        /*
        if(StopSensing() == 0){
            close(sock);
        }
        */
    }
    return result;
}

int8_t SendDM(int16_t sock){
    int16_t send_size = 0;
        
    memset(send_buf, 0, 1024);
    strcat((char *)send_buf, (char *)send_DM);
    strcat((char *)send_buf, (char *)device_marking);

    send_size = send(sock, (unsigned char *)send_buf, sizeof(send_buf)/sizeof(send_buf[0]), 0);
    
    if (send_size == -1) {
        //printf("送信エラー\n");
        return -1;
    }
    // normal terminaion on Device Marking read
    return 0;
}

int8_t OtherCMD(int16_t sock){
    int16_t send_size = 0;
    //int16_t error = 0;
    
    // "finish"以外の場合はクライアントとの接続を継続
    send_size = send(sock, send_greeting, sizeof(send_greeting)/sizeof(send_greeting[0]), 0);
    if (send_size == -1) {
        printf("send error\n");
        return -1;
    }
    // normal termination on other command
    return 0;
}
