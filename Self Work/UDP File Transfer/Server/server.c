#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#define bufsize 1024
#define EOFPACKET "wYZX3bXY6i7B0kZYJE1dLWXqdhJWwkR0tyJ4eh6vOT5B0DznPuwDr7sBRiUPG2MJWgdIpwXgMU18Sd8mTLUIwIEHr1s8Vdm1ED3yeXnv3f5HZL6hGeNmT5X5lWbBpy2JWZOIVDLvYT9DAjH1OA8eoJEcEz66aVw9SFrFcd7tZncPQxej80aEL1r6MTx9P6az"

int zeroBuf(char *buf, int size);
int numBytesToReadInBuf(char *buf, int size);
void printCharBufInInts(char *buf, int size, char *bufName);
int isEOFPacket(char *buf, int size);

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


    (void)bind(sckt, (struct sockaddr *)&me, sizeof(me));


    char buf[bufsize];
    unsigned int clientlen = sizeof(client);
    char fileName[128];
    char command[10];
    int flags = fcntl(sckt,F_GETFL);
    while(1)
    {   
        zeroBuf(buf, bufsize);
        printf("UDP server: waiting for datagram\n");
        fcntl(sckt, F_SETFL, flags & ~O_NONBLOCK);
        int numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
        printf("Received datagram from [host:port] = [%s:%d]\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        // printf("Num bytes received: %d\nReceived Buffer-->\n%s\n", numBytesreceived, buf);
        // printf("Sending reply\n");
        // sendto(sckt, buf, numBytesreceived, 0, (struct sockaddr *)&client, clientlen);

        zeroBuf(command, 10);
        zeroBuf(fileName, 128);

        sscanf(buf, "%s %s", command, fileName);

        FILE *f = fopen(fileName, "w");

        fd_set readfds;
        struct timeval tv;
        zeroBuf(buf, bufsize);
        fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
        tv.tv_sec = 0;
        tv.tv_usec = 200;
        FD_ZERO(&readfds);
        FD_SET(sckt, &readfds);
        while(1)
        {   
            int rv = select(sckt + 1, &readfds, NULL, NULL, &tv);
            if(rv == 1)
            {
                numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
                if(numBytesReceived < 0) {
                    //break;
                    continue;
                }
                fprintf(stderr, "Number of bytes received: %d\n", numBytesReceived);
                int numBytesInBuf = numBytesToReadInBuf(buf, bufsize);
                fprintf(stderr, "Number of bytes in the buffer: %d\n", numBytesInBuf);
                if(isEOFPacket(buf, bufsize))
                {
                    fprintf(stderr, "All Bytes in File Read\n");
                    break;
                }
                (void)fwrite(buf, sizeof(char), numBytesInBuf, f);
                printCharBufInInts(buf, bufsize, "buf");
                zeroBuf(buf, bufsize);
                fprintf(stderr,"Reached the end of the loop\n");
                // if(numBytesInBuf < bufsize - 5)
                // {
                //     // This must be the last packet
                //     fprintf(stderr, "Last Packet received!\n");
                //     break;
                // }
            }
            else 
            {
                // fprintf(stderr, "Nothing in socket right now!\n");
            }
        }

        fprintf(stderr,"Closing file\n");

        fclose(f);

    }



    return 0;
}

int isEOFPacket(char *buf, int size)
{
    
    //int numBytesInBuf = numBytesToReadInBuf(buf, bufsize);
    (void)size;
    if(strncmp(buf, EOFPACKET, strlen(EOFPACKET)) == 0)
    {
        return 1;
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
