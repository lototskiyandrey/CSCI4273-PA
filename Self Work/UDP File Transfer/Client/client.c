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

#define bufsize 1024
#define EOFPACKET "wYZX3bXY6i7B0kZYJE1dLWXqdhJWwkR0tyJ4eh6vOT5B0DznPuwDr7sBRiUPG2MJWgdIpwXgMU18Sd8mTLUIwIEHr1s8Vdm1ED3yeXnv3f5HZL6hGeNmT5X5lWbBpy2JWZOIVDLvYT9DAjH1OA8eoJEcEz66aVw9SFrFcd7tZncPQxej80aEL1r6MTx9P6az"
#define ACK "zFZ7HvRNh3jZjp5snMyNby3Cu0giNBc46S4hnQlYJqwb6R1Eh0nVNgIZ9REDDKLam9QcXviMnd0kg3TWGJNVm4qt43V0hRCYMEon34p68zqSUAj0JkW4ykXsCqZW6bQhWitTBMeCLy8XcR08Kx50c0VPpT9MNYE4"
#define NUMSENDS 10

int zeroBuf(char *buf, int size);
// void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert);
void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert, int packetNum);
void printCharBufInInts(char *buf, int size, char *bufName);
int numBytesToReadInBuf(char *buf, int size);
int bufPacketNum(char *buf, int size);

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
        //buf[strlen(buf)-1] = 0;
        
        // printf("%s\n", buf);

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

            int numBytesSent = sendto(sckt, buf, strlen(buf), 0, (struct sockaddr *)&serveraddress, serverlen);
            if(numBytesSent < 0)
            {
                fprintf(stderr, "Message not sent.\n");
                continue;
            }

            fcntl(sckt, F_SETFL, flags & ~O_NONBLOCK);
            fprintf(stdout, "Sending the file to the server\n");
            int packetNum = 0;
            zeroBuf(buf, bufsize);
            ssize_t bytesReadInFile;
            while(1)
            {
                bytesReadInFile = fread(buf, sizeof(char), bufsize-5, f);
                if(bytesReadInFile <= 0)
                {
                    zeroBuf(buf, bufsize);
                    strcpy(buf, EOFPACKET);
                    numBytesReadToStringInBuf(buf, bufsize, strlen(EOFPACKET), 0);
                    numBytesSent = sendto(sckt, buf, bufsize, 0, (struct sockaddr *)&serveraddress, serverlen);
                    break;
                }
                
                int numSends = 0;
                int hasSent = 0;
                while(numSends < NUMSENDS && hasSent == 0)
                {
                    numBytesReadToStringInBuf(buf, bufsize, (int)bytesReadInFile, packetNum);
                    int numBytesSent = sendto(sckt, buf, bufsize, 0, (struct sockaddr *)&serveraddress, serverlen);
                    fprintf(stderr, "Num bytes sent %d\n", numBytesSent);
                    (void)numBytesSent;
                    printCharBufInInts(buf, bufsize, "buf");
                    // zeroBuf(buf, bufsize);

                    fprintf(stderr, "Waiting for Acknowledgement.\n");
                    fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
                    int numBytesReceived;
                    int numListens = 0;

                    char ACKBuf[bufsize];

                    while(numListens < 10)
                    {
                        tv.tv_sec = 0;
                        tv.tv_usec = 2000;
                        FD_ZERO(&readfds);
                        FD_SET(sckt, &readfds);
                        fcntl(sckt, F_SETFL, flags | O_NONBLOCK);
                        int rv = select(sckt + 1, &readfds, NULL, NULL, &tv);
                        if(rv == 1)
                        {   
                            zeroBuf(ACKBuf, bufsize);
                            numBytesReceived = recvfrom(sckt, ACKBuf, bufsize, 0, (struct sockaddr *)&serveraddress, &serverlen);
                            if(numBytesReceived < 0)
                            {
                                continue;
                            }
                            //fprintf(stderr, "Received buf: %s\n", buf);
                            printCharBufInInts(ACKBuf, bufsize, "ACK");
                            if(strncmp(ACKBuf, ACK, strlen(ACK)) == 0)
                            {
                                // ack received
                                fprintf(stderr, "ACK Received.\n");
                                //hasSent = 1;
                                fprintf(stderr, "%d\n", buf[strlen(ACK)]);
                                fprintf(stderr, "%d\n", packetNum+1);
                                if(ACKBuf[strlen(ACK)] == (packetNum%10)+1)
                                {
                                    fprintf(stderr, "ACK is for correct packet\n");
                                    hasSent = 1;
                                    break;
                                }
                            }

                        }
                        numListens++;
                    }
                    numSends++;    
                }
                if(numSends >= NUMSENDS)
                {
                    fprintf(stderr, "Connection to server must have been lost.\n");
                    return 1;
                }
                
                packetNum++;

            }
            
            fclose(f);
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
