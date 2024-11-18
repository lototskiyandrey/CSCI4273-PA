
#include "proxy.h"



int main(int argc, char **argv)
{   
    
    if(argc != 2 || argc != 3)
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