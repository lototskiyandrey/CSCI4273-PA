#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <netdb.h>
#include <sys/stat.h>
#include <pthread.h>
#include <strings.h>
#include <openssl/md5.h>
#include <errno.h>

#define maxdnslength 8192
#define maxbuflength 8192


int open_listenSocket(int portNum);
void* thread(void* vargp);
void echo(int connFd);
void send_error(int connFd, char *message);
int checkForBadRequest(int connFd, char *request, char *hostName, char *version);
void md5_str(char *str, char* md5Buf);
int isBlackListed(char *hostName, char *message);
int checkDNSCache(char* hostname, struct in_addr* cacheAddr);
int checkCacheMD5(char *str);
int addIpToCache(char* hostname, char* ip);
void sendFileFromCache(int connFd, char* fileName);
