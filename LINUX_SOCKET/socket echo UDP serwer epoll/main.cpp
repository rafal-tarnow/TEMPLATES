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

#define BUFLEN 512  //Max length of buffer
#define PORT 1234   //The port on which to listen for incoming data

void die(char *s)
{
    perror(s);
    exit(1);
}

static int make_socket_non_blocking (int sfd);

int main(void)
{

    //SOCKET ************************************************

    struct sockaddr_in si_me, si_other;

    int listen_socket, i, s, recv_len;
    socklen_t slen = sizeof(si_other);

    char buf[BUFLEN];

    //create a UDP socket
    if ((listen_socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if( bind(listen_socket , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    s = make_socket_non_blocking (listen_socket);
    if (s == -1)
        abort ();

    //EPOLL ************************************************
#define MAX_EVENTS 10
    struct epoll_event ev, events[MAX_EVENTS];
    int epollfd, number_of_file_descriptors;

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN; //level
    //ev.events = EPOLLET;// (edge-triggered)
    //ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_socket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_socket, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    static int idx = 0;
    //*******************************************************
    for(;;) {
        cout << "for(;;;) = " << idx++ << endl;
        number_of_file_descriptors = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        cout << "number_of_file_descriptors = " << number_of_file_descriptors << endl;

        if (number_of_file_descriptors == -1) {
            cout << "epoll_wait error" << endl;
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }else if(number_of_file_descriptors == 0){
            cout << "epoll_wait timeout" << endl;
        }else if(number_of_file_descriptors > 0){

            for (int n = 0; n < number_of_file_descriptors; ++n) {
                if (events[n].data.fd == listen_socket) {
                    //process_socket_event();


                    cout << "listen_sock file descriptor event" << endl;
                    //try to receive some data, this is a blocking call
                    recv_len = recvfrom(listen_socket, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);

                    if(recv_len == -1)
                    {
                        cout << "Read Error " << endl;
                        int errsv = errno;
                        if(errsv == EAGAIN){
                            cout << "EAGAIN" << endl;
                        }else if(errsv == EWOULDBLOCK){
                            cout << "EWOULDBLOCK" << endl;
                        }else{
                            die("recvfrom()");
                        }
                    }else{
                        cout << "Read OK" << endl;

                        //print details of the client/peer and the data received
                        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                        printf("Data: %s\n" , buf);

                        //now reply the client with the same data
                        if (sendto(listen_socket, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
                        {
                            //die("sendto()");
                        }
                    }

                }
            }
        }
    }

    close(listen_socket);
    return 0;
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

