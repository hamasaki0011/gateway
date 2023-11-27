#include <stdio.h>

uint8_t StartProcess(int32_t sock, uint8_t status){
            // If status is NOT "START"
        if(status == STOP){
            if(StartMeasure(sock) == 0){
                char *msg = "Start to measure...\r\n";
                SendStatus(sock, msg);
                return status = START;
            }else{
                char *msg = "Failed to execute start command for sensor\r\nPlease try again\r\n";
                SendStatus(sock, msg);
                sensorStatus = FAILED_START_SENSOR;
                return status = ERROR;
            }
        }else{
            char *msg = "Already started!\r\n";
            SendStatus(sock, msg);
            return status = START; // overwrite the status
        }
}
