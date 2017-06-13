/*
    Simple udp server
    Silver Moon (m00n.silv3r@gmail.com)
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include "./serialPort/serialport.hpp"

using namespace std;

#define BUFLEN 512

#define SERIAL_PORT "/dev/ttyCTI1"

int main(void)
{
    cout << "***** SERIAL PORT ECHO " << SERIAL_PORT << "*****" << endl << endl;

    //SERIAL PORT ******************************************
    int serial_port_fd;
    ssize_t rozmiarOdebranychDanych = 0;
    unsigned char rx_buffer[256];
    initConnection(serial_port_fd, SERIAL_PORT , B115200,NO_PARITY,ONE_STOP_BIT);


    //EPOLL ************************************************
#define MAX_EVENTS 10
    struct epoll_event serial_ev, events[MAX_EVENTS];
    int epollfd, number_of_file_descriptors;

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    serial_ev.events = EPOLLIN; //level
    //ev.events = EPOLLET;// (edge-triggered)
    //ev.events = EPOLLIN | EPOLLET;
    serial_ev.data.fd = serial_port_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serial_port_fd, &serial_ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    cout << "Sucessfull added serial to Epoll" << endl;

    //*******************************************************
    for(;;) {
        number_of_file_descriptors = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        if (number_of_file_descriptors == -1) {
            cout << "epoll_wait error" << endl;
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }else if(number_of_file_descriptors == 0){
            cout << "epoll_wait timeout" << endl;
        }else if(number_of_file_descriptors > 0){

            for (int n = 0; n < number_of_file_descriptors; ++n) {
                if (events[n].data.fd == serial_port_fd) {
                    rozmiarOdebranychDanych = read( serial_port_fd, &rx_buffer, 256);
                    write(serial_port_fd, rx_buffer, rozmiarOdebranychDanych);
                }
            }
        }
    }
    return 0;
}



