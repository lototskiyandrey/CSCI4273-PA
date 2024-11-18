#include "proxy.h"

void send_error(int connFd, char *message)
{
    char errorMessageToSend[maxbuflength];
    sprintf(errorMessageToSend, "HTTP/1.1 %s\r\nContent-Type:text/plain\r\nContent-Length:0\r\n\r\n", message);
    write(connFd, errorMessageToSend, strlen(errorMessageToSend));
}