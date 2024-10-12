#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define bufsize 1024

int zeroBuf(char *buf, int size);

int main(int argc, char **argv)
{
    
    if(argc != 2)
    {
        fprintf(stderr, "Enter a port number\n");
        exit(-1);
    }

    int portnumber = atoi(argv[1]);

    int sckt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in me, client;

    memset((char *)&me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_port = htons(portnumber);
    me.sin_addr.s_addr = htonl(INADDR_ANY);


    bind(sckt, (struct sockaddr *)&me, sizeof(me));


    char buf[bufsize];
    unsigned int clientlen = sizeof(client);
    while(1)
    {   
        zeroBuf(buf, bufsize);
        printf("UDP server: waiting for datagram\n");
        int numBytesreceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
        printf("Received datagram from [host:port] = [%s:%d]\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        printf("Num bytes received: %d\nReceived Buffer-->\n%s\n", numBytesreceived, buf);
        printf("Sending reply\n");
        sendto(sckt, buf, numBytesreceived, 0, (struct sockaddr *)&client, clientlen);
    }



    return 0;
}

int zeroBuf(char *buf, int size)
{
    if(size <= 0) {
        return -1;
    }

    memset(buf, 0, size*sizeof(char));

    return size;
}
