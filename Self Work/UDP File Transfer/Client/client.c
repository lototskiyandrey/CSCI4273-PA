#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h> 
#include <arpa/inet.h>

#define bufsize 1024

int zeroBuf(char *buf, int size);
void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert);
void printCharBufInInts(char *buf, int size, char *bufName);

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
    
    while(1)
    {   
        zeroBuf(buf, bufsize);
        fprintf(stdout, "Enter a valid command (put FILENAME): ");
        fgets(buf, bufsize, stdin);
        //buf[strlen(buf)-1] = 0;
        
        // printf("%s\n", buf);

        zeroBuf(command, 10);
        zeroBuf(fileName, 128);

        sscanf(buf, "%s %s", command, fileName);
        
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

        // no reliable protcol checking here yet!
        fprintf(stdout, "Sending the file to the server\n");

        zeroBuf(buf, bufsize);
        ssize_t bytesReadInFile;
        while((bytesReadInFile = fread(buf, sizeof(char), bufsize-5, f)) > 0)
        {
            numBytesReadToStringInBuf(buf, bufsize, (int)bytesReadInFile);
            int numBytesSent = sendto(sckt, buf, bufsize, 0, (struct sockaddr *)&serveraddress, serverlen);
            fprintf(stderr, "Num bytes sent %d\n", numBytesSent);
            (void)numBytesSent;
            printCharBufInInts(buf, bufsize, "buf");
            zeroBuf(buf, bufsize);
        }
        
        fclose(f);
        
        // Old Code
        // sendto(sckt, buf, strlen(buf), 0, (struct sockaddr *)&serveraddress, serverlen);
        // zeroBuf(buf, bufsize);
        // printf("try to receive a line from the server\n");
        // int numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&serveraddress, &serverlen);
        // printf("Received %d bytes\n%s\n\n", numBytesReceived, buf);
    }
    
    return 0;
}


void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert)
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
    buf[(size-1)-0] = bytesToInsert[4];
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
