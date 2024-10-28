

#include "server.h"


int main (int argc, char *argv[])
{
   if(argc != 2)
   {
      printf("Error, wrong number of inputs.\n");
      exit(-1);
   }

   int portNum = atoi(argv[1]);

   struct config configuration;

   setupConfig(&configuration, portNum);

   int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
   if(listenSocket < 0) 
   {
      printf("Error creating socket.\n");
      exit(-1);
   }

   printf("Socket created.\n");

   int option = 1;
   setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

   struct sockaddr_in server;
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = htons(portNum);

   int bindResult = bind(listenSocket, (struct sockaddr *)&server, sizeof(server));
   if(bindResult < 0)
   {
      printf("Binding failed.\n");
      exit(-1);
   }

   int listening = listen(listenSocket,3);
   if(listening < 0)
   {
      printf("Listening failed.\n");
      exit(-1);
   }

   printf("Awaiting connections...\n");

   struct sockaddr_in client;
   unsigned long clientLen = sizeof(struct sockaddr_in);
   char receivedMessage[bufsize];
   char requestMethod[bufsize];
   char requestURL[bufsize];
   char requestVersion[bufsize];

   while(1)
   {
      int clientSocket = accept(listenSocket, (struct sockaddr *)&client, (socklen_t *)&clientLen);
      if(clientSocket < 0) 
      {
         printf("Accept failed.\n");
         return 1;
      }
      printf("Connection established.\n");

      int pid;
      if((pid = fork()) == 0)
      {
         close(listenSocket);

         int numBytesReceived;
         zeroBuf(receivedMessage, bufsize);
         while((numBytesReceived = recv(clientSocket, receivedMessage, bufsize, 0)) > 0)
         {
            zeroBuf(requestMethod, bufsize);
            zeroBuf(requestURL, bufsize);
            zeroBuf(requestVersion, bufsize);

            printf("Client Socket: %d.\n", clientSocket);
            printf("Message: %s\n", receivedMessage);

            sscanf(receivedMessage, "%s %s %s", requestMethod, requestURL, requestVersion);

            int returnNum = responseHandler(clientSocket, 0, requestMethod, requestURL, requestVersion);

            if(returnNum == 0)
            {
               processRequest(clientSocket, requestMethod, requestURL, requestVersion, &configuration);
            }
            else
            {
               printf("Client - %d terminated.\n", clientSocket);
               close(clientSocket);
               exit(0);
            }
            zeroBuf(receivedMessage, bufsize);
         }

         if(numBytesReceived == 0)
         {
            printf("Client disconnected %d.\n", clientSocket);
         }
         else if(numBytesReceived < 0)
         {
            printf("Receive Function failed.\n");
            exit(-1);
         }

         close(clientSocket);
         exit(0);
      }
   }

   printf("Closing server.\n");
   close(listenSocket);

   return 0;
}

void zeroBuf(char *buf, int size)
{
   memset(buf, 0, size*sizeof(char));
}

void setupConfig(struct config *configuration, int port) 
{

   configuration->port = port;

   strcpy(configuration->root, "./www");

   strcpy(configuration->index, "index.html index.htm index.ws");

   strcpy(configuration->contentType, ".html text/html\n.htm text/html\n.txt text/plain\n.png image/png\n.gif image/gif\n.jpg image/jpg\n.css text/css\n.js  text/javascript\n.ico image/x-icon");
}

int getFileExtension( char *buffer, char* fileName, struct config *configuration)
{
   char contentType[bufsize];

   //Find the last occurence in the string
   char *dot = strrchr(fileName, '.');
   if(dot == NULL || dot == fileName)
   {  
      // strrchar returns NULL if the requested char does not exist
      // So if there is no . in the requested file, we will not display the file.
      // Also, we will not display the file if the file is simply "." 
      return 0;
   }
   
   // copy configuration->contentType into temporary buffer contentType for reading.
   snprintf(contentType, bufsize, "%s", configuration->contentType);

   char *token = strtok(contentType, " \n");

   printf("Token is: %s\n", token);

   if(strcmp(dot, ".html") == 0)
   {
      // Now get the next token
      token = strtok(NULL, " \n");
      printf("Token is: %s\n", token);
   }
   else
   {
      // Since the default has not been found, scan the remaining tokens.
      while(strcmp(dot, token) != 0)
      {
         token = strtok(NULL, " \n");
         printf("Token is: %s\n", token);
      }

      // tokenize one more time to get the content type
      token = strtok(NULL, " \n");
      printf("Token is: %s\n", token);
   }

   snprintf(buffer, bufsize, "%s", token);

   return 1;
}


void generate501(int clientSocket, char *responseHeader, char *responseContent, char *requestMethod) 
{   
   int length = 0;
   snprintf(responseContent, bufsize, "<html><body>501 Not Implemented Request Method: %s</body></html>", requestMethod);
   length += snprintf(responseHeader, bufsize, "HTTP/1.1 501 Not Implemented\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Type: text/html\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Length: %lu\r\n\r\n", strlen(responseContent));
   send(clientSocket, responseHeader, strlen(responseHeader), 0);
   write(clientSocket, responseContent, strlen(responseContent));
}


void generate400(int clientSocket, char *responseHeader, char *responseContent, char *requestVersion)
{
   
   int length = 0;

   snprintf(responseContent, bufsize, "<html><body>400 Bad Request Reason: Invalid Version:%s</body></html>", requestVersion);

   length += snprintf(responseHeader, bufsize, "HTTP/1.1 400 Bad Request\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Type: text/html\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Length: %lu\r\n\r\n", strlen(responseContent));

   send(clientSocket, responseHeader, strlen(responseHeader), 0);
   write(clientSocket, responseContent, strlen(responseContent));
}

void generate404(int clientSocket, char *responseHeader, char *responseContent, char *requestURL)
{
   int length = 0;
   snprintf(responseContent, bufsize, "<html><body>404 Not Found: Reason URL does not exist: %s</body></html>", requestURL);
   length += snprintf(responseHeader, bufsize, "HTTP/1.1 404 Not Found\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Type: text/html\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Length: %lu\r\n\r\n", strlen(responseContent));
   send(clientSocket, responseHeader, strlen(responseHeader), 0);
   write(clientSocket, responseContent, strlen(responseContent));
}

void generate500(int clientSocket, char *responseHeader, char *responseContent)
{
   snprintf(responseContent, bufsize, "<html><body>500 Internal Server Error: Cannot allocate memory</body></html>");
   int length = 0;
   length += snprintf(responseHeader, bufsize, "HTTP/1.1 500 Internal Server Error\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Type: text/html\r\n");
   length += snprintf(responseHeader+length, bufsize-length, "Content-Length: %lu\r\n\r\n", strlen(responseContent));
   send(clientSocket, responseHeader, strlen(responseHeader), 0);
   write(clientSocket, responseContent, strlen(responseContent));
}

//Check for valid requests and output errors
int responseHandler(int clientSocket, int statusCode, char* requestMethod, char* requestURL, char* requestVersion)
{
   char responseHeader[bufsize];
   char responseContent[bufsize];
   

   //Clear the buffers before use
   zeroBuf(responseHeader, bufsize);
   zeroBuf(responseContent, bufsize);

   //we don't have a specific error to print
   if(statusCode == 0)
   {
      //501 Error: The request methods don't exist.
      if( !(strcmp(requestMethod, "GET") == 0) )
      {
         generate501(clientSocket, responseHeader, responseContent, requestMethod);
         return -1;
      }
      // 400 Error: The request version is wrong.
      if (strcmp(requestVersion, "HTTP/1.1") != 0 && strcmp(requestVersion, "HTTP/1.0") != 0)
      {
         generate400(clientSocket, responseHeader, responseContent, requestVersion);
         return -1;
      }
   }
   
   //For specific errors
   switch(statusCode)
   {
      //404 Error: File not found.
      case 404:
         generate404(clientSocket, responseHeader, responseContent, requestURL);
         return -1;
         break;
      //500 Error: Internal Server Error.
      case 500:
         generate500(clientSocket, responseHeader, responseContent);
         return -1;
         break;
      // 501 Error: The request Methods dont exist.
      case 501:
         generate501(clientSocket, responseHeader, responseContent, requestMethod);
         return -1;
         break;
   }
   

   return 0;
}

//Handles client requests
void processRequest(int clientSocket, char* requestMethod, char* requestURL, char* requestVersion, struct config *configuration)
{
   FILE *f;

   char filePath[bufsize];
   zeroBuf(filePath, bufsize);
   if(strcmp(requestURL, "/") == 0 || strcmp(requestURL, "/index.html") == 0 || strcmp(requestURL, "/index.htm") == 0 || strcmp(requestURL, "/index.ws") == 0)
   {
      // We want the default path that routes to index.html
      snprintf(filePath, bufsize, "%s%s", configuration->root, "/index.html");
      // Change the document root to "/index.html"
      snprintf(requestURL, bufsize, "%s", "/index.html");
   }
   else
   {
      // Otherwise create the path to find the root
      snprintf(filePath, bufsize, "%s%s", configuration->root, requestURL);
   }

   // Opening the file
   f = fopen(filePath, "r");

   if(f == NULL)
   {
      // File must not have been found --> Send a file not found error
      printf("File was not found.\n");
      responseHandler(clientSocket, 404, requestMethod, requestURL, requestVersion);
      return;
   }

   // response Handler

   char fileExtension[bufsize];
   zeroBuf(fileExtension, bufsize);
   int returnedFileExtension = 1;
   printf("Getting file extension.\n");
   returnedFileExtension = getFileExtension(fileExtension, requestURL, configuration);

   printf("File Extension is: %s\n", fileExtension);

   if(returnedFileExtension == 0)
   {
      // File Extension was invalid, so return an error response, and break
      responseHandler(clientSocket, 404, requestMethod, requestURL, requestVersion);
      return;
   }

   fseek(f, 0, SEEK_END);
   int fileSize = (int)ftell(f);
   rewind(f);

   char contentHeader[bufsize];

   generateHTTPHeader(contentHeader, fileExtension, fileSize);

   send(clientSocket, contentHeader, strlen(contentHeader), 0);

   char fileBuffer[bufsize];
   size_t bytesRead = fread(fileBuffer, sizeof(char), bufsize, f);
   while(bytesRead > 0 || !feof(f))
   {
      write(clientSocket, fileBuffer, (int)bytesRead);
      zeroBuf(fileBuffer, bufsize);
      bytesRead = fread(fileBuffer, sizeof(char), bufsize, f);
   }
   return;
}

void generateHTTPHeader(char *contentHeader, char *fileExtension, int fileSize)
{  
   int contentLength = 0;
   contentLength = snprintf(contentHeader, bufsize, "HTTP/1.1 200 OK\r\n");
   contentLength += snprintf(contentHeader + contentLength, bufsize - contentLength, "Content-Type: %s\r\n", fileExtension);
   contentLength += snprintf(contentHeader + contentLength, bufsize - contentLength, "Content-Length: %d\r\n", fileSize);
   contentLength += snprintf(contentHeader + contentLength, bufsize - contentLength, "Connection: Keep-Alive\r\n\r\n");
}
