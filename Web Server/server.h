
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>

#define bufsize 2048
#define true 1
#define false 0
#define SECONDSINMILS 1000


struct config
{
    int port;
    char root[bufsize];
    char index[bufsize];
    char contentType[bufsize];
};

void zeroBuf(char *buf, int size);
void setupConfig(struct config *configuration, int port);
int responseHandler(int client_sock, int statusCode, char* requestMethod, char* requestURI, char* requestVersion);
void processRequest(int clientSocket, char* requestMethod, char* requestURL, char* requestVersion, struct config *configuration, int keepAlive);
void generateHTTPHeader(char *contentHeader, char *fileExtension, int fileSize, char *requestVersion, int keepAlive);
int getFileExtension( char *buffer, char* fileName, struct config *configuration);