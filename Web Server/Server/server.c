// TCP Server program


#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void error(char *errorMsg);
void errorAndExit(char *errorMsg);

int main (int argc, char **argv)
{

   if(argc != 2)
   {
      errorAndExit("Wrong number of program inputs.");
   }


   int listenSocket = socket(AF_INET6, SOCK_STREAM, 0);
   if(listenSocket < 0)
   {
      errorAndExit("socket() failed.");
   }

   int on = 1;
   int rc = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
   if(rc < 0)
   {
      error("setsockopt() failed.");
      close(listenSocket);
      exit(-1);
   }

   rc = ioctl(listenSocket, FIONBIO, (char *)&on);
   if(rc < 0)
   {
      error("ioctl() failed.");
      close(listenSocket);
      exit(-1);
   }

   struct sockaddr_in6 addr;
   int portNumber = atoi(argv[1]);

   memset(&addr, 0, sizeof(addr));
   addr.sin6_family = AF_INET6;
   memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
   addr.sin6_port = htons(portNumber);
   rc = bind(listenSocket, (struct sockaddr *)&addr, sizeof(addr));
   if(rc < 0)
   {
      error("bind() failed.");
      close(listenSocket);
      exit(-1);
   }

   rc = listen(listenSocket, 32);
   if(rc < 0)
   {
      error("listen() failed.");
      close(listenSocket);
      exit(-1);
   }


   struct pollfd fds[200];
   int nfds = 1;
   memset(fds, 0, sizeof(fds));

   fds[0].fd = listenSocket;
   fds[0].events = POLLIN;
   int timeout = 3 * 60 * 1000;

   int endServer = 0;
   while(endServer == 0)
   {
      printf("Waiting on poll()...\n");
      rc = poll(fds, nfds, timeout);
      if(rc < 0)
      {
         error("  poll() failed");
         break;
      }

      if(rc == 0)
      {
         printf("  poll() timed out. Ending program.\n");
         break;
      }

      int currentSize = nfds;
      for(int i = 0; i < currentSize; i++)
      {
         if(fds[i].revents == 0)
         {
            continue;
         }

         if(fds[i].revents != POLLIN)
         {
            printf("    Error! revents = %d.\n", fds[i].revents);
            endServer = 1;
            break;
         }

         if(fds[i].fd == listenSocket)
         {
            printf("    Listening socket is readable.\n");
            int newSd;
            while((newSd = accept(listenSocket, NULL, NULL)) != -1)
            {
               if(newSd < 0)
               {
                  if(errno != EWOULDBLOCK)
                  {
                     error("  accept() failed.");
                     endServer = 1;
                  }
                  break;
               }
            }
         }
      }
   }


   return 0;
}


void error(char *errorMsg)
{
   fprintf(stderr, "%s\n", errorMsg);
   // exit(-1);
}

void errorAndExit(char *errorMsg)
{
   error(errorMsg);
   exit(-1);
}