#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "solarmax.h"

typedef struct {
    char name[50];
    char key[4];
    char unit[5];
    char isData;
    uint32_t value;
}solarmax_cmd_type;


solarmax_cmd_type solarmax_cmd[] =
{
{.name = "AC Power",                .key = "PAC", .unit = "mW"      , .isData = 0,  .value = 0 },
{.name = "Operating hours",         .key = "KHR", .unit = "h"       , .isData = 0,  .value = 0 },
{.name = "Date",                    .key = "DATE", .unit = ""       , .isData = 0,  .value = 0 },
{.name = "Year",                    .key = "DYR", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Month",                   .key = "DMT", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Day",                     .key = "DDY", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Energy this year",        .key = "KYR", .unit = "kWh"     , .isData = 0,  .value = 0 },
{.name = "Energy this month",       .key = "KMT", .unit = "kWh"     , .isData = 0,  .value = 0 },
{.name = "Energy today",            .key = "KDY", .unit = "kWh"     , .isData = 0,  .value = 0 },
{.name = "Energy Total",            .key = "KT0", .unit = "kWh"     , .isData = 1,  .value = 0xA02 },
#ifdef PULSE_SUPPORT
{.name = "Pulse counter 1 year",    .key = "I1Y", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 1 power",   .key = "I1P", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 1 scaling", .key = "I1S", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 1 day",     .key = "I1D", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 1 Total",   .key = "I1T", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 2 year",    .key = "I2Y", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 2 power",   .key = "I2P", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 2 scaling", .key = "I2S", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 2 day",     .key = "I2D", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Pulse counter 2 Total",   .key = "I2T", .unit = ""        , .isData = 0,  .value = 0 },
#endif
{.name = "Installed capacity",      .key = "PIN", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Network period duration", .key = "TNP", .unit = "us"      , .isData = 0,  .value = 0 },
{.name = "Network address",         .key = "ADR", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Relative power",          .key = "PRL", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Software Version",        .key = "SWV", .unit = ""        , .isData = 1,  .value = 321 },
{.name = "Solar energy year",       .key = "RYR", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Solar energy day",        .key = "RDY", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Solar energy total",      .key = "RT0", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Solar radiation",         .key = "RAD", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Voltage DC",              .key = "UDC", .unit = ""        , .isData = 1,  .value = 0xA23 },
{.name = "Voltage Phase 1",         .key = "UL1", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Voltage Phase 2",         .key = "UL2", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Voltage Phase 3",         .key = "UL3", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Current DC",              .key = "IDC", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Current Phase 1",         .key = "IL1", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Current Phase 2",         .key = "IL2", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Current Phase 3",         .key = "IL3", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Tempereture section 1",   .key = "TKK", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Temperature section 2",   .key = "TK2", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Tenoerature section 3",   .key = "TK3", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Solar Cells",             .key = "TSZ", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Typ",                     .key = "TYP", .unit = ""        , .isData = 1,  .value = 123 },
{.name = "Date",                    .key = "TIME", .unit = ""       , .isData = 0,  .value = 0 },
{.name = "Minutes",                 .key = "TMI", .unit = ""        , .isData = 0,  .value = 0 },
{.name = "Hour",                    .key = "THR", .unit = ""        , .isData = 0,  .value = 0 },
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

#define CMD_ARRAY_SIZE      (sizeof(solarmax_cmd) / sizeof(solarmax_cmd_type))

char command_buffer[MAX_CMD_BUF_SIZE];
char response_buffer[MAX_RESP_BUF_SIZE];
int cmd_len = 0;
recv_state cmd_buf_state = WAITING_FOR_START;


void solarmax_cmd_process(void);
void solarmax_store_value(char *cmd, uint32_t value);
uint32_t solarmax_get_value(char *cmd);

#define CMD_PROCESS_DEBUG

void solarmax_store_value(char *cmd, uint32_t value) {
    int i;

    for (i=0; i < CMD_ARRAY_SIZE; i++) {
        if (0 == strcmp(solarmax_cmd[i].key,cmd)) {
            solarmax_cmd[i].isData = 1;
            solarmax_cmd[i].value = value;
            break;
        }
    }
}

uint32_t solarmax_get_value(char *cmd) {

    int i;
    uint32_t retVal = 0;
    for (i=0; i< CMD_ARRAY_SIZE; i++) {
        if (0 == strcmp(solarmax_cmd[i].key,cmd)) {
            if (solarmax_cmd[i].isData)
                retVal = solarmax_cmd[i].value;
        }
    }

    return retVal;
}


uint32_t solarmax_serial_input(char *data, size_t len) {
char *ptr;
uint32_t retVal = 0;

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

uint8_t hextostr(char *dest, uint32_t val) {
    uint8_t i,len = 0;
    if (val <= 0xFF) {
        len = 2;
    } else if (val <= 0xFFF) {
        len = 3;
    } else if (val <= 0xFFFF) {
        len = 4;
    }

    for (i=0; i< len; i++) {
        if ((val&0x0000000f) < 0x0000000a) {
            dest[i] = ((val&0x0000000f) + 48 );
        } else {
            dest[i] = ((val&0x0000000f) + 55 );
        }
        val >>= 4;
    }
    return len;
}

void solarmax_process_msg(char *data) {
    uint16_t src_addr;
    uint16_t dest_addr;
    uint16_t length;
    uint16_t port;
    uint16_t resp_idx = 13;
    char *ptr;
    char *tptr;
    char cmd[5];
    char val[5];
    cmd_t cmd_type = REQUEST;

#ifdef CMD_PROCESS_DEBUG
    printf("solarmax_process_msg(%s)\n",data);
#endif
    memset(response_buffer,0,sizeof(response_buffer));
    src_addr = strtohex(data,2);
    dest_addr = strtohex(data+3,2);
    length = strtohex(data+6,2);
    //lets find payload start
    //port = strtohex(data+9,2);
    ptr = strstr(data,"|");
    ptr++;
    port = strtohex(ptr,2);

#ifdef CMD_PROCESS_DEBUG
    printf("Src-Addr: %02x\n",src_addr);
    printf("Dest-Addr: %02x\n",dest_addr);
    printf("Length: %02x\n",length);
    printf("Port: %02x\n",port);
#endif

    //skip port and :
    ptr+=3;

    // ABC=33;DEF=00;GHI=12|CRC
    //now search for cmd identifier
    char eodata = 1;
    while ((*ptr != '|') && (eodata)) {
        memset(cmd,0,sizeof(cmd));
        memset(val,0,sizeof(val));
        printf("while: %s\n", ptr);
        //get the command, this can end with = or ; dependeing if it is req or response
        int itr = 0;
        if (*ptr == ';') {
            //if we have multiple requests, we need to jump over separators
            *ptr++;
        }
        while ((*ptr != '=') && (*ptr != ';') && (*ptr != '|')) {
            cmd[itr] = *ptr;
            ptr++;
            itr++;
        }

        if (*ptr == '=') {
            cmd_type = RESPONSE;
        } else if (*ptr == ';') {
            cmd_type = REQUEST;
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
            solarmax_store_value(cmd,strtohex(val, sizeof(val)));
            if (*ptr == '|') {
                eodata = 0;
            }
        } else {
            //prepare response
            uint32_t val;
            uint8_t len;
            val = solarmax_get_value(cmd);
            len = strlen(cmd);
            strncpy(&response_buffer[resp_idx],cmd,len);
            resp_idx += len;
            response_buffer[resp_idx] = '=';
            resp_idx++;
            len = hextostr(&response_buffer[resp_idx], val);
            resp_idx += len;
            if (*ptr == '|') {
                eodata = 0;
            } else {
                response_buffer[resp_idx] = ';';
                resp_idx++;
                *ptr++;
            }

        }
        printf("CMD_TYPE: %d, cmd: %s val: %s\n",cmd_type,cmd,val);
    }

    if (cmd_type == REQUEST) {
        char tbf[14];
        sprintf(tbf,"{%02X:%02X:%02X|%02X:",dest_addr,src_addr,resp_idx+6,port);
        strncpy(response_buffer,tbf,13);
        response_buffer[resp_idx] = '|';
        resp_idx++;
        uint32_t crc_calc = solarmax_cmd_crc(&response_buffer[1], resp_idx);
        printf("%d index, %04x crc\n",resp_idx,crc_calc);
        memset(tbf,0,sizeof(tbf));
        sprintf(tbf,"%04X}",crc_calc);
        strncpy(&response_buffer[resp_idx],tbf,5);

        for (int i = 0; i<resp_idx+5; i++ ) {
            printf("%c",response_buffer[i]);
            if (response_buffer[i] == 0) printf(".");
        }
        printf("\n");
    }
    //if we had a req we need to assamble to response buffer
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
