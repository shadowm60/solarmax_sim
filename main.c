#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include "solarmax.h"

#define SERIAL_PORT_DEV "/tmp/dev/ttyS20"

static int serial_port;
int loop;
struct termios tty;


void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("received SIGINT\n");
    loop = 0;
    }
}

/* https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/ */

void writeSerialPort(char *data, size_t len) {
    if ((data != NULL) && (serial_port >= 0 )) {
        write(serial_port,data,len);
    }
}

size_t readSerialPort(char *data) {

    size_t data_len;

    if (serial_port >= 0) {
        data_len = read(serial_port, data, 50);
    }
    return data_len;
}

void openSerialPort(void){
    serial_port = open(SERIAL_PORT_DEV, O_RDWR | O_NOCTTY);

    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return;
    }

    // Flush away any bytes previously read or written.
    int result = tcflush(serial_port, TCIOFLUSH);
    if (result)
    {
        perror("tcflush failed");  // just a warning, not a fatal error
    }

    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        close(serial_port);
        return;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    //tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    //tty.c_lflag &= ~ICANON; //disable canonical, that means we will not process data only when newline is detected.
    //tty.c_lflag &= ~ECHO; // Disable echo
    //tty.c_lflag &= ~ECHOE; // Disable erasure
    //tty.c_lflag &= ~ECHONL; // Disable new-line echo
    //tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    //tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    //tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    //tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    //tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    //tty.c_cc[VTIME] = 0; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    //tty.c_cc[VMIN] = 0;

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    if(tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        close(serial_port);
    }

    printf("Serial port opened %s baudrate: B9600 fd: %d\n",SERIAL_PORT_DEV,serial_port);


}


int main()
{
    char read_buf[200];
    size_t len;
    signal(SIGINT, sig_handler);
    //signal(SIGUSR1, sig_handler);
    //signal(SIGKILL, sig_handler);
    //signal(SIGSTOP, sig_handler);

    printf("Opening serial port: %s\n", SERIAL_PORT_DEV);
    openSerialPort();

    if (serial_port >= 0){
        writeSerialPort("TEST_WRITE\n\r",12);

        loop = 1;
        while (loop) {
            memset(&read_buf, '\0', sizeof(read_buf));
            len = readSerialPort(read_buf);
            if (len != 0) {
                //feed data to solarmax
                //printf("Read from serial: %s - [%d]\n",read_buf,len);
                if (0 != solarmax_serial_input(read_buf,len)) {
                    //we just received a complete message, call once again to process it
                    solarmax_serial_input(read_buf,len);
                }
            }
        }
        close(serial_port);
    }
    return 0;
}
