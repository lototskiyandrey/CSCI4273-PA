#include "proxy.h"

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