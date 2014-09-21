/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN 1029

typedef struct request {
   uint16_t length;
   uint16_t id;
   uint8_t op;
   char* message;
} request_t;

typedef struct response {
   uint16_t length;
   uint16_t id;
   char message[MAXBUFLEN];
} response_t;

// From Dr. Biaz's code
void displayBuffer(char *Buffer, int length){
   int currentByte, column;

   currentByte = 0;
   printf("\n>>>>>>>>>>>> Content in hexadecimal <<<<<<<<<<<\n");
   while (currentByte < length){
      printf("%3d: ", currentByte);
      column =0;
      while ((currentByte < length) && (column < 10)){
         printf("%2x ",Buffer[currentByte]);
         column++;
         currentByte++;
      }
      printf("\n");
   }
   printf("\n\n");
}

int isVowel(char c)
{
   switch(c) {
      case 'a':
      case 'A':
      case 'e':
      case 'E':
      case 'i':
      case 'I':
      case 'o':
      case 'O':
      case 'u':
      case 'U':
         return 1;
      default:
         return 0;
   }
}

int numVowels(char* str, int len) {
   int i, count = 0;

   for (i = 0; i < len; i++) {
      if(isVowel(str[i])) {
         count++;
      }

      // if (str[i] == '\0') break;
   }

   return count;
}

void disemvowel(char* source, char* dest, int len) {
   int i,j = 0;

   for (i = 0; i < len; i++) {
      if(!isVowel(source[i])) {
         dest[j++] = source[i];
      }

      // if (str[i] == '\0') break;
   }
   dest[j] = '\0';
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET) {
      return &(((struct sockaddr_in*)sa)->sin_addr);
   }

   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[])
{
   int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   int numbytesRecv, numbytesSent;
   struct sockaddr_storage their_addr;
   char buf[MAXBUFLEN];
   socklen_t addr_len;
   char their_addr_str[INET6_ADDRSTRLEN];
   response_t response;
   request_t request;

   if (argc != 2)
      exit(1);
   // else
      // myPort = atoi(argv[1]);

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_PASSIVE; // use my IP

   if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and bind to the first we can
   for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
         perror("listener: socket");
         continue;
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
         close(sockfd);
         perror("listener: bind");
         continue;
      }

      break;
   }

   if (p == NULL) {
      fprintf(stderr, "listener: failed to bind socket\n");
      return 2;
   }

   freeaddrinfo(servinfo);

   while(1) {
      printf("listener: waiting to recvfrom...\n");

      addr_len = sizeof their_addr;
      if ((numbytesRecv = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
         (struct sockaddr *)&their_addr, &addr_len)) ==-1)           {
         perror("recvfrom");
         exit(1);
      }

      printf("listener: got packet from %s\n",
         inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            their_addr_str, sizeof their_addr_str));


      request.length = (buf[0] << 8) | buf[1];
      request.id = (buf[2] << 8) | buf[3];
      request.op = buf[4];
      // printf("length is: %d, id is: %d, op is: %d", request.length, request.id, request.op);
      // printf(", message is: ");
      // for (count = 5; count < numbytesRecv; count++) {
      //    printf("%c", buf[count]);
      // }
      // printf("\n");
      // displayBuffer((char*) &request, request.length);

      if (request.op == 0x55) {
         sprintf(response.message, "%d vowels", numVowels(&buf[5], request.length-5));
      } else if (request.op = 0xAA) {
         disemvowel(&buf[5], response.message, request.length-5);
      }

      response.length = strlen(response.message) + 5;
      response.id = request.id;
      printf("message: %s", response.message);

      // displayBuffer((char*) &response, response.length);
      if ((numbytesSent = sendto(sockfd, (char*) &response, response.length, 0,
         (struct sockaddr *)&their_addr, addr_len))==-1) {
         perror("serverUDP: sendto");
         exit(1);
      }
   }
   close(sockfd);

   return 0;
}
