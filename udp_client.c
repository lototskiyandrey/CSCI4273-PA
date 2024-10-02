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
int sendPacket(int sockfd, struct sockaddr_in serveraddr, char *buf);

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    char command[BUFSIZE], inputFile[BUFSIZE];
    int numBytes;
    int flags;

    int pS; // if programState (pS) is 0, then 

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

      // Listen for response

      // If no reponse after a certain period of time, send again

    }


    /*
    while(true) {

      // Get message from the user

      bzero(buf, BUFSIZE);
      printf("Enter a Command: ls, exit, get [file_name], put [file_name], delete [file_name], put [file_name]\n");
      printf("> ");
      fgets(buf, BUFSIZE, stdin);

      sscanf(buf, "%s %s", command, inputFile);

      printf("%s\n", inputFile);
      printf("%s\n", command);

      if(strncmp(command, "ls", 2) == 0) {

        // send the message to the server 
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
          // print the server's reply 
          bzero(buf, BUFSIZE);
          n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
          if(n < 0) {
            error("ERROR in recvfrom");
            // print the buffer sent back
            continue; // message was not recieved
          }
          
          // send an ack saying that the message has been recieved
          //printf("Message recieved from server: %s\n", buf);
          //printf("Message being sent to server: %s\n", ack);
          n = sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&serveraddr, serverlen);
          if (n < 0) {
            error("ERROR in sendto");
            continue; //
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
        // printf("Entered get command!\n");

        char receiverBuf[BUFSIZE];

        // first send the command and file name
        //char a[] = "get command recieved file no exist";

        printf("Command: %s, FileName: %s\n", command, inputFile);
        printf("%s\n", buf);

        do {
          serverlen = sizeof(serveraddr);
          bzero(receiverBuf, BUFSIZE);
          printf("sending command\n");
          n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
          if(n < 0) {
            error("Error: sendto failed\n");
            continue;
          }
          printf("receiving command\n");
          n = recvfrom(sockfd, receiverBuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
          if(n < 0) {
            error("Error: recvfrom failed\n");
          }
          printf("%s\n", receiverBuf);
        } while(strncmp(receiverBuf, "get command received", 20) != 0);

        if(strncmp(receiverBuf, "get command received file no exist", 35) == 0) {
          printf("File %s does not exist on server.\n", inputFile);
          continue; 
        }

        printf("The file does exist on server\n");

        FILE *f;
        f = fopen(inputFile, "w");

        char previouslyReceivedBuf[BUFSIZE];
        bzero(previouslyReceivedBuf, BUFSIZE);

        do {     
          bzero(receiverBuf, BUFSIZE);
          n = recvfrom(sockfd, receiverBuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
          if(n < 0) {
            error("recvfrom failed\n");
            continue;
          }
          char ack[] = "ack\0";
          n = sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr *)&serveraddr, serverlen);
          if(n < 0) {
            error("sendto filed\n");
            continue;
          }

          if(strncmp(receiverBuf, "File Transfer Complete", 22) == 0) {
            fclose(f);
            break;
          }

          // if the previously received buff is the same as the currently received buffer, 
          // likely the ack we sent was never received, so do not 
          // write the buffer into the file


          if(strncmp(previouslyReceivedBuf, receiverBuf, BUFSIZE) != 0) { 
            fwrite(receiverBuf, sizeof(char), indexOfEOFInFile(receiverBuf, BUFSIZE), f);
            //previouslyReceivedBuf = receiverBuf;
            printf("Getting the last char of receiverBuf: %d\n", receiverBuf[BUFSIZE-1]);
            memcpy(previouslyReceivedBuf, receiverBuf, BUFSIZE);
          }
        } while(1);

      }

      else if(strncmp(command, "delete", 6) == 0) {
        //printf("Entered delete command!\n");

        char receiverBuf[BUFSIZE] = "NORESP";

        do {
          serverlen = sizeof(serveraddr);
          printf("Sending delete command\n");
          n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
          if (n < 0) {
            error("ERROR in sendto");
          }

          //bzero(buf, BUFSIZE);
          printf("receiving command\n");
          n = recvfrom(sockfd, receiverBuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
          if(n < 0) {
            error("ERROR in recvfrom");
          }

          printf("%s\n", receiverBuf);

        } while(strncmp(receiverBuf, "Error: file does not exist.", 28) != 0 && strncmp(receiverBuf, "Deleted File", 13) != 0);

        printf("%s\n", receiverBuf);
      }

      else if(strncmp(command, "put", 3) == 0) {
        
        //printf("%s\n", inputFile);
        //printf("%s\n", command);

        // First send the message to the server

        // check if fileName Exists

        printf("sending buf: %s\n", buf);

        FILE *f = fopen(inputFile, "rb");
        if(f == NULL) {
          // file does not exist
          printf("Error: File %s does not exist.\n", inputFile);
          continue;
        }

        // since fileName Exists - establish connection with the server
        char sendBuf[BUFSIZE];
        char receiveBuf[BUFSIZE];

        bzero(sendBuf, BUFSIZE);
        bzero(receiveBuf, BUFSIZE);

        // strncpy(sendBuf, "Connection Established", 22);

        do {
          serverlen = sizeof(serveraddr);
          bzero(receiveBuf, BUFSIZE);
          printf("sending buf\n");
          n = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);
          if(n < 0) {
            error("sendto error\n");
            continue;
          }
          printf("receiving ack\n");
          n = recvfrom(sockfd, receiveBuf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, &serverlen);
          printf("received buf: %s\n", receiveBuf);
          if(n < 0) {
            error("recvfrom error\n");
            continue;
          }
        } while(strncmp(receiveBuf, "ack", 3) != 0);

        // with connection with server established, begin reading the contents of the file, and sending 
        // them to the server. 

        printf("Connection Established\n");

        do {
          bzero(receiveBuf, BUFSIZE);
          bzero(sendBuf, BUFSIZE);

          
          // serverlen = sizeof(serveraddr);
          // bzero(receiveBuf, BUFSIZE);
          // n = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serveraddr, serverlen);
          // if(n < 0) {
          //   error("sendto error\n");
          //   continue;
          // }
          
          // read the file in 1024 bytes chunks
          int numBytesSent = fread(sendBuf, sizeof(char), BUFSIZE, f);
          if(numBytesSent <= 0) {
            // all bytes sent
            printf("All Byte have been sent\n");
            break;
          }

          // send the chunk to the server. Await ack. If no ack, repeat.
          do {
            bzero(receiveBuf, BUFSIZE);
            // bzero(sendBuf, BUFSIZE);  
            printf("sending %s\n", sendBuf);
            n = sendto(sockfd, sendBuf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, serverlen);
            if(n < 0) {
              error("sendto error\n");
              continue;
            }
            n = recvfrom(sockfd, receiveBuf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, &serverlen);
            printf("received %s\n", receiveBuf);
            if(n < 0) {
              error("recvfrom error\n");
              continue;
            }
          } while(strncmp(receiveBuf, "ack", 3) != 0);

        } while(1);

        // Finally, with all bytes sent, announce to the server that all bytes have been sent
        // and to stop the connection.

        do {
          bzero(receiveBuf, BUFSIZE);
          char endMessage[] = "File Transfer Complete";
          n = sendto(sockfd, endMessage, strlen(endMessage), 0, (struct sockaddr*)&serveraddr, serverlen);
          if(n < 0) {
            error("sendto error\n");
            continue;
          }
          n = recvfrom(sockfd, receiveBuf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, &serverlen);
          if(n < 0) {
            error("recvfrom error\n");
            continue;
          }
        } while(strncmp(receiveBuf, "ack", 3) != 0);
        
      }

      else {
        // Unknown Command

        printf("Unknown Command\n.");
      }
    }
    */
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


int indexOfEOFInFile(char *buf, int bufSize) {

  for(int i = 0; i < bufSize; i++) {
    if(buf[i] == EOF) {
      return i;
    }
  }

  return bufSize;
}

char *getUserCommand(char *buf) {


  if(strncmp(buf, "ls ", 3) == 0) {    
    return "ls";
  }

  if(strncmp(buf, "exit ", 5) == 0) {
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
      strncpy(buf, "ERROR", 5);
      return buf;
    }
    bzero(buf, BUFSIZE);
    strncpy(buf, input, strlen(input));
    return buf;
  }

  bzero(buf, BUFSIZE);
  strncpy(buf, "No File to Send", 5);
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

  int numBytesSent = sendto(sockfd, buf, BUFSIZE, 0, &serveraddr, serverlen);

  if(numBytesSent < 0) {
    error("ERROR in sendto");
  }

  return numBytesSent;
}

int receivePacket(int sockfd, struct sockaddr_in serveraddr, char *buf) {
  int serverlen = sizeof(serveraddr);

  int numBytesReceieved = recvfrom(sockfd, buf, BUFSIZE, &serveraddr, &serverlen);

  if(numBytesReceieved < 0) {
    error("ERROR in recvfrom");
  }

  return numBytesReceieved;
}
