#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define bufsize 1024

int zeroBuf(char *buf, int size);
int numBytesToReadInBuf(char *buf, int size);
void printCharBufInInts(char *buf, int size, char *bufName);

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
    char fileName[128];
    char command[10];
    int flags = fcntl(sckt,F_GETFL);
    while(1)
    {   
        zeroBuf(buf, bufsize);
        printf("UDP server: waiting for datagram\n");
        //fcntl(sckt, flags & ~O_NONBLOCK);
        int numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
        printf("Received datagram from [host:port] = [%s:%d]\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        // printf("Num bytes received: %d\nReceived Buffer-->\n%s\n", numBytesreceived, buf);
        // printf("Sending reply\n");
        // sendto(sckt, buf, numBytesreceived, 0, (struct sockaddr *)&client, clientlen);

        zeroBuf(command, 10);
        zeroBuf(fileName, 128);

        sscanf(buf, "%s %s", command, fileName);

        FILE *f = fopen(fileName, "w");

        zeroBuf(buf, bufsize);
        fcntl(sckt, flags | O_NONBLOCK);
        while(1)
        {   
            numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
            if(numBytesReceived < 0) {
                break;
            }
            fprintf(stderr, "Number of bytes received: %d\n", numBytesReceived);
            int numBytesInBuf = numBytesToReadInBuf(buf, bufsize);
            fprintf(stderr, "Number of bytes in the buffer: %d\n", numBytesInBuf);
            (void)fwrite(buf, sizeof(char), numBytesInBuf, f);
            printCharBufInInts(buf, bufsize, "buf");
            zeroBuf(buf, bufsize);
            fprintf(stderr,"Reached the end of the loop\n");
        }

        fprintf(stderr,"Closing file\n");

        fclose(f);

    }



    return 0;
}

void printCharBufInInts(char *buf, int size, char *bufName)
{   
    (void)buf;
    (void)size;
    (void)bufName;
    fprintf(stderr, "Printing buffer %s as ints\n", bufName);

    for(int i = 0; i < size; i++)
    {
        fprintf(stderr, "%d", buf[i]);
    }
    fprintf(stderr, "\n");
}

int numBytesToReadInBuf(char *buf, int size)
{
    char numAsString[5];

    numAsString[0] = buf[(size-1) - 4];
    numAsString[1] = buf[(size-1) - 3];
    numAsString[2] = buf[(size-1) - 2];
    numAsString[3] = buf[(size-1) - 1];
    numAsString[4] = buf[(size-1) - 0];   

    // fprintf(stderr, "%d %d %d %d %d", numAsString[0], numAsString[1], numAsString[2], numAsString[3], numAsString[4]);

    return atoi(numAsString);
}

int zeroBuf(char *buf, int size)
{
    if(size <= 0) {
        return -1;
    }

    memset(buf, 0, size*sizeof(char));

    return size;
}
