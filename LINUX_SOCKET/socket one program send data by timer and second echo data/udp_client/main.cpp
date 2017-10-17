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


using namespace std;

#define BUFLEN 70000

#define CLIENT_IP "192.168.103.14"
//#define CLIENT_IP "127.0.0.1"

#define FIRST_LISTEN_PORT 1234
#define FIRST_SEND_PORT 6321



void die(const char *s);
static int make_socket_non_blocking (int sfd);

int main(void)
{

    const int sendBufferSize = 65507;
    char sendBuffer[sendBufferSize];

    for(int i = 0; i < sendBufferSize; i++){
        sendBuffer[i] = i;
    }

    const int lenght_ofAdressStruct = sizeof(sockaddr_in);


    // SOCKET ************************************************
    struct sockaddr_in listenAddress, sendAddress;

    int socket_fd, recv_len;

    char socket_rx_buffer[BUFLEN];

    if ((socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("socket");
    }else{
        cout << "Sucessfull created socket First" << endl;
    }

    memset((char *) &sendAddress, 0, sizeof(sendAddress));
    sendAddress.sin_family = AF_INET;
    sendAddress.sin_port = htons(FIRST_SEND_PORT);
    if (inet_aton(CLIENT_IP , &sendAddress.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    memset((char *) &listenAddress, 0, sizeof(listenAddress));
    listenAddress.sin_family = AF_INET;
    listenAddress.sin_port = htons(FIRST_LISTEN_PORT);
    listenAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if(make_socket_non_blocking (socket_fd) == -1){
        abort ();
    }

    if( bind(socket_fd , (struct sockaddr*)&listenAddress, sizeof(listenAddress) ) == -1){
        die("bind");
    }




    //EPOLL ************************************************
    int epollfd, number_of_file_descriptors;

#define MAX_EVENTS 10
    struct epoll_event events[MAX_EVENTS];

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event first_socket_ev;
    first_socket_ev.events = EPOLLIN;
    first_socket_ev.data.fd = socket_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socket_fd, &first_socket_ev) == -1) {
        perror("Error adding first socket to epoll");
        exit(EXIT_FAILURE);
    }else{
        cout << "Sucessfull adding first socket to epoll" << endl;
    }




    static int ret_val = 0;
    static int idx = 0;
    //*******************************************************
    for(;;) {
        number_of_file_descriptors = epoll_wait(epollfd, events, MAX_EVENTS, 5);
        //cout << idx++ << " epoll_wait = " << number_of_file_descriptors << endl;
        if (number_of_file_descriptors == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }else if(number_of_file_descriptors == 0){

            //cout << "Epoll Timeout" << endl;

            if (sendto(socket_fd, sendBuffer, sendBufferSize, 0, (struct sockaddr*) &sendAddress, lenght_ofAdressStruct) == -1){
                die("sendto()");
            }
            continue;

        }else if(number_of_file_descriptors > 0){

            for (int n = 0; n < number_of_file_descriptors; ++n) {


                if (events[n].data.fd == socket_fd) {

                    //cout << "Parse first_socket event" << endl;

                    recv_len = recvfrom(socket_fd, socket_rx_buffer, BUFLEN, 0, NULL, NULL);

                    //cout << "   Read data from socket_fd = " << socket_fd << " with resul = " << recv_len << endl;

                    if(recv_len == -1){
                        //cout << "Read Error " << endl;
                        int errsv = errno;
                        if(errsv == EAGAIN){
                            //cout << "EAGAIN" << endl;
                        }else if(errsv == EWOULDBLOCK){
                            //cout << "EWOULDBLOCK" << endl;
                        }else{
                            die("recvfrom()");
                        }
                    }else{
                        //cout << "Received data from socket" << endl;
                        //printf("Received packet from %s:%d\n", inet_ntoa(second_si_other.sin_addr), ntohs(second_si_other.sin_port));

                        //ret_val = write(first_serial_fd, first_socket_rx_buffer, first_recv_len);

                    }
                    continue;
                }

            }

        }
    }
    close(socket_fd);



    return 0;
}


void die(const char *s)
{
    perror(s);
    exit(1);
}

static int make_socket_non_blocking (int sfd)
{
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }

    return 0;
}


