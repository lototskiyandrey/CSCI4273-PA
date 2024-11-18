#include "proxy.h"

int checkCacheMD5(char *str)
{
    struct stat fileStat;
    DIR *dir = opendir("./cache");

    char buf[strlen("cache/") + strlen(str)];
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "cache/");
    strcat(buf, str);
    printf("looking for file %s\n", buf);


    if(dir)
    {
        closedir(dir);
        if(stat(buf, &fileStat) != 0)
        {
            printf("File not cached\n");
            return 0;
        }

        printf("File in cache, check timeout\n");
        if(timeout == 0)
        {
            printf("No timeout set\n");
            return 1;
        }
        time_t fileModify = fileStat.st_mtime;
        time_t currentTime = time(NULL);
        double diffTime = difftime(currentTime, fileModify);
        if(diffTime > timeout)
        {
            printf("Timeout occurred. File was modified %.2f seconds ago, and timeout is %d\n", diffTime, timeout);
            return 0;
        }
        printf("File is valid for %d seconds\n", timeout - (int)diffTime);
        return 1;
    }
    else if(errno = ENONET)
    {
        printf("Cache folder doesn't exit.\n");
        mkdir("cache", 0777);
        printf("Cache directory created\n");
        closedir(dir);
        return 0;
    }
    else
    {   
        printf("Error opening cached folder\n");
        return 0;
    }
}