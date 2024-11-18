#include "proxy.h"

int addIpToCache(char* hostName, char* ip)
{
    pthread_mutex_lock(&dns_lock);
    char *enter = strchr(dnsCache, '\n');
    char buf[100];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, 100, "%s:%s\n", hostName, ip);

    if(enter == NULL)
    {
        printf("Cache is empty\n");
        strncpy(dnsCache, buf, strlen(buf));
        pthread_mutex_unlock(&dns_lock);
        return 0;
    }

    if(enter + strlen(buf) + 1 > dnsCache + sizeof(dnsCache))
    {
        return -1;
        pthread_mutex_unlock(&dns_lock);
    }

    strncpy(enter + 1, buf, strlen(buf));
    pthread_mutex_unlock(&dns_lock);
}