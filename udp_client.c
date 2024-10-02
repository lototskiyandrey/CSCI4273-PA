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
#include <stdbool.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>

#define BUFSIZE 1024

#define true 1
#define false 0

int indexOfEOFInFile(char *buf, int bufSize);
char *getUserCommand(char *buf);
char *getFileInputted(char *buf);
int doesFileExist(char *file);
int sendPacket(int sockfd, struct sockaddr_in serveraddr, char *buf);
int receivePacket(int sockfd, struct sockaddr_in serveraddr, char *buf);
int sendAndReceieveMessageSequence(char *sendBuf, char *receiveBuf, int sockfd, struct sockaddr_in serveraddr, unsigned int sleepTime);

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno;
    // int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    // char command[BUFSIZE];
    //int numBytes;
    int flags;
    unsigned int sleepTime = 300;

    //int pS; // if programState (pS) is 0, then 

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    flags = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
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

    // set the socket to be non-blocking (will prevent the program from hanging in the event of packet loss)

    while(true) {
      char userInput[2][BUFSIZE];
      bzero(userInput[0], BUFSIZE);
      bzero(userInput[1], BUFSIZE);

      bzero(buf, BUFSIZE);
      printf("Enter a Command: ls, exit, get [file_name], put [file_name], delete [file_name], put [file_name]\n> ");
      fgets(buf, BUFSIZE, stdin);

      //char bufCopy[BUFSIZE];

      //strcpy(bufCopy, buf);

      strcpy(userInput[0], getUserCommand(buf));
      strcpy(userInput[1], getFileInputted(buf));

      if(strncmp(userInput[0], "ERROR", 5) == 0 || strncmp(userInput[1], "ERROR", 5) == 0) {
        fprintf(stderr, "ERROR: COMMAND INVALID\n");
        continue;
      }

      if(strncmp(userInput[0], "put", 3) == 0 && doesFileExist(userInput[1]) == 0) {
        fprintf(stderr, "ERROR: FILE %s ON CLIENT DOES NOT EXIST\n", userInput[1]);
        continue;
      }

      // Send message
      char messageToSend[BUFSIZE], messageToReceive[BUFSIZE];
      bzero(messageToReceive, BUFSIZE);
      bzero(messageToSend, BUFSIZE);
      strcat(messageToSend, userInput[0]);
      strcat(messageToSend, " ");
      strcat(messageToSend, userInput[1]);

      int numBytesReceived = sendAndReceieveMessageSequence(messageToSend, messageToReceive, sockfd, serveraddr, sleepTime);
      fprintf(stderr, "Receieved Message: %s\n", messageToReceive);
    }

    return 0;

    
}

int sendAndReceieveMessageSequence(char *sendBuf, char *receiveBuf, int sockfd, struct sockaddr_in serveraddr, unsigned int sleepTime) {

  int numBytesReceived;
  int i = 0;
  do {
    int numBytesSent = sendPacket(sockfd, serveraddr, sendBuf);

    fprintf(stderr, "Sent a message of size: %d\n", numBytesSent);

    usleep(sleepTime);
    numBytesReceived = receivePacket(sockfd, serveraddr, receiveBuf);
    if(numBytesReceived > 0) {
      break;
    }
    

    if(numBytesReceived > 0) {
      break;
    }
    i++;
  } while(i < 3);
  return numBytesReceived;
}

int indexOfEOFInFile(char *buf, int bufSize) {

  for(int i = 0; i < bufSize; i++) {
    if(buf[i] == EOF) {
      return i;
    }
  }

  return bufSize;
}

char *getUserCommand(char *buf) {


  if(strncmp(buf, "ls", 2) == 0) {    
    return "ls";
  }

  if(strncmp(buf, "exit", 4) == 0) {
    return "exit";
  }

  if(strncmp(buf, "get", 3) == 0) {
    return "get";
  }

  if(strncmp(buf, "put", 3) == 0) {
    return "put";
  }

  if(strncmp(buf, "delete", 6) == 0) {
    return "delete";
  }
  
  return "ERROR";
}

char *getFileInputted(char *buf) {
  // bzero(returnString, BUFSIZE);
  if(strncmp(buf, "get", 3) == 0 || strncmp(buf, "put", 3) == 0 || strncmp(buf, "delete", 6) == 0) {
    char command[BUFSIZE], input[BUFSIZE];
    bzero(command, BUFSIZE);
    bzero(input, BUFSIZE);
    sscanf(buf, "%s %s", command, input);
    //fprintf(stderr, "%d\n", strlen(input));
    if(strlen(input) == 0) {
      bzero(buf, BUFSIZE);
      strncpy(buf, "ERROR", 6);
      return buf;
    }
    bzero(buf, BUFSIZE);
    strncpy(buf, input, strlen(input));
    return buf;
  }

  bzero(buf, BUFSIZE);
  strncpy(buf, "NoFiletoSend", 13);
  return buf;
}

int doesFileExist(char *file) {
  // return 1 if yes, 0 if no
  FILE *f = fopen(file, "rb");
  if(f == NULL) {
    return 0;
  }
  fclose(f);
  return 1;
}

int sendPacket(int sockfd, struct sockaddr_in serveraddr, char *buf) {
  int serverlen = sizeof(serveraddr);

  int numBytesSent = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);

  if(numBytesSent < 0) {
    error("ERROR in sendto");
  }

  return numBytesSent;
}

int receivePacket(int sockfd, struct sockaddr_in serveraddr, char *buf) {
  int serverlen = sizeof(serveraddr);
  bzero(buf, BUFSIZE);
  int numBytesReceieved = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);

  if(numBytesReceieved < 0) {
    error("ERROR in recvfrom");
  }

  return numBytesReceieved;
}
