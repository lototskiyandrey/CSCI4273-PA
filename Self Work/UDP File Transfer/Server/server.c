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

#define bufsize 1020
#define LASTPACKET "wYZX3bXY6i7B0kZYJE1dLWXqdhJWwkR0tyJ4eh6vOT5B0DznPuwDr7sBRiUPG2MJWgdIpwXgMU18Sd8mTLUIwIEHr1s8Vdm1ED3yeXnv3f5HZL6hGeNmT5X5lWbBpy2JWZOIVDLvYT9DAjH1OA8eoJEcEz66aVw9"  //"wYZX3bXY6i7B0kZYJE1dLWXqdhJWwkR0tyJ4eh6vOT5B0DznPuwDr7sBRiUPG2MJWgdIpwXgMU18Sd8mTLUIwIEHr1s8Vdm1ED3yeXnv3f5HZL6hGeNmT5X5lWbBpy2JWZOIVDLvYT9DAjH1OA8eoJEcEz66aVw9SFrFcd7tZncPQxej80aEL1r6MTx9P6az"
#define ACK "zFZ7HvRNh3jZjp5snMyNby3Cu0giNBc46S4hnQlYJqwb6R1Eh0nVNgIZ9REDDKLam9QcXviMnd0kg3TWGJNVm4qt43V0hRCYMEon34p68zqSUAj0JkW4ykXsCqZW6bQhWitTBMeCLy8XcR08Kx50c0VPpT9MNYE4"
#define packetsize 1024

int zeroBuf(char *buf, int size);
int numBytesToReadInBuf(char *buf, int size);
void printCharBufInInts(char *buf, int size, char *bufName);
void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert, int packetNum);
int isEOFPacket(char *buf, int size);
int getBufPacketNum(char *buf, int size);
void buildPacket(int *packetNum, int packetDataLength, char *data, char *packet);
void deconstructPacket(int *packetNum, int *packetDataLength, char *data, char *packet);
// void receiveCommandFromClient(int sckt, struct sockaddr_in client, unsigned int clientlen, char *recvPacket, int flags, char *command, char *inputFile);
void receiveCommandFromClient(int *sckt, struct sockaddr_in *client, unsigned int *clientlen, char *recvPacket, int flags, char *command, char *inputFile);
void receivePacketFromClient(int sckt, struct sockaddr_in client, unsigned int clientlen, char *recvPacket);
// void sendAcknowledgementToClient(int sckt, struct sockaddr_in client, unsigned int clientlen, char *recvPacket);
void sendAcknowledgementToClient(int *sckt, struct sockaddr_in *client, unsigned int *clientlen, char *recvPacket);
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

    char recvPacket[packetsize];
    unsigned int clientlen = sizeof(client);
    char inputFile[bufsize];
    char command[bufsize];
    int flags = fcntl(sckt,F_GETFL);
    while(1)
    {   
        // zeroBuf(buf, bufsize);
        // printf("UDP server: waiting for datagram\n");
        // fcntl(sckt, F_SETFL, flags & ~O_NONBLOCK);
        // int numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
        // printf("Received datagram from [host:port] = [%s:%d]\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        // zeroBuf(command, 10);
        // zeroBuf(fileName, 128);

        // sscanf(buf, "%s %s", command, fileName);

        receiveCommandFromClient(&sckt, &client, &clientlen, recvPacket, flags, command, inputFile);

        if(strncmp(command, "put", 3) == 0)
        {
            FILE *f = fopen(inputFile, "w");

            struct timeval tv;
            int packetNum;
            int prevPacketNum = 0;
            int packetDataLength;
            char data[bufsize];
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
                    // int numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&client, &clientlen);
                    // if(numBytesReceived < 0) {
                    //     continue;
                    // }

                    // Receive a packet from the client here.

                    receivePacketFromClient(sckt, client, clientlen, recvPacket);

                    // Send an acknowledgement here.

                    sendAcknowledgementToClient(&sckt, &client, &clientlen, recvPacket);



                    // char data[bufsize];
                    // int packetNum;
                    // int packetDataLength;
                    // deconstructPacket(&packetNum, &packetDataLength, data, recvPacket);

                    // char sendPacket[packetsize];
                    // char newData[bufsize];
                    // zeroBuf(newData, bufsize);
                    // strcpy(newData, ACK);

                    // buildPacket(&packetNum, bufsize, newData, sendPacket);

                    // int numBytesSent = sendto(sckt, sendPacket, packetsize, 0, (struct sockaddr *)&client, clientlen);

                    // fprintf(stderr, "Sent acknowledgement of %d.\n", numBytesSent);
                    // fprintf(stderr, "Acknowledgement is %s\n", newData);



                    // Check whether or not the received packet is unique, and if it is unique, then write it to the file.

                    deconstructPacket(&packetNum, &packetDataLength, data, recvPacket);

                    printCharBufInInts(data, bufsize, "data");

                    if(strncmp(data, LASTPACKET, strlen(LASTPACKET)) == 0)
                    {
                        // We have reached the end of the transmission.
                        fprintf(stderr, "End of Transmision reached.\n");
                        break;
                    }

                    fprintf(stderr, "prevPacketNum %d   packetNum %d\n", prevPacketNum, packetNum);

                    if(prevPacketNum != packetNum)
                    {
                        // The new packet is different from the old one --> So it is unique
                        fprintf(stderr, "New packet is unique.\n");
                        fwrite(data, sizeof(char), packetDataLength, f);
                        prevPacketNum = packetNum;
                    }
                }
            }

            fprintf(stderr,"Closing file\n");

            fclose(f);
        }


        if(strncmp(command, "get", 3) == 0)
        {
        }


    }



    return 0;
}

// int isEOFPacket(char *buf, int size)
// {
    
//     //int numBytesInBuf = numBytesToReadInBuf(buf, bufsize);
//     (void)size;
//     if(strncmp(buf, EOFPACKET, strlen(EOFPACKET)) == 0)
//     {
//         return 1;
//     }
    
//     return 0;
// }

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


// This function is responsible for building a packet
// Packet number is an integer between 0 and 127
// packetDataLength corresponds to the amount data in the payload portion of the packet
// data is the buffer full of the data we want to send in the packet -->
// packet is the buffer which will actually be sent in the socket. packet has length 
void buildPacket(int *packetNum, int packetDataLength, char *data, char *packet)
{

    const int operationBase = 127;
    const int packetDataStart = 3;

    zeroBuf(packet, packetsize);

    if(*packetNum < 0 || *packetNum > operationBase)
    {
        fprintf(stderr, "Invalid range for a packet.\n");
        return;
    }

    if(packetDataLength < 0 || packetDataLength > bufsize)
    {
        fprintf(stderr, "Invalid range for the packet data length.\n");
        return;
    }

    if(strlen(data) > bufsize)
    {
        fprintf(stderr, "Data is too big to fit in packet.\n");
    }

    // First element of the packet contains the packetNum

    packet[0] = *packetNum % operationBase;


    // Next convert the base 10 integer packetDatalength into a base 127 number for more compact storage.
    
    packet[1] = packetDataLength / operationBase;
    packet[2] = packetDataLength % operationBase;

    // Finally, dump the data into the packet

    for(int i = 0;i < (int)strlen(data); i++)
    {
        packet[packetDataStart + i] = data[i];
    }

    // Now we are done. Our packet has been created.
    // Increment packet num by 1
    *packetNum = *packetNum + 1;

}


// This function takes a received packet, and desconstructs it into its components.
// Note that is assumes that the packet is formatted correctly, which is reasonable, since 
// we expect all packets being to be packaged correctly.
void deconstructPacket(int *packetNum, int *packetDataLength, char *data, char *packet)
{
    const int operationBase = 127;
    const int packetDataStart = 3;

    *packetNum = packet[0];

    *packetDataLength = packet[1] * operationBase + packet[2];

    zeroBuf(data, bufsize);
    for(int i = 0; i < *packetDataLength; i++)
    {
        data[i] = packet[i + packetDataStart];
    }
}


// This function checks whether or not buf is empty after reading from the file
// If Buf is empty after reading from the file, then we have reached the EOF
// If not, then proceed normally
void formatBuf(char *buf, ssize_t bytesReadInFile, int* eof)
{
    if(bytesReadInFile <= 0)
    {
        *eof = 1;
        strcpy(buf, LASTPACKET);
    }
}


void receiveCommandFromClient(int *sckt, struct sockaddr_in *client, unsigned int *clientlen, char *recvPacket, int flags, char *command, char *inputFile)
{
    
    // First received the packet
    zeroBuf(recvPacket, packetsize);
    printf("UDP server: waiting for datagram\n");
    fcntl(*sckt, F_SETFL, flags & ~O_NONBLOCK);
    int numBytesReceived = recvfrom(*sckt, recvPacket, packetsize, 0, (struct sockaddr *)client, clientlen);
    printf("Received datagram from [host:port] = [%s:%d]\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port));
    (void)numBytesReceived;

    // Next, disassemble the received packet to get the command and input file of the packet.
    char data[bufsize];
    int packetNum = 0; 
    int packetDataLength = 0;

    deconstructPacket(&packetNum, &packetDataLength, data, recvPacket);

    // Next take the data and get the command and input file from the data.
    zeroBuf(command, bufsize);
    zeroBuf(inputFile, bufsize);
    sscanf(data, "%s %s", command, inputFile);
    
}

void receivePacketFromClient(int sckt, struct sockaddr_in client, unsigned int clientlen, char *recvPacket)
{
    zeroBuf(recvPacket, packetsize);
    int numBytesReceived = recvfrom(sckt, recvPacket, packetsize, 0, (struct sockaddr *)&client, &clientlen);
    fprintf(stderr, "Received %d bytes.\n", numBytesReceived);
}

void sendAcknowledgementToClient(int *sckt, struct sockaddr_in *client, unsigned int *clientlen, char *recvPacket)
{
    char data[bufsize];
    int packetNum;
    int packetDataLength;
    deconstructPacket(&packetNum, &packetDataLength, data, recvPacket);

    char sendPacket[packetsize];
    char newData[bufsize];
    zeroBuf(newData, bufsize);
    strcpy(newData, ACK);

    buildPacket(&packetNum, bufsize, newData, sendPacket);

    int numBytesSent = sendto(*sckt, sendPacket, packetsize, 0, (struct sockaddr *)client, *clientlen);

    fprintf(stderr, "Sent acknowledgement of %d.\n", numBytesSent);
    fprintf(stderr, "Acknowledgement is %s\n", newData);
}
