
#include "proxy.h"

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