#ifndef _SOLARMAX_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // write(), read(), close()


//how big do we want to keep this buffer, it needs to be as big as the longest command/response
#define MAX_CMD_BUF_SIZE    512

int solarmax_serial_input(char *data, size_t len);
#endif
