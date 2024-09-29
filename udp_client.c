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

    // /* socket: create the socket */
    // sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // if (sockfd < 0) 
    //     error("ERROR opening socket");

    // /* gethostbyname: get the server's DNS entry */
    // server = gethostbyname(hostname);
    // if (server == NULL) {
    //     fprintf(stderr,"ERROR, no such host as %s\n", hostname);
    //     exit(0);
    // }

    // /* build the server's Internet address */
    // bzero((char *) &serveraddr, sizeof(serveraddr));
    // serveraddr.sin_family = AF_INET;
    // bcopy((char *)server->h_addr, 
	  // (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    // serveraddr.sin_port = htons(portno);


    // --------------------------------------------
    printf("Welcome to Primitive ftp\n");
    // --------------------------------------------

    while(true) {

      // Get message from the user

      bzero(buf, BUFSIZE);
      printf("Enter a Command: (ls, exit, get [file_name], put [file_name], delete [file_name]\n");
      printf("> ");
      fgets(buf, BUFSIZE, stdin);

      char command[10];

      snprintf(command, sizeof command, "%.03s", buf);

      //printf("%s\n", buf);

      //printf("%s\n", command);

      if(strcmp(buf, "ls\n\0") == 0) {
        printf("Entered the ls command!\n");
      } 

      if(strcmp(buf,"exit\n\0") == 0) {
        printf("Entered the exit command\n");
      } 

      if(strcmp(command,"get\0") == 0) {
        printf("Entered get command!\n");
      }

      if(strcmp(command, "del\0") == 0) {
        printf("Entered delete command!\n");
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
