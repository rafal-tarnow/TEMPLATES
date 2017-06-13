#include <termios.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "./serialPort/serialport.hpp"


using namespace std;



int serial_port_fd;
ssize_t rozmiarOdebranychDanych = 0;
unsigned char rx_buffer[256];




unsigned char tx_frame[10] = { 0xFA, 0xFF, 0x00, 0x00, 0x01};


int main(int argc,char **argv) {


    initConnection(serial_port_fd, "/dev/ttyCTI6", B115200,NO_PARITY,TWO_STOP_BITS);

    while(1){
        rozmiarOdebranychDanych = read( serial_port_fd, &rx_buffer, 256);

        if(rozmiarOdebranychDanych > 0){
            cout << "Frame size = " << rozmiarOdebranychDanych  << endl;
            for(int i =0; i < rozmiarOdebranychDanych; i++){
                cout << (int)rx_buffer[i];
            }
            cout << endl;
            flush(cout);
        }

    }

}
