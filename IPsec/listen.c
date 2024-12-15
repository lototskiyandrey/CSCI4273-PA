
// Andriy Lototskiy
// Code is extended from the defautl given in PA1


/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>

#define BUFSIZE 1024


#define PRIME_MODULUS 23
#define GENERATOR 5

void bitWiseXOR(char *buf, unsigned char shared_key);

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
    int sockfd; /* socket */
    int portno; /* port to listen on */
    int clientlen; /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp; /* client host info */
    char buf[BUFSIZE]; /* message buf */
    char *hostaddrp; /* dotted decimal host addr string */
    int optval; /* flag value for setsockopt */
    int n; /* message byte size */

    /* 
    * check command line arguments 
    */
    if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
    }
    portno = atoi(argv[1]);

    /* 
    * socket: create the parent socket 
    */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
    error("ERROR opening socket");
    }

    /* setsockopt: Handy debugging trick that lets 
    * us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    * Eliminates "ERROR on binding: Address already in use" error. 
    */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
            (const void *)&optval , sizeof(int));

    /*
    * build the server's Internet address
    */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /* 
    * bind: associate the parent socket with a port 
    */
    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
    {
        error("ERROR on binding");
    }
    

    clientlen = sizeof(clientaddr);

    // Again, before receiving inputs, generate our private, and public key.


    srand(time(NULL));
    unsigned long private_key = rand() % 13;

    unsigned long public_key = (long)pow((double)GENERATOR, (double)private_key) % PRIME_MODULUS;
    printf("Prime Modulus is: %d\nGenerator is:%d\n", PRIME_MODULUS, GENERATOR);
    printf("Private key is:%lu\nPublic key is: %lu\n", private_key, public_key);
    printf("%ld\n", (long)pow((double)GENERATOR, (double)private_key));

    // expect to receive a public key
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, (socklen_t *)&clientlen);
    if(n < 0)
    {
        error("ERROR in recvfom");
    }

    unsigned long client_public_key = strtoul(buf, NULL, 10);

    unsigned char shared_key = (long)pow((double)client_public_key, (double)private_key) % PRIME_MODULUS;

    // Finally, send the public key back to the client
    bzero(buf, BUFSIZE);
    snprintf(buf, BUFSIZE, "%lu", public_key);
    n = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, clientlen);
    if(n < 0)
    {
        error("ERROR in sendto");
    }

    printf("Received a key of: %lu\n", client_public_key);
    printf("Shared key is %d\n", shared_key);

    while (1) {
        bzero(buf, BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, (socklen_t *)&clientlen);
        if (n < 0)
            error("ERROR in recvfrom");


        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL)
        {
            error("ERROR on gethostbyaddr");
        }
            
        hostaddrp = inet_ntoa(clientaddr.sin_addr);

        if (hostaddrp == NULL)
        {
            error("ERROR on inet_ntoa\n");
        }

        printf("Received message from %s (%s)\n", hostp->h_name, hostaddrp);
        printf("Received %d/%d bytes: %s\n", (int)strlen(buf), n, buf);

        // Finally, use the shared key to decrypt our message
        bitWiseXOR(buf, shared_key);
        printf("Decrypted Message: %s", buf);
    }
}

void bitWiseXOR(char *buf, unsigned char shared_key)
{
    for(int i = 0; i < BUFSIZE-1; i++)
    {
        buf[i] = buf[i] ^ shared_key;
    }
}