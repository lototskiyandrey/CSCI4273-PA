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

#define BUFSIZE 1024

#define true 1
#define false 0

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
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);


    // --------------------------------------------
    printf("Welcome to Primitive ftp\n");
    // --------------------------------------------

    while(true) {

      // Get message from the user

      bzero(buf, BUFSIZE);
      printf("Enter a Command: ls, exit, get [file_name], put [file_name], delete [file_name], put [file_name]\n");
      printf("> ");
      fgets(buf, BUFSIZE, stdin);

      char command[BUFSIZE], inputFile[BUFSIZE];

      sscanf(buf, "%s, %s", command, inputFile);


      if(strncmp(command, "ls", 2) == 0) {

        /* send the message to the server */
        serverlen = sizeof(serveraddr);
        n = sendto(sockfd, command, strlen(command), 0, (struct sockaddr *)&serveraddr, serverlen);
        if (n < 0) {
          error("ERROR in sendto");
        }

        char ack[] = "ack\0";
        char eof[] = "000000\0";
        //char data[1024];
        bzero(buf, BUFSIZE);
        do {
          /* print the server's reply */
          bzero(buf, BUFSIZE);
          n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
          if(n < 0) {
            error("ERROR in recvfrom");
            // print the buffer sent back
          }
          
          // send an ack saying that the message has been recieved
          //printf("Message recieved from server: %s\n", buf);
          //printf("Message being sent to server: %s\n", ack);
          n = sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&serveraddr, serverlen);
          if (n < 0) {
            error("ERROR in sendto");
          }

          // break if we recieve an end of transmission string
          if(strncmp(buf, eof, 6) == 0) {
            break;
          }

          printf("%s ", buf);
          
        } while(1);


        printf("\n");
      } 

      else if(strncmp(command,"exit", 4) == 0) {
        serverlen = sizeof(serveraddr);
        n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
        if (n < 0) {
          error("ERROR in sendto");
        }

        close(sockfd);
        printf("Connection Terminated.\n");
        break;  
      } 

      else if(strncmp(command,"get", 3) == 0) {
        printf("Entered get command!\n");
      }

      else if(strncmp(command, "delete", 6) == 0) {
        //printf("Entered delete command!\n");

        char receiverBuf[BUFSIZE] = "NORESP";

        do {
          serverlen = sizeof(serveraddr);
          n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
          if (n < 0) {
            error("ERROR in sendto");
          }

          //bzero(buf, BUFSIZE);
          n = recvfrom(sockfd, receiverBuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
          if(n < 0) {
            error("ERROR in recvfrom");
          }

        } while(strncmp(receiverBuf, "NORESP", 6) == 0);

        printf("%s\n", receiverBuf);
      }

      else if(strncmp(command, "put", 3) == 0) {
        printf("Entered put command!\n");
      }

      else {
        // Unknown Command

        printf("Unknown Command\n.");
      }
    }

    // /* get a message from the user */
    // bzero(buf, BUFSIZE);
    // printf("Please enter msg: ");
    // fgets(buf, BUFSIZE, stdin);

    // /* send the message to the server */
    // serverlen = sizeof(serveraddr);
    // n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
    // if (n < 0) 
    //   error("ERROR in sendto");
    
    // /* print the server's reply */
    // n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
    // if (n < 0) 
    //   error("ERROR in recvfrom");
    // printf("Echo from server: %s", buf);
    return 0;

    
}
