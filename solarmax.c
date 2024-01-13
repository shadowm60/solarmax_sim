#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "solarmax.h"

typedef struct {
    char name[50];
    char key[4];
    char unit[5];
}solarmax_cmd_type;


solarmax_cmd_type solarmax_cmd[] =
{
{.name = "AC Power",                .key = "PAC", .unit = "mW" },
{.name = "Operating hours",         .key = "KHR", .unit = "h" },
{.name = "Date",                    .key = "DATE", .unit = "" },
{.name = "Year",                    .key = "DYR", .unit = "" },
{.name = "Month",                   .key = "DMT", .unit = "" },
{.name = "Day",                     .key = "DDY", .unit = "" },
{.name = "Energy this year",        .key = "KYR", .unit = "kWh" },
{.name = "Energy this month",       .key = "KMT", .unit = "kWh" },
{.name = "Energy today",            .key = "KDY", .unit = "kWh" },
{.name = "Energy Total",            .key = "KT0", .unit = "kWh" },
#ifdef PULSE_SUPPORT
{.name = "Pulse counter 1 year",    .key = "I1Y", .unit = "" },
{.name = "Pulse counter 1 power",   .key = "I1P", .unit = "" },
{.name = "Pulse counter 1 scaling", .key = "I1S", .unit = "" },
{.name = "Pulse counter 1 day",     .key = "I1D", .unit = "" },
{.name = "Pulse counter 1 Total",   .key = "I1T", .unit = "" },
{.name = "Pulse counter 2 year",    .key = "I2Y", .unit = "" },
{.name = "Pulse counter 2 power",   .key = "I2P", .unit = "" },
{.name = "Pulse counter 2 scaling", .key = "I2S", .unit = "" },
{.name = "Pulse counter 2 day",     .key = "I2D", .unit = "" },
{.name = "Pulse counter 2 Total",   .key = "I2T", .unit = "" },
#endif
{.name = "Installed capacity",      .key = "PIN", .unit = "" },
{.name = "Network period duration", .key = "TNP", .unit = "us" },
{.name = "Network address",         .key = "ADR", .unit = "" },
{.name = "Relative power",          .key = "PRL", .unit = "" },
{.name = "Software Version",        .key = "SWV", .unit = "" },
{.name = "Solar energy year",       .key = "RYR", .unit = "" },
{.name = "Solar energy day",        .key = "RDY", .unit = "" },
{.name = "Solar energy total",      .key = "RT0", .unit = "" },
{.name = "Solar radiation",         .key = "RAD", .unit = "" },
{.name = "Voltage DC",              .key = "UDC", .unit = "" },
{.name = "Voltage Phase 1",         .key = "UL1", .unit = "" },
{.name = "Voltage Phase 2",         .key = "UL2", .unit = "" },
{.name = "Voltage Phase 3",         .key = "UL3", .unit = "" },
{.name = "Current DC",              .key = "IDC", .unit = "" },
{.name = "Current Phase 1",         .key = "IL1", .unit = "" },
{.name = "Current Phase 2",         .key = "IL2", .unit = "" },
{.name = "Current Phase 3",         .key = "IL3", .unit = "" },
{.name = "Tempereture section 1",   .key = "TKK", .unit = "" },
{.name = "Temperature section 2",   .key = "TK2", .unit = "" },
{.name = "Tenoerature section 3",   .key = "TK3", .unit = "" },
{.name = "Solar Cells",             .key = "TSZ", .unit = "" },
{.name = "Typ",                     .key = "TYP", .unit = "" },
{.name = "Date",                    .key = "TIME", .unit = "" },
{.name = "Minutes",                 .key = "TMI", .unit = "" },
{.name = "Hour",                    .key = "THR", .unit = "" },
};

typedef enum {
    WAITING_FOR_START = 0,
    RECEIVING_CMD,
    PROCESS_CMD
} recv_state;

typedef enum {
    REQUEST = 0,
    RESPONSE
} cmd_t;

char command_buffer[MAX_CMD_BUF_SIZE];
int cmd_len = 0;
recv_state cmd_buf_state = WAITING_FOR_START;


void solarmax_cmd_process(void);


int solarmax_serial_input(char *data, size_t len) {
char *ptr;
int retVal = 0;

    switch(cmd_buf_state)
    {
        case WAITING_FOR_START:
            //check if we have a start of command character
            ptr = strstr(data,"{");
            if (NULL != ptr) {
                memset(&command_buffer, '\0', sizeof(command_buffer));
                int skip = ptr - data;
                strncpy(command_buffer,ptr,len-skip);
                cmd_len = len-skip;
                cmd_buf_state = RECEIVING_CMD;
                printf("INPUT: |%s|, copy %d bytes\n",data,len-skip);
            }
            break;
        case RECEIVING_CMD :
            ptr = strstr(data,"}");
            if (NULL == ptr) {
                //we just received a part of the cmd, so we just save it
                strncpy(&command_buffer[cmd_len],data,len);
                cmd_len += len;
            } else {
                int skip = ptr - data;
                skip+=1; //also include {
                strncpy(&command_buffer[cmd_len],data,skip);
                printf("INPUT: |%s|, copy %d bytes\n",data,skip);
                cmd_len += skip;
                cmd_buf_state = PROCESS_CMD;
                retVal = 1;
            }
            break;
        case PROCESS_CMD:
                printf("CMD: %s\n",command_buffer);
                solarmax_cmd_process();
                cmd_buf_state = WAITING_FOR_START;
            break;
        default :
            printf("Error State, reseting!\n");
            memset(&command_buffer, '\0', sizeof(command_buffer));
            cmd_len = 0;
            cmd_buf_state = WAITING_FOR_START;
            break;

    }

    return retVal;
}

uint32_t solarmax_cmd_crc(char *data, size_t len) {

uint32_t crc = 0;
    for (int i=0; i< len; i ++) {
        crc += *data;
        data++;
    }
    return crc;
}

uint32_t strtohex(char *ptr, uint8_t len){

  uint32_t retVal = 0;
  if (ptr != NULL){
    for (uint8_t i=0; i<len; i++){
      retVal <<= 4;
      if ((*ptr >= 48) && (*ptr <= 57)){
        retVal |= *ptr - 48;
        ptr++;
      } else if ((*ptr >= 65) && (*ptr <= 70)){
        retVal |= *ptr - 55;
        ptr++;
      } else if ((*ptr >= 97) && (*ptr <= 102)){
        retVal |= *ptr - 87;
        ptr++;
      }
    }
  }

  return retVal;
}

#define CMD_PROCESS_DEBUG
//char command_buffer[MAX_CMD_BUF_SIZE];
//int cmd_len = 0;
void solarmax_process_msg(char *data) {
    uint16_t src_addr;
    uint16_t dest_addr;
    uint16_t length;
    uint16_t port;
    char *ptr;
    char *tptr;
    char cmd[5];
    char val[5];
    cmd_t cmd_type = REQUEST;

#ifdef CMD_PROCESS_DEBUG
    printf("solarmax_process_msg(%s)\n",data);
#endif
    src_addr = strtohex(data,2);
    dest_addr = strtohex(data+3,2);
    length = strtohex(data+6,2);
    //lets find payload start
    //port = strtohex(data+9,2);
    ptr = strstr(data,"|");
    port = strtohex(ptr,2);

#ifdef CMD_PROCESS_DEBUG
    printf("Src-Addr: %02x\n",src_addr);
    printf("Dest-Addr: %02x\n",dest_addr);
    printf("Length: %02x\n",length);
    printf("Port: %02x\n",port);
#endif

    //skip port and :
    ptr+=4;

    // ABC=33;DEF=00;GHI=12|CRC
    //now search for cmd identifier
    char eodata = 1;
    while ((*ptr != '|') && (eodata)) {
        memset(cmd,0,sizeof(cmd));
        memset(val,0,sizeof(val));
        printf("while: %s\n", ptr);
        //get the command, this can end with = or ; dependeing if it is req or response
        int itr = 0;
        while ((*ptr != '=') && (*ptr != ';') && (*ptr != '|')) {
            cmd[itr] = *ptr;
            ptr++;
            itr++;
        }

        if (*ptr == '=') {
            cmd_type = RESPONSE;
#ifdef  CMD_PROCESS_DEBUG
            printf("CMD RESPONSE: %s\n",cmd);
#endif               
        } else if (*ptr == ';') {
            cmd_type = REQUEST;
#ifdef  CMD_PROCESS_DEBUG
            printf("CMD REQUEST: %s\n",cmd);
#endif                               
        } else if (*ptr == '|') {
            eodata = 0;        
        }

        if (cmd_type == RESPONSE) {
            //consume data
            itr = 0;
            while ((*ptr != ';') && (*ptr != '|')) {
                if (*ptr == '=') {
                    //skip =
                    *ptr++;
                }
                val[itr] = *ptr;
                ptr++;
                itr++;
            }
            if (*ptr == '|') {
                eodata = 0;
            }
        } else {
            //prepare response
        }
        printf("CMD_TYPE: %d, cmd: %s val: %s\n",cmd_type,cmd,val);
    }
#ifdef TEMP_WHILE
    while (*(ptr+1) == ';') {
        tptr = strstr(ptr,"=");
        strncpy(cmd,ptr,(tptr-ptr));

        printf("Cmd: %s",cmd);
        printf("Value: %s,");

    }
#endif
}




void solarmax_cmd_process(void) {

    char *ptr_crc_start, *ptr_crc_end;
    char *ptr;

    ptr_crc_start = strstr(command_buffer, "{");
    ptr_crc_end = strstr(command_buffer, "}");

    //move the ptrs where we really need them
    ptr_crc_start++;
    ptr_crc_end -= 4;

    int len = ptr_crc_end - ptr_crc_start;
#ifdef CMD_PROCESS_DEBUG
    printf("Processing cmd string: ");
    ptr = ptr_crc_start;
    for (int i = 0; i < len; i++) {
        printf("%c",*ptr);
        ptr++;
    }
    printf("\n\n");
#endif

    uint32_t crc_calc = solarmax_cmd_crc(ptr_crc_start, len);
    uint32_t crc_recv = strtohex(ptr_crc_end, 4);
    printf("Crc: %04x  - %04x \n", crc_calc, crc_recv);

    if (crc_calc == crc_recv) {
        solarmax_process_msg(ptr_crc_start);
    } else {
        printf("Dropping msg: %s\n",command_buffer);
    }


}
