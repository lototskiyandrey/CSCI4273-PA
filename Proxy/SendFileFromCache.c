#include "proxy.h"

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