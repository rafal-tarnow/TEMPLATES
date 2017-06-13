#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP

#include <string>
#include <termios.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

using namespace std;

typedef enum{
    NO_PARITY,
    ODD_PARITY,
    EVEN_PARITY
} ParityMode;

typedef enum{
    ONE_STOP_BIT,
    TWO_STOP_BITS
}StopBits;

bool initConnection(int & fileDescriptor, string port_name, speed_t baudrate, ParityMode parityMode,StopBits stopBits );

#endif // SERIALPORT_HPP

