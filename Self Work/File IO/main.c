#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define bufsize 1024


void printCharBufInInts(char *buf, int size, char *bufName);
int zeroBuf(char *buf, int size);

int main(int argc, char **argv)
{

    if(argc != 2) 
    {
        fprintf(stderr, "Please Enter one file\n");
        exit(1);
    }

    //char buf[bufsize];
    FILE *f;
    
    f = fopen(argv[1], "r");

    char buf[bufsize];

    zeroBuf(buf, bufsize);

    if(f == NULL)
    {
        exit(1);
    }

    ssize_t bytesRead;
    while((bytesRead = fread(buf, sizeof(char), bufsize-5, f)) > 0)
    {
        fprintf(stderr, "Number of bytes read: %ld\n", bytesRead);
        printf("Buffer is: %s\n", buf);
        printCharBufInInts(buf, bufsize, "buf");
    }

    fclose(f);

    return 0;
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


