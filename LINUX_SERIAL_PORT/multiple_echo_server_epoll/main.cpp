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

//**** CONFIG ****

#define BUFLEN 512
#define DEBUG_MAIN_LOOP
#define LICZBA_PORTOW 6

int main(void)
{
    const char * tablica_nazw_portow[LICZBA_PORTOW];

    tablica_nazw_portow[0] = "/dev/tnt0";
    tablica_nazw_portow[1] = "/dev/tnt2";
    tablica_nazw_portow[2] = "/dev/tnt4";
    tablica_nazw_portow[3] = "/dev/tnt6";
    tablica_nazw_portow[4] = "/dev/tnt8";
    tablica_nazw_portow[5] = "/dev/tnt10";


    cout << "***** SERIAL PORT ECHO *****" << endl << endl;

    //SERIAL PORT ******************************************
    int serial_port_fd[LICZBA_PORTOW];
    ssize_t rozmiarOdebranychDanych = 0;


    unsigned char rx_buffer[256];

     int ret_val = 0;

    for(int i =0; i < LICZBA_PORTOW; i++){
        ret_val = initConnection(serial_port_fd[i], tablica_nazw_portow[i] , B115200,NO_PARITY,ONE_STOP_BIT);
        if(ret_val == 1){
            cout << "Sucesfull open port " << tablica_nazw_portow[i] << ", deskryptor portu = " << serial_port_fd[i] << endl;
        }else if(ret_val == 0){
             cout << "Failed open port " << tablica_nazw_portow[i]  << endl;
        }
    }


    //EPOLL ************************************************
#define MAX_EVENTS 10
    struct epoll_event serial_ev[LICZBA_PORTOW], events[MAX_EVENTS];
    int epollfd, number_of_file_descriptors;

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }



    for(int i = 0; i < LICZBA_PORTOW; i++){

        serial_ev[i].events = EPOLLIN; //level
        //ev.events = EPOLLET;// (edge-triggered)
        //ev.events = EPOLLIN | EPOLLET;
        serial_ev[i].data.fd = serial_port_fd[i];
        ret_val = epoll_ctl(epollfd, EPOLL_CTL_ADD, serial_port_fd[i], &(serial_ev[i]));
        if (ret_val == -1) {
            perror("epoll_ctl: listen_sock");
            exit(EXIT_FAILURE);
        }else if(ret_val == 0){
            cout << "Sucessfull added " << i << " serial to Epoll" << endl;
        }
    }

    static int liczba_wszystkich_eventow = 0;
    //*******************************************************
    for(;;) {
        number_of_file_descriptors = epoll_wait(epollfd, events, MAX_EVENTS, -1);
#ifdef DEBUG_MAIN_LOOP
        cout << "Odebrano " << number_of_file_descriptors <<" event z epool numer "<< liczba_wszystkich_eventow++ << endl;
#endif
        if (number_of_file_descriptors == -1) {
#ifdef DEBUG_MAIN_LOOP
            cout << "epoll_wait error" << endl;
            perror("epoll_wait");
#endif
            exit(EXIT_FAILURE);
        }else if(number_of_file_descriptors == 0){
#ifdef DEBUG_MAIN_LOOP
            cout << "epoll_wait timeout" << endl;
#endif
        }else if(number_of_file_descriptors > 0){

            for (int n = 0; n < number_of_file_descriptors; ++n) {
                for(int i = 0; i < LICZBA_PORTOW; i++){
                    if (events[n].data.fd == serial_port_fd[i]) {
#ifdef DEBUG_MAIN_LOOP
                        cout << "Przetwarzanie epoll eventu z deskryptora = " << serial_port_fd[i] << endl;
#endif
                        rozmiarOdebranychDanych = read( serial_port_fd[i], &rx_buffer, 256);
                        write(serial_port_fd[i], rx_buffer, rozmiarOdebranychDanych);
                    }
                }
            }
        }
    }
    return 0;
}



