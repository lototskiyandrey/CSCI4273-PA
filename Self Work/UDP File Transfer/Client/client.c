#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

#define bufsize 1020
#define LASTPACKET "wYZX3bXY6i7B0kZYJE1dLWXqdhJWwkR0tyJ4eh6vOT5B0DznPuwDr7sBRiUPG2MJWgdIpwXgMU18Sd8mTLUIwIEHr1s8Vdm1ED3yeXnv3f5HZL6hGeNmT5X5lWbBpy2JWZOIVDLvYT9DAjH1OA8eoJEcEz66aVw9"      //"wYZX3bXY6i7B0kZYJE1dLWXqdhJWwkR0tyJ4eh6vOT5B0DznPuwDr7sBRiUPG2MJWgdIpwXgMU18Sd8mTLUIwIEHr1s8Vdm1ED3yeXnv3f5HZL6hGeNmT5X5lWbBpy2JWZOIVDLvYT9DAjH1OA8eoJEcEz66aVw9SFrFcd7tZncPQxej80aEL1r6MTx9P6az"
#define ACK       "zFZ7HvRNh3jZjp5snMyNby3Cu0giNBc46S4hnQlYJqwb6R1Eh0nVNgIZ9REDDKLam9QcXviMnd0kg3TWGJNVm4qt43V0hRCYMEon34p68zqSUAj0JkW4ykXsCqZW6bQhWitTBMeCLy8XcR08Kx50c0VPpT9MNYE4"
#define NUMSENDS 10
#define packetsize 1024

int zeroBuf(char *buf, int size);
// void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert);
void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert, int packetNum);
void printCharBufInInts(char *buf, int size, char *bufName);
int numBytesToReadInBuf(char *buf, int size);
int bufPacketNum(char *buf, int size);
void buildPacket(int *packetNum, int packetDataLength, char *data, char *packet);
void deconstructPacket(int *packetNum, int *packetDataLength, char *data, char *packet);
void formatBuf(char *buf, ssize_t bytesReadInFile, int* eof, char *packet, int *packetNum);
int checkIfPacketMatchesAck(int *packetNum, int *packetDataLength, char *data, int *prevPacket);

// Todo: Add a packet number to each packet to test if packets are the same.

int main(int argc, char **argv)
{
    
    if(argc != 3)
    {
        fprintf(stderr, "Wrong number of inputs\n");
        exit(-1);
    }

    char *servername = argv[1];
    int serverport = atoi(argv[2]);

    int sckt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(sckt < 0)
    {
        fprintf(stderr, "Socket opening failed\n");
    }

    // struct hostend *server = gethostbyname(servername);

    // if(server == NULL)
    // {
    //     fprintf(stderr, "Host %s does not exist\n", servername);
    //     exit(-1);
    // }

    struct sockaddr_in serveraddress;
    memset((char *) &serveraddress, 0, sizeof(serveraddress));
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(serverport);
    inet_aton(servername, &serveraddress.sin_addr);

    unsigned int serverlen = sizeof(serveraddress);
    (void)serverlen;
    char buf[bufsize];
    char packet[packetsize];
    // int numBytesSent;
    char fileName[128];
    char command[10];

    int flags = fcntl(sckt, F_GETFL);
    fd_set readfds;
    struct timeval tv;

    
    while(1)
    {   
        fcntl(sckt, F_SETFL, flags & ~O_NONBLOCK);
        zeroBuf(buf, bufsize);
        fprintf(stdout, "Enter a valid command (put FILENAME): ");
        fgets(buf, bufsize, stdin);

        zeroBuf(command, 10);
        zeroBuf(fileName, 128);

        sscanf(buf, "%s %s", command, fileName);

        if(strncmp(command, "put", 3) == 0)
        {
            FILE *f = fopen(fileName, "r");

            if(f == NULL)
            {
                fprintf(stderr, "File %s does not exist.\n", fileName);
                continue;
            }
            
            // First we will build the packet to send
            int packetNum = 0;

            buildPacket(&packetNum, strlen(buf), buf, packet);
            int numBytesSent = sendto(sckt, packet, packetsize, 0, (struct sockaddr *)&serveraddress, serverlen);
            if(numBytesSent < 0)
            {
                fprintf(stderr, "Message not sent.\n");
                continue;
            }

            fcntl(sckt, F_SETFL, flags & ~O_NONBLOCK);
            fprintf(stderr, "Sent the file.\n");

            
            int isEOF = 0;
            while(isEOF == 0)
            {   
                zeroBuf(buf, bufsize);   
                ssize_t bytesReadInFile = fread(buf, sizeof(char), bufsize, f);
                
                
                // build packet
                formatBuf(buf, bytesReadInFile, &isEOF, packet, &packetNum);
                

                int numSends = 0;
                int hasSent = 0;
                while(numSends < NUMSENDS && hasSent == 0)
                {
                    int numBytesSent = sendto(sckt, packet, packetsize, 0, (struct sockaddr *)&serveraddress, serverlen);
                    fprintf(stderr, "Num bytes sent %d\n", numBytesSent);
                    (void)numBytesSent;
                    

                    fprintf(stderr, "Waiting for Acknowledgement.\n");
                    // fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
                    // int numBytesReceived;
                    fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
                    int numListens = 0;
                    while(hasSent == 0  && numListens < NUMSENDS)
                    {
                        tv.tv_sec = 0;
                        tv.tv_usec = 2000;
                        FD_ZERO(&readfds);
                        FD_SET(sckt, &readfds);
                        fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
                        int rv = select(sckt + 1, &readfds, NULL, NULL, &tv);
                        char recvPacket[packetsize];
                        if(rv == 1)
                        {   
                            zeroBuf(recvPacket, packetsize);
                            int numBytesReceived = recvfrom(sckt, recvPacket, packetsize, 0, (struct sockaddr *)&serveraddress, &serverlen);
                            if(numBytesReceived < 0)
                            {
                                continue;
                            }
                            
                            int recvPacketNum;
                            int recvPacketDataLength;
                            char recvData[bufsize];

                            deconstructPacket(&recvPacketNum, &recvPacketDataLength, recvData, recvPacket);

                            //printCharBufInInts(recvPacket, packetsize, "received packet");
                            fprintf(stderr, "Printing received: %s\n", recvData);

                            if(checkIfPacketMatchesAck(&recvPacketNum, &recvPacketDataLength, recvData, &packetNum) == 1)
                            {
                                fprintf(stderr, "Acknowledgement Received.\n");
                                hasSent = 1;
                            }

                        }
                        numListens++;
                    }
                    numSends++;    
                }

                if(isEOF == 1)
                {
                    break;
                }

                if(numSends >= NUMSENDS && hasSent == 0)
                {
                    fprintf(stderr, "Connection to server must have been lost.\n");
                    return 1;
                }

            }
            
            fclose(f);
        }
        
        if(strncmp(command, "get", 3) == 0)
        {
        }

    }
    
    return 0;
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
    // buf[(size-1)-0] = bytesToInsert[4];
    buf[(size-1)-0] = (packetNum % 10) + 1;
}


int zeroBuf(char *buf, int size)
{
    if(size <= 0) {
        return -1;
    }

    memset(buf, 0, size*sizeof(char));

    return size;
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

int bufPacketNum(char *buf, int size)
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

    // if(strlen(data)-1 > bufsize)
    // {
    //     fprintf(stderr, "Data is too big to fit in packet.\n");
    // }

    // First element of the packet contains the packetNum

    packet[0] = *packetNum % operationBase;


    // Next convert the base 10 integer packetDatalength into a base 127 number for more compact storage.
    
    packet[1] = packetDataLength / operationBase;
    packet[2] = packetDataLength % operationBase;

    // Finally, dump the data into the packet

    for(int i = 0;i < packetDataLength; i++)
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
void formatBuf(char *buf, ssize_t bytesReadInFile, int* eof, char *packet, int *packetNum)
{
    if(bytesReadInFile <= 0)
    {
        *eof = 1;
        zeroBuf(buf, bufsize);
        strcpy(buf, LASTPACKET);
        fprintf(stderr, "Sending end of transmission message.\n");
        fprintf(stderr, "%d\n", (int)strlen(buf));
        buildPacket(packetNum, strlen(LASTPACKET), buf, packet);
        return;
    }

    fprintf(stderr, "%d\n", (int)strlen(buf));
    buildPacket(packetNum, (int)bytesReadInFile, buf, packet);
    fprintf(stderr, "Sending regular buf.\n");
}

// This function checks whether or not a received packet is the appropriate acknowledgement to the sent packet
// It return 1 if the acknowledgement does match, and 0 if it does not.
int checkIfPacketMatchesAck(int *packetNum, int *packetDataLength, char *data, int *prevPacket)
{
    const int operationBase = 127;
    int truePrevPacket = *prevPacket - 1;
    if(*prevPacket - 1 < 0)
    {
        truePrevPacket = operationBase;
    }

    if(*packetNum == truePrevPacket && strncmp(data, ACK, *packetDataLength) == 0)
    {
        return 1;
    }

    return 0;
}