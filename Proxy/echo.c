#include "proxy.h"
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

// Function echos the request back
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
