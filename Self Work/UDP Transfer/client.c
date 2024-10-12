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
    while(1)
    {   
        char buf[bufsize];
        zeroBuf(buf, bufsize);
        printf("Enter some text: ");
        fgets(buf, bufsize, stdin);
        buf[strlen(buf)-1] = 0;
        sendto(sckt, buf, strlen(buf), 0, (struct sockaddr *)&serveraddress, serverlen);
        zeroBuf(buf, bufsize);
        printf("try to receive a line from the server\n");
        int numBytesReceived = recvfrom(sckt, buf, bufsize, 0, (struct sockaddr *)&serveraddress, &serverlen);
        printf("Received %d bytes\n%s\n\n", numBytesReceived, buf);
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
