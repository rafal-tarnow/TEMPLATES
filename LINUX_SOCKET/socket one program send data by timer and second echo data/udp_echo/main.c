#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>


#define BUFLEN 512  //Max length of buffer
#define LISTEN_PORT 6321   //The port on which to listen for incoming data

void die(char *s)
{
    perror(s);
    exit(1);
}

static int make_socket_non_blocking (int sfd);

int main(void)
{

    //SOCKET ************************************************

    #define MAXBUF 1024*1024
    int socket_fd, s;
    struct sockaddr_in listenAddress;
    int length;
    int receivedDataLenght;

    char bufin[MAXBUF];

    struct sockaddr_in sendAddress;
    int adress_strict_lenght;

    /* create a socket
 IP protocol family (PF_INET)
 UDP protocol (SOCK_DGRAM)
*/

    if ((socket_fd = socket( PF_INET, SOCK_DGRAM, 0 )) < 0) {
        printf("Problem creating socket\n");
        exit(1);
    }

    /* establish our address
 address family is AF_INET
 our IP address is INADDR_ANY (any of our IP addresses)
 the port number is assigned by the kernel
*/

    listenAddress.sin_family = AF_INET;
    listenAddress.sin_port = htons(LISTEN_PORT);
    listenAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_fd, (struct sockaddr *) &listenAddress, sizeof(listenAddress))<0) {
        printf("Problem binding\n");
        exit(0);
    }

    /* find out what port we were assigned and print it out */

    length = sizeof( listenAddress );
    if (getsockname(socket_fd, (struct sockaddr *) &listenAddress, &length)<0) {
        printf("Error getsockname\n");
        exit(1);
    }

    /* port number's are network byte order, we have to convert to
 host byte order before printing !
*/
    printf("The server UDP port number is %d\n",ntohs(listenAddress.sin_port));


    s = make_socket_non_blocking (socket_fd);
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
    ev.data.fd = socket_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    static int idx = 0;
    //*******************************************************
    for(;;) {
       // printf("for(;;;) = %d\n", idx++);
        number_of_file_descriptors = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        //printf("number_of_file_descriptors = %d\n", number_of_file_descriptors);

        if (number_of_file_descriptors == -1) {
            printf("epoll_wait error\n");
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }else if(number_of_file_descriptors == 0){
            printf("epoll_wait timeout\n");
        }else if(number_of_file_descriptors > 0){

            for (int n = 0; n < number_of_file_descriptors; ++n) {
                if (events[n].data.fd == socket_fd) {

                    /* read a datagram from the socket (put result in bufin) */
                    receivedDataLenght=recvfrom(socket_fd,bufin,MAXBUF,0,(struct sockaddr *)&sendAddress,&adress_strict_lenght);

                    /* print out the address of the sender */
                   // printf("Got a datagram from %s port %d\n",
                   //        inet_ntoa(sendAddress.sin_addr), ntohs(sendAddress.sin_port));

                    if (receivedDataLenght<0) {
                        perror("Error receiving data");
                    } else {
                       // printf("GOT %d BYTES\n",receivedDataLenght);
                        /* Got something, just send it back */
                        sendto(socket_fd,bufin,receivedDataLenght,0,(struct sockaddr *)&sendAddress,adress_strict_lenght);
                    }

                }
            }
        }
    }

    close(socket_fd);
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










