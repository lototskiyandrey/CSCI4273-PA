
#include "proxy.h"


pthread_mutex_t dns_lock;
pthread_mutex_t cache_lock;

int timeout = 0;
char dnsCache[maxdnslength];


int main(int argc, char **argv)
{   
    
    if(argc != 2 && argc != 3)
    {
        fprintf(stderr, "Wrong number of inputs.\n");
        exit(0);
    }

    int portNum;
    // no specified timeout
    if(argc == 2)
    {
        portNum = atoi(argv[1]);
    }

    // timeout specified
    if(argc == 3)
    {
        timeout = atoi(argv[2]);
    }


    int *connfdp;
    int clientLen = sizeof(struct sockaddr_in);

    struct sockaddr_in clientAddr;
    pthread_t threadId;

    memset(dnsCache, 0, sizeof(dnsCache));
    
    int listenSocket = open_listenSocket(portNum);

    if(pthread_mutex_init(&dns_lock, NULL) != 0)
    {
        fprintf(stderr, "Cannot initialize mutex dns_lock.\n");
        return -1;
    }

    if(pthread_mutex_init(&cache_lock, NULL) != 0)
    {
        fprintf(stderr, "Cannot initialize mutex cache_lock.\n");
        return -1;
    }

    while(1)
    {
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientLen);
        pthread_create(&threadId, NULL, thread, connfdp);
    }

    return 0;
}






int addIpToCache(char* hostName, char* ip)
{
    pthread_mutex_lock(&dns_lock);
    char *enter = strchr(dnsCache, '\n');
    char buf[100];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, 100, "%s:%s\n", hostName, ip);

    if(enter == NULL)
    {
        printf("Cache is empty\n");
        strncpy(dnsCache, buf, strlen(buf));
        pthread_mutex_unlock(&dns_lock);
        return 0;
    }

    if(enter + strlen(buf) + 1 > dnsCache + sizeof(dnsCache))
    {
        return -1;
        pthread_mutex_unlock(&dns_lock);
    }

    strncpy(enter + 1, buf, strlen(buf));
    pthread_mutex_unlock(&dns_lock);
}




void sendFileFromCache(int connFd, char* fileName)
{
    FILE *f = fopen(fileName, "rb");

    if(!f)
    {
        printf("Error opening file %s\n", fileName);
        return;
    }

    fseek(f, 0L, SEEK_END);
    int fileSize = ftell(f);
    rewind(f);
    char fileBuf[fileSize];
    fread(fileBuf, 1, fileSize, f);
    write(connFd, fileBuf, fileSize);
}






// this function opens and returns a listening socket on port portNum
int open_listenSocket(int portNum)
{
    int listenSocket;
    int optVal = 1;
    struct sockaddr_in serverAddr;

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket < 0)
    {
        return -1;
    }

    int sockOpt = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optVal, sizeof(int));
    if(sockOpt < 0)
    {
        return -1;
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons((unsigned short) portNum);

    int bindOpt = bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(bindOpt < 0)
    {
        return -1;
    }

    const int maxNumQueueConnections = 1024;
    int listenOpt = listen(listenSocket, maxNumQueueConnections);
    if(listenOpt < 0)
    {
        return -1;
    }

    return listenSocket;

}





void echo(int connFd)
{

    char receivedBuf[maxdnslength];
    char fileName[maxbuflength];
    size_t n = read(connFd, receivedBuf, maxdnslength);
    printf("Proxy received a request.\n");

    // type of htt request
    char *request = strtok(receivedBuf, " ");
    // name of host
    char *hostName = strtok(NULL, " ");
    // http version 1 or 1.1
    char* version = strtok(NULL, "\r");

    printf("Request type: %s.\nHostname: %s.\nVersion: %s.\n", request, hostName, version);

    int requestStatus = checkForBadRequest(connFd, request, hostName, version);
    if(requestStatus != 0)
    {
        return;
    }

    char *doubleSlash = strstr(hostName, "//");
    if(doubleSlash != NULL)
    {
        hostName = doubleSlash + 2;
    }

    char *slash = strchr(hostName, '/');
    // no spefic file has been requested
    if(slash == NULL || *(slash + 1) == '\0')
    {
        printf("Default page requested.\n");
        strcpy(fileName, "index.html");
    }
    else
    {
        strcpy(fileName, slash+1);
    }

    printf("Host is: %s\nFile is: %s\n", hostName, fileName);

    if(slash != NULL)
    {
        *slash = '\0';
    }



    // give two extra spots for the / between host and file name and \0 terminator
    char md5Input[strlen(hostName) + strlen(fileName) + 2];
    strcpy(md5Input, hostName);
    strcat(md5Input, "/");
    strcat(md5Input, fileName);



    char md5Output[33];
    memset(md5Output, 0, sizeof(md5Output));
    printf("Hashing input %s\n", md5Input);
    md5_str(md5Input, md5Output);


    char cachedBuf[strlen("cache/") + strlen(md5Output)];
    strcpy(cachedBuf, "cache/");
    strcat(cachedBuf, md5Output);

    if(checkCacheMD5(md5Output)) 
    {
        printf("Cache matches, sending file: %s\n", cachedBuf);
        sendFileFromCache(connFd, cachedBuf);
        return;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0)
    {
        printf("Error opening socket");
    }

    struct sockaddr_in serverAddr;
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    const int portNum = 80; // Default HTTP port
    serverAddr.sin_port = htons(portNum);

    struct in_addr cacheAddr;
    int serverAddrCache = checkDNSCache(hostName, &cacheAddr);

    struct hostent *server;
    if(serverAddrCache == -1)
    {
        printf("Host %s is ont in the cache\n", hostName);
        server = gethostbyname(hostName);
        if(server == NULL)
        {
            // Host cannot be resolved
            printf("Unable to resolve host %s\n", hostName);
            send_error(connFd, "404 Not Found");
            return;
        }
        bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    }
    else 
    {
        // host is in the cache
        serverAddr.sin_addr.s_addr = cacheAddr.s_addr;
    }


    char ipBuf[20];
    if(inet_ntop(AF_INET, (char *)&serverAddr.sin_addr.s_addr, ipBuf, (socklen_t)20) == NULL)
    {
        printf("Error: failed conversion from hostname to ip\n");
        return;
    }

    if(serverAddrCache == -1)
    {
        int success = addIpToCache(hostName, ipBuf);
        if(success == -1)
        {
            printf("Cache must be full, cannot add %s:%s\n", hostName, ipBuf);
        }
    }

    printf("Checking blacklist for %s:%s\n", hostName, ipBuf);
    if(isBlackListed(hostName, ipBuf) == 1)
    {
        send_error(connFd, "403 Forbidden");
        return;
    }


    int serverLen = sizeof(serverAddr);

    int lengthWriting = connect(sock_fd, (struct sockaddr *)&serverAddr, serverLen);
    if(lengthWriting < 0)
    {
        printf("connect() failed\n");
        return;
    }

    char sendBuf[maxbuflength];
    memset(sendBuf, 0, maxbuflength);
    sprintf(sendBuf, "GET /%s %s\r\nHost: %s\r\n\r\n", fileName, version, hostName);

    lengthWriting = write(sock_fd, sendBuf, sizeof(sendBuf));
    if(lengthWriting < 0)
    {
        printf("sendto failed\n");
        return;
    }

    int totalLengthSent = 0;
    memset(receivedBuf, 0, sizeof(receivedBuf));

    FILE *f = fopen(cachedBuf, "wb");
    pthread_mutex_lock(&cache_lock);
    int sending;
    while((sending = read(sock_fd, receivedBuf, sizeof(receivedBuf))) > 0)
    {
        if(lengthWriting < 0)
        {
            printf("read() failed\n");
            return;
        }

        totalLengthSent += sending;

        write(connFd, receivedBuf, sending);
        fwrite(receivedBuf, 1, sending, f);
        memset(receivedBuf, 0, sizeof(receivedBuf));
    }

    pthread_mutex_unlock(&cache_lock);
    fclose(f);
    if(sending == -1)
    {
        printf("read() failed -- errorNum: %d\n", errno);
        return;
    }
    printf("Received a total of %d bytes\n", totalLengthSent);

}






void md5_str(char *str, char* md5Buf)
{
    const int md5Len = 16;
    unsigned char md5sum[md5Len];
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, str, strlen(str));
    MD5_Final(md5sum, &context);

    for(int i = 0; i < md5Len; i++)
    {
        sprintf(md5Buf + i * 2, "%02x", (unsigned int)md5sum[i]);
    }

    printf("Hash is: %s -> %s\n", str, md5Buf);
}






int checkCacheMD5(char *str)
{
    struct stat fileStat;
    DIR *dir = opendir("./cache");

    char buf[strlen("cache/") + strlen(str)];
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "cache/");
    strcat(buf, str);
    printf("looking for file %s\n", buf);


    if(dir)
    {
        closedir(dir);
        if(stat(buf, &fileStat) != 0)
        {
            printf("File not cached\n");
            return 0;
        }

        printf("File in cache, check timeout\n");
        if(timeout == 0)
        {
            printf("No timeout set\n");
            return 1;
        }
        time_t fileModify = fileStat.st_mtime;
        time_t currentTime = time(NULL);
        double diffTime = difftime(currentTime, fileModify);
        if(diffTime > timeout)
        {
            printf("Timeout occurred. File was modified %.2f seconds ago, and timeout is %d\n", diffTime, timeout);
            return 0;
        }
        printf("File is valid for %d seconds\n", timeout - (int)diffTime);
        return 1;
    }
    else if(errno = ENONET)
    {
        printf("Cache folder doesn't exit.\n");
        mkdir("cache", 0777);
        printf("Cache directory created\n");
        closedir(dir);
        return 0;
    }
    else
    {   
        printf("Error opening cached folder\n");
        return 0;
    }
}







int checkForBadRequest(int connFd, char *request, char *hostName, char *version)
{

    if(hostName == NULL || version == NULL)
    {
        printf("Hostname or version is NULL\n");
        send_error(connFd, "500 Internal Server Error");
        return 500;
    }

    if(strlen(hostName) == 0)
    {
        printf("No host requested\n");
        send_error(connFd, "500 Internal Server Error");
        return 500;
    }

    if(strcmp(version, "HTTP/1.1") != 0 && strcmp(version, "HTTP/1.0") != 0)
    {
        printf("Invalid HTTP version: %s", version);
        send_error(connFd, "500 Invalid HTTP request");
        return 500;
    }

    if(strcmp(request, "GET") != 0)
    {
        printf("Invalid HTTP request: %s\n", request);
        send_error(connFd, "400 Bad Request");
        return 400;
    }


    return 0;
}






void send_error(int connFd, char *message)
{
    char errorMessageToSend[maxbuflength];
    sprintf(errorMessageToSend, "HTTP/1.1 %s\r\nContent-Type:text/plain\r\nContent-Length:0\r\n\r\n", message);
    write(connFd, errorMessageToSend, strlen(errorMessageToSend));
}






// checks if a given hostname is in the blacklist
// return 1 if true, 0 if false
int isBlackListed(char *hostName, char *ip)
{
    FILE *f;

    char line[100];
    char *newLine;

    if(access("blacklist", F_OK) == -1)
    {
        printf("No blacklist\n");
        return 0;
    }

    f = fopen("blacklist", "r");
    while(fgets(line, sizeof(line), f))
    {
        newLine = strchr(line, '\n');
        if(newLine != NULL)
        {
            *newLine = '\0';
        }
        if(strstr(line, hostName) || strstr(line, ip))
        {
            printf("Blacklist match found: %s\n", line);
            return 1;
        }
    }
    fclose(f);
    printf("blacklist match not found\n");
    return 0;
}



void* thread(void* vargp)
{
    int connFd = *((int *) vargp);
    pthread_detach(pthread_self());
    free(vargp);
    echo(connFd);
    close(connFd);
    return NULL;
}


int checkDNSCache(char *hostName, struct in_addr *cachedAddr)
{
    printf("Checking for %s in cache\n", hostName);

    char *line;
    char *tempBuf = calloc(strlen(dnsCache) + 1, sizeof(char));
    strcpy(tempBuf, dnsCache);

    char *match = strstr(tempBuf, hostName);
    if(match == NULL)
    {
        return -1;
    }

    line = strtok(match, ":");
    line = strtok(NULL, "\n");
    printf("Found DNS Entry %s:%s\n", hostName, line);
    inet_pton(AF_INET, line, cachedAddr);
    free(tempBuf);
}