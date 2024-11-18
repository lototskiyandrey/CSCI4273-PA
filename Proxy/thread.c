#include "proxy.h"

void* thread(void* vargp)
{
    int connFd = *((int *) vargp);
    pthread_detach(pthread_self());
    free(vargp);
    echo(connFd);
    close(connFd);
    return NULL;
}