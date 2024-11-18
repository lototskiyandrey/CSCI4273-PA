#include "proxy.h"


// checks if a given hostname is in the blacklist
// return 1 if true, 0 if false
int isBlackListed(char *hostName, char *ip)
{
    FILE *f;

    char line[100];
    char *newLine;

    if(access("blacklist", F_OK) == -1)
    {
        printf("No blacklist\n");
        return 0;
    }

    f = fopen("blacklist", "r");
    while(fgets(line, sizeof(line), f))
    {
        newLine = strchr(line, '\n');
        if(newLine != NULL)
        {
            *newLine = '\0';
        }
        if(strstr(line, hostName) || strstr(line, ip))
        {
            printf("Blacklist match found: %s\n", line);
            return 1;
        }
    }
    fclose(f);
    printf("blacklist match not found\n");
    return 0;
}
