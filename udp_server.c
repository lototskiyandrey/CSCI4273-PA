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
#include <dirent.h>

#define BUFSIZE 1024

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
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

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
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
    
    /* 
     * sendto: echo the input back to the client 
     */

    // first determine what command was sent.

    char command[BUFSIZE], fileName[BUFSIZE];

    sscanf(buf, "%s %s", command, fileName);

    if(strncmp(command, "ls", 2) == 0) {

      bzero(buf, BUFSIZE);
      DIR *dirp;

      dirp = opendir(".");

      while(1) {
        struct dirent *dp = readdir(dirp);
        if(dp == NULL) {
          break;
        }
        // strcat(buf, dp->d_name);
        //strncat(buf, "\n", 1);

        printf("Sending message %s\n", dp->d_name);

        // send the name of the file to the client
        char ack[4];
        char recieveBuffer[BUFSIZE];
        do {
          bzero(buf, BUFSIZE);
          strcat(buf, dp->d_name);
          n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
          if(n < 0) {
            error("ERROR in sendto");
          }

          

          // recieve an ACK from the client
          bzero(recieveBuffer, BUFSIZE);
          n = recvfrom(sockfd, recieveBuffer, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);

          sscanf(recieveBuffer, "%s", ack);

          if(strncmp(ack, "ack", 3) == 0) {
            printf("Acknowledgement Received.\n");
          }

          // printf("%s\n", dp->d_name);
          // printf("Ack: %s\n", ack);
          // printf("Buffer state: %s\n", buf);
          // printf("Recieve Buffer State: %s\n", recieveBuffer);
        } while(strncmp(ack, "ack", 3) != 0);
        
      }

      char endOfMessage[7] = "000000\0";

      // send the indicator that we have finished transmission to the host.
      char ack[4];
      char recieveBuffer[BUFSIZE];
      do {
        printf("Transmission complete: Sending termination message\n");
        n = sendto(sockfd, endOfMessage, strlen(endOfMessage), 0, (struct sockaddr *) &clientaddr, clientlen);
        if(n < 0) {
          error("ERROR in sendto");
        }

        bzero(recieveBuffer, BUFSIZE);
        n = recvfrom(sockfd, recieveBuffer, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);

        sscanf(recieveBuffer, "%s", ack);

        if(strncmp(ack, "ack", 3) == 0) {
          printf("Acknowledgement Received.\n");
        }

      } while(strncmp(ack, "ack", 3) != 0);
      



    }

    else if(strncmp(command, "exit", 4) == 0) {
      printf("Exiting Program.\n");
      break;
    }

    else if(strncmp(command, "get", 3) == 0) {

    }

    else if(strncmp(command, "put", 3) == 0) {
      
    }

    else if(strncmp(command, "delete", 3) == 0) {
      
    }

    /*
    n = sendto(sockfd, buf, strlen(buf), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
    */
  }
}
