/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>
#include <time.h>

#define BUFSIZE 1024


#define PRIME_MODULUS 23
#define GENERATOR 5


void bitWiseXOR(char *buf, unsigned char shared_key);

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        error("ERROR opening socket");
    }
        

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    serverlen = sizeof(serveraddr);
    
    // Before we send any messages, we need to generate a public and private key, and send the public key to the server.
    //  Generate the private key as some random number
    srand(time(NULL));
    unsigned long private_key = rand() % 13;

    unsigned long public_key = (long)pow((double)GENERATOR, (double)private_key) % PRIME_MODULUS;
    printf("Prime Modulus is: %d\nGenerator is:%d\n", PRIME_MODULUS, GENERATOR);
    printf("Private key is:%lu\nPublic key is: %lu\n", private_key, public_key);

    // Next, send the public key to the server.
    // Convert the public key to a string
    bzero(buf, BUFSIZE);
    snprintf(buf, BUFSIZE, "%lu", public_key);
    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
    if(n < 0)
    {
        error("ERROR in sendto");
    }

    // Next, await the response for the server's public key
    unsigned long server_public_key;
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, (socklen_t *)&serverlen); 
    if(n < 0)
    {
        error("ERROR in recvfrom");
    }
    server_public_key = strtoul(buf, NULL, 10);

    //Finally, generate the shared key

    unsigned char shared_key = (long)pow((double)server_public_key, (double)private_key) % PRIME_MODULUS;

    printf("Received a key of: %lu\n", server_public_key);
    printf("Shared key is %d\n", shared_key);

    // Send as many messages as we want to the server
    while(1)
    {
        bzero(buf, BUFSIZE);
        printf("Please enter msg: ");
        fgets(buf, BUFSIZE, stdin);

        // Before sending the message, XOR eveyr char with our shared key

        bitWiseXOR(buf, shared_key);

        /* send the message to the server */
        n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
        if (n < 0) 
        error("ERROR in sendto");
        
    }


    
    return 0;
}


void bitWiseXOR(char *buf, unsigned char shared_key)
{
    for(int i = 0; i < BUFSIZE-1; i++)
    {
        buf[i] = buf[i] ^ shared_key;
    }
}