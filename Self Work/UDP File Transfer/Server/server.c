#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define bufsize 1024
#define EOFPACKET "wYZX3bXY6i7B0kZYJE1dLWXqdhJWwkR0tyJ4eh6vOT5B0DznPuwDr7sBRiUPG2MJWgdIpwXgMU18Sd8mTLUIwIEHr1s8Vdm1ED3yeXnv3f5HZL6hGeNmT5X5lWbBpy2JWZOIVDLvYT9DAjH1OA8eoJEcEz66aVw9SFrFcd7tZncPQxej80aEL1r6MTx9P6az"
#define ACK "zFZ7HvRNh3jZjp5snMyNby3Cu0giNBc46S4hnQlYJqwb6R1Eh0nVNgIZ9REDDKLam9QcXviMnd0kg3TWGJNVm4qt43V0hRCYMEon34p68zqSUAj0JkW4ykXsCqZW6bQhWitTBMeCLy8XcR08Kx50c0VPpT9MNYE4"

int zeroBuf(char *buf, int size);
int numBytesToReadInBuf(char *buf, int size);
void printCharBufInInts(char *buf, int size, char *bufName);
void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert, int packetNum);
int isEOFPacket(char *buf, int size);
int getBufPacketNum(char *buf, int size);

// Todo: Add a packet number to each packet to test if packets are the same.

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


    int bindResult = bind(sckt, (struct sockaddr *)&me, sizeof(me));
    (void)bindResult;

    char buf[bufsize], prevBuf[bufsize];
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

        // Send an ack that we have received the message

        // fprintf(stderr, "Received Message: %s\n", buf);

        zeroBuf(command, 10);
        zeroBuf(fileName, 128);

        sscanf(buf, "%s %s", command, fileName);

        if(strncmp(command, "put", 3) == 0)
        {
            FILE *f = fopen(fileName, "w");

            struct timeval tv;
            zeroBuf(buf, bufsize);
            zeroBuf(prevBuf, bufsize);
            fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
            int packetNum;
            int oldPacketNum = 0;
            while(1)
            {   
                fd_set readfds;
                tv.tv_sec = 0;
                tv.tv_usec = 2000;
                FD_ZERO(&readfds);
                FD_SET(sckt, &readfds);
                fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
                int rv = select(sckt + 1, &readfds, NULL, NULL, &tv);
                if(rv == 1)
                {
                    // fprintf(stderr, "Attempting to Receive\n");
                    numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
                    if(numBytesReceived < 0) {
                        continue;
                    }
                    // fprintf(stderr, "Number of bytes received: %d\n", numBytesReceived);
                    int numBytesInBuf = numBytesToReadInBuf(buf, bufsize);
                    // fprintf(stderr, "Number of bytes in the buffer: %d\n", numBytesInBuf);
                    packetNum = getBufPacketNum(buf, bufsize);
                    if(isEOFPacket(buf, bufsize))
                    {
                        // fprintf(stderr, "All Bytes in File Read\n");
                        strcpy(buf, ACK);
                        numBytesReadToStringInBuf(buf, bufsize, strlen(ACK), packetNum);
                        (void)sendto(sckt, buf, bufsize, 0, (struct sockaddr *)&client, clientlen);
                        break;
                    }
                    if(packetNum != oldPacketNum)
                    {
                        // fprintf(stderr, "New buf received is unique.\n");
                        // fprintf(stderr, "Difference between this and previous packet is: %d.\n", packetNum - getBufPacketNum(prevBuf, bufsize));
                        // fprintf(stderr, "Buf packet num: %d, prevBuf packet num: %d\n", packetNum, getBufPacketNum(prevBuf, bufsize));
                        // zeroBuf(prevBuf, bufsize);
                        // strncpy(prevBuf, buf, bufsize);
                        (void)fwrite(buf, sizeof(char), numBytesInBuf, f);
                        fprintf(stderr, "New packet num: %d, Old packet num: %d\n", packetNum, oldPacketNum);
                        oldPacketNum = packetNum;
                        // printCharBufInInts(prevBuf, bufsize, "prevBuf");
                    }
                    zeroBuf(buf, bufsize);
                    // fprintf(stderr, "Sending Acknowledgement.\n");
                    strcpy(buf, ACK);
                    numBytesReadToStringInBuf(buf, bufsize, strlen(ACK), packetNum);
                    fcntl(sckt, F_SETFL, flags & ~O_NONBLOCK);
                    int numBytesSent = sendto(sckt, buf, bufsize, 0, (struct sockaddr *)&client, clientlen);
                    // fprintf(stderr, "Acknowledgement Sent. %d\n", numBytesSent);
                    (void)numBytesSent;
                }
            }

            fprintf(stderr,"Closing file\n");

            fclose(f);
        }


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
    // numAsString[4] = buf[(size-1) - 0];   

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

void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert, int packetNum)
{
    char bytesToInsert[5];
    zeroBuf(bytesToInsert, 5);
    sprintf(bytesToInsert, "%d", numBytesToInsert);
    //strcat(buf, bytesToInsert);

    // fprintf(stderr, "%c %c %c %c %c", bytesToInsert[0], bytesToInsert[1], bytesToInsert[2], bytesToInsert[3], bytesToInsert[4]);

    buf[(size-1)-4] = bytesToInsert[0];
    buf[(size-1)-3] = bytesToInsert[1];
    buf[(size-1)-2] = bytesToInsert[2];
    buf[(size-1)-1] = bytesToInsert[3];
    buf[(size-1)-0] = (packetNum % 10) + 1;//bytesToInsert[4];
}

int getBufPacketNum(char *buf, int size)
{
    return buf[(size-1)-0];
}
