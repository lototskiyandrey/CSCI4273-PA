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
        printf("Buffer is: %s\n", buf);
        printCharBufInInts(buf, bufsize, "buf");
        (void)fwrite(buf, sizeof(char), (int)bytesRead, f2);
        zeroBuf(buf, bufsize);
    }

    fclose(f1);
    fclose(f2);

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


