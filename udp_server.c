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
#include <fcntl.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int indexOfEOFInFile(char *buf, int bufSize);
int waitConnectionFromClient(int sockfd, struct sockaddr_in *clientaddr, char *connectionMessage, int flags, struct hostent *hostp, char *hostaddrp);
int sendPacket(int sockfd, struct sockaddr_in *clientaddr, char *buf);
char *getUserCommand(char *buf);
int receivePacket(int sockfd, struct sockaddr_in *clientaddr, char *buf);
int doesFileExist(char *file);
char *getFileInputted(char *buf);
int sendFile(int sockfd, struct sockaddr_in *clientaddr, char *fileName, unsigned int sleepTime, int flags);
void setBlocking(int sock);
void setNonBlocking(int sock);


int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  //int clientlen; /* byte size of client's address */
  struct sockaddr_in *serveraddr; /* server's addr */
  struct sockaddr_in *clientaddr; /* client addr */
  struct sockaddr_in initializeServerAddr;
  struct sockaddr_in initializeClientAddr;
  struct hostent *hostp; /* client host info */
  //char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  //int n; /* message byte size */
  int flags;
  unsigned int sleepTime = 300;

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
  flags = fcntl(sockfd, F_GETFL);
  //fcntl(sockfd, F_SETFL, flags);
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
  bzero((char *) &(initializeServerAddr), sizeof(initializeServerAddr));
  (initializeServerAddr).sin_family = AF_INET;
  (initializeServerAddr).sin_addr.s_addr = htonl(INADDR_ANY);
  (initializeServerAddr).sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &(initializeServerAddr), sizeof(initializeServerAddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  //clientlen = sizeof(clientaddr);

  serveraddr = &initializeServerAddr;
  clientaddr = &initializeClientAddr;

  while(1) {
    fprintf(stderr, "Waiting for a client\n");

    // wait for a client to connect.

    char connectionMessage[BUFSIZE];
    bzero(connectionMessage, BUFSIZE);
    int numBytesReceived = waitConnectionFromClient(sockfd, clientaddr, connectionMessage, flags, hostp, hostaddrp);

    if(numBytesReceived < 0) {
      continue;
    }

    // fcntl(sockfd, F_SETFL, flags);
    // bzero(connectionMessage, BUFSIZE);
    // int clientlen = sizeof(clientaddr);
    // int numBytesReceived = recvfrom(sockfd, connectionMessage, BUFSIZE, 0, (struct sockaddr*)&clientaddr, &clientlen);
    // if(numBytesReceived < 0) {
    //   error("ERROR in recvfrom");
    // }

    // hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    // if (hostp == NULL){
    //   error("ERROR on gethostbyaddr");
    // }
    // hostaddrp = inet_ntoa(clientaddr.sin_addr);
    // if (hostaddrp == NULL){
    //   error("ERROR on inet_ntoa\n");
    //   }
    // printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);

    // fprintf(stderr, "Message Received: %s\n", connectionMessage);

    // fprintf(stderr, "Echoing received Message: %s\n", connectionMessage);
    // int numBytesSent = sendPacket(sockfd, clientaddr, connectionMessage);

    char command[BUFSIZE], inputFile[BUFSIZE];
    bzero(command, BUFSIZE);
    bzero(inputFile, BUFSIZE);

    strcpy(command, getUserCommand(connectionMessage));
    //get

    if(strncmp(command, "get", 3) == 0) {
      fprintf(stderr, "Getting file\n");
      strcpy(inputFile, getFileInputted(connectionMessage));
      bzero(connectionMessage, BUFSIZE);
      strncpy(connectionMessage, "Sending_File", 12);

      int numBytesSent = sendPacket(sockfd, clientaddr, connectionMessage);
      sendFile(sockfd, clientaddr, inputFile, sleepTime, flags);
    }

    // printf("server received %ld/%d bytes: %s\n", strlen(buf), n, buf);

    // when a client connects, service that client.
  }

  return 0;
}

int receivePacket(int sockfd, struct sockaddr_in *clientaddr, char *buf) {
  int clientlen = sizeof(*clientaddr);
  bzero(buf, BUFSIZE);
  int numBytesReceieved = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)clientaddr, &clientlen);
  printf(stderr, "Num bytes received: %d\n", numBytesReceieved);
  // if(numBytesReceieved < 0) {
  //   error("ERROR in recvfrom");
  // }

  return numBytesReceieved;
}

int sendPacket(int sockfd, struct sockaddr_in *clientaddr, char *buf) {
  int clientlen = sizeof(*clientaddr);

  int numBytesSent = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&(*clientaddr), clientlen);

  if(numBytesSent < 0) {
    error("ERROR in sendto");
  }

  return numBytesSent;
}

int waitConnectionFromClient(int sockfd, struct sockaddr_in *clientaddr, char *connectionMessage, int flags, struct hostent *hostp, char *hostaddrp) {
  //fcntl(sockfd, F_SETFL, flags);
  bzero(connectionMessage, BUFSIZE);
  int clientlen = sizeof(*clientaddr);
  setBlocking(sockfd);
  int numBytesReceived = recvfrom(sockfd, connectionMessage, BUFSIZE, 0, (struct sockaddr*)clientaddr, &clientlen);
  //setNonBlocking(sockfd);
  //setBlocking(sockfd);
  fprintf(stderr, "receieved message %s\n", connectionMessage);
  fprintf(stderr, "Num bytes received %d\n", numBytesReceived);
  char ack[] = "1234567890";
  if(numBytesReceived < 0) {
    error("ERROR in recvfrom");
    return numBytesReceived;
  }
  if(strncmp(connectionMessage, ack , 10) == 0) {
    fprintf(stderr, "Invalid Ack\n");
    return -1;
  }

  hostp = gethostbyaddr((const char *)&((*clientaddr).sin_addr.s_addr), sizeof((*clientaddr).sin_addr.s_addr), AF_INET);
  if (hostp == NULL){
    error("ERROR on gethostbyaddr");
  }
  hostaddrp = inet_ntoa((*clientaddr).sin_addr);
  if (hostaddrp == NULL){
    error("ERROR on inet_ntoa\n");
    }
  fprintf(stderr, "server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);


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

  if(strncmp(buf, "get ", 4) == 0) {
    return "get";
  }

  if(strncmp(buf, "put ", 4) == 0) {
    return "put";
  }

  if(strncmp(buf, "delete ", 7) == 0) {
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

int sendFile(int sockfd, struct sockaddr_in *clientaddr, char *fileName, unsigned int sleepTime, int flags) {
  // this assumes that the file we want does indeed exist
  FILE *f = fopen(fileName, "rb");

  // get the file size
  fseek(f, 0L, SEEK_END);
  int numBytesNeededToSend = ftell(f);
  rewind(f);

  int numBytesSent = 0;

  char buf[BUFSIZE], previousBuf[BUFSIZE], messageReceived[BUFSIZE], expectedAck[BUFSIZE];
  bzero(buf, BUFSIZE);
  bzero(previousBuf, BUFSIZE);
  bzero(messageReceived, BUFSIZE);
  bzero(expectedAck, BUFSIZE);

  strncpy(expectedAck, "1234567890", 10);

  do {
    bzero(buf, BUFSIZE);
    int numBytesNeededToSendInThisBuf = fread(buf, sizeof(char), BUFSIZE, f);
    if(numBytesNeededToSendInThisBuf <= 0) {
      // all bytes must have been sent -- break;
      fprintf(stderr, "All Bytes have been sent\n");
      break;
    }
    
    int i = 0;
    do {
      numBytesSent = sendPacket(sockfd, clientaddr, buf);
      fprintf(stderr, "Sent message: %s\n", buf);
      fprintf(stderr, "message sent length %d\n", numBytesSent);
      usleep(sleepTime);
      setNonBlocking(sockfd);
      int numBytesReceived = receivePacket(sockfd, clientaddr, messageReceived);
      setBlocking(sockfd);

      fprintf(stderr, "Receieved ack: %s\n", messageReceived);
      fprintf(stderr, "%d\n", strncmp(messageReceived, expectedAck, BUFSIZE));

      if(numBytesReceived > 0 && strncmp(messageReceived, expectedAck, BUFSIZE) == 0) {
        break;
      }
      i++;
    } while(i < 3);
    

    if(i == 3) {
      fprintf(stderr, "Error packet timeout\n");
      break;
    }

  } while(1);

  fclose(f);
  return 0;
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

void setNonBlocking(int sock){
  int opts;

  opts = fcntl(sock,F_GETFL);
  if (opts < 0) {
      perror("fcntl(F_GETFL)");
      exit(EXIT_FAILURE);
  }
  opts = (opts | O_NONBLOCK);
  if (fcntl(sock,F_SETFL,opts) < 0) {
      perror("fcntl(F_SETFL)");
      exit(EXIT_FAILURE);
  }
  return;
}

void setBlocking(int sock) {
  int opts;

  opts = fcntl(sock,F_GETFL);
  if (opts < 0) {
      perror("fcntl(F_GETFL)");
      exit(EXIT_FAILURE);
  }
  //opts = (opts | O_NONBLOCK);
  opts = opts & (~O_NONBLOCK);
  if (fcntl(sock,F_SETFL,opts) < 0) {
      perror("fcntl(F_SETFL)");
      exit(EXIT_FAILURE);
  }
  return;
}
