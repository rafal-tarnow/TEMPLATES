#include "serialport.hpp"



bool initConnection(int & fileDescriptor, string port_name, speed_t baudrate, ParityMode parityMode,StopBits stopBits ) {
    termios serialPortSettings;

    fileDescriptor = open( port_name.c_str(), O_RDWR | O_NONBLOCK);

    if( fileDescriptor != ( -1 ) ){
        cout << "Serial port opened succesfull" << endl;

        //get serial port settings
        tcgetattr( fileDescriptor, &serialPortSettings );
        //set baudrate
        cfsetspeed( &serialPortSettings, baudrate );
        cfmakeraw( &serialPortSettings );
        //set parity
        switch(parityMode){
        case NO_PARITY:
            serialPortSettings.c_cflag &= ~PARENB;
            break;
        case ODD_PARITY:
            serialPortSettings.c_cflag |=  PARENB;
            serialPortSettings.c_cflag |= PARODD;
            break;
        case EVEN_PARITY:
            serialPortSettings.c_cflag |=  PARENB;
            serialPortSettings.c_cflag &= ~PARODD;
            break;
        }
        //set stop bits
        switch(stopBits){
        case TWO_STOP_BITS:
            serialPortSettings.c_cflag |= CSTOPB;
            break;
        case ONE_STOP_BIT:
            serialPortSettings.c_cflag &= ~CSTOPB;
            break;
        }
        //set serial port settings
        tcsetattr( fileDescriptor, TCSANOW, &serialPortSettings );
        tcflush( fileDescriptor, TCIOFLUSH );
        return true;
    }else if(fileDescriptor == -1) {
        cout << "blad otwarcia portu szeregowego" << endl;
        flush( cout );
        return false;
    }
}



