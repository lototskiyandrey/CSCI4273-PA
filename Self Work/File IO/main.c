#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define bufsize 1024


void printCharBufInInts(char *buf, int size, char *bufName);
int zeroBuf(char *buf, int size);
void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert);
int numBytesToReadInBuf(char *buf, int size);

int main(int argc, char **argv)
{

    if(argc != 2) 
    {
        fprintf(stderr, "Please Enter one file\n");
        exit(1);
    }

    //char buf[bufsize];
    FILE *f1, *f2;
    
    f1 = fopen(argv[1], "r"); 

    char buf[bufsize];

    zeroBuf(buf, bufsize);

    strcpy(buf, "Copy of ");
    strcat(buf, argv[1]);

    f2 = fopen(buf, "w");

    if(f1 == NULL)
    {
        fprintf(stderr, "read opened file is null\n");
        exit(1);
    }

    if(f2 == NULL) 
    {   
        fprintf(stderr, "written opened file is null\n");
        fclose(f1);
        exit(1);
    }

    zeroBuf(buf, bufsize);

    ssize_t bytesRead;
    while((bytesRead = fread(buf, sizeof(char), bufsize-5, f1)) > 0)
    {
        fprintf(stderr, "Number of bytes read: %ld\n", bytesRead);
        //printf("Buffer is: %s\n", buf);
        //printCharBufInInts(buf, bufsize, "buf");
        numBytesReadToStringInBuf(buf, bufsize, (int)bytesRead);
        printf("Buffer is: %s\n", buf);
        int numBytesInBuf = numBytesToReadInBuf(buf, bufsize);
        printCharBufInInts(buf, bufsize, "buf");
        printf("numBytes in buffer: %d\n", numBytesInBuf);
        (void)fwrite(buf, sizeof(char), numBytesInBuf, f2);
        zeroBuf(buf, bufsize);
    }

    fclose(f1);
    fclose(f2);

    return 0;
}

int numBytesToReadInBuf(char *buf, int size)
{
    char numAsString[5];

    numAsString[0] = buf[(size-1) - 4];
    numAsString[1] = buf[(size-1) - 3];
    numAsString[2] = buf[(size-1) - 2];
    numAsString[3] = buf[(size-1) - 1];
    numAsString[4] = buf[(size-1) - 0];   

    return atoi(numAsString);
}

void numBytesReadToStringInBuf(char *buf, int size, int numBytesToInsert)
{
    char bytesToInsert[5];
    zeroBuf(bytesToInsert, 5);
    sprintf(bytesToInsert, "%d", numBytesToInsert);
    //strcat(buf, bytesToInsert);

    buf[(size-1)-4] = bytesToInsert[0];
    buf[(size-1)-3] = bytesToInsert[1];
    buf[(size-1)-2] = bytesToInsert[2];
    buf[(size-1)-1] = bytesToInsert[3];
    buf[(size-1)-0] = bytesToInsert[4];
}

void printCharBufInInts(char *buf, int size, char *bufName)
{   
    (void)buf;
    (void)size;
    (void)bufName;
    fprintf(stderr, "Printing buffer %s as ints\n", bufName);

    for(int i = 0; i < size; i++)
    {
        fprintf(stderr, "%d", buf[i]);
    }
    fprintf(stderr, "\n");
}

int zeroBuf(char *buf, int size)
{
    if(size <= 0) {
        return -1;
    }

    memset(buf, 0, size*sizeof(char));

    return size;
}


