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

typedef struct dvResponse {
   uint16_t length;
   uint16_t id;
   char* message;
} dvResponse_t;

typedef struct nvResponse {
   uint16_t length;
   uint16_t id;
   uint16_t numV;
} nvResponse_t;

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

uint16_t numVowels(char* str, int len) {
   int i = 0; 
	uint16_t count = 0;

   for (i = 0; i < len; i++) {
      if(isVowel(str[i])) {
         count++;
      }

   }

   return count;
}

void disemvowel(char* source, char* dest, int len) {
   int i,j = 0;

   for (i = 0; i < len; i++) {
      if(!isVowel(source[i])) {
         dest[j++] = source[i];
      }

   }
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
   int packetSize = 0;
   int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   int numbytesRecv, numbytesSent;
   struct sockaddr_storage their_addr;
   char buf[MAXBUFLEN];
   socklen_t addr_len;
   char their_addr_str[INET6_ADDRSTRLEN];
   request_t request;
   char* sendData = NULL;
   char message[MAXBUFLEN];


   if (argc != 2)
      exit(1);

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

      if (request.op == 0x55) {
			printf("this is working");
			nvResponse_t response;
         response.length = 6;
			packetSize = 6;
			response.id = request.id;
			response.numV = numVowels(&buf[5], request.length-5);
			sendData = (char*) &response;
			
      } else if (request.op = 0xAA) {
			dvResponse_t response;
         disemvowel(&buf[5], message, request.length-5);
			response.length = 4 + strlen(message);
			packetSize = response.length;
			response.id = request.id;
			response.message = message;
			sendData = (char*) &response;
      }
			printf("length %u %u", sendData[0], sendData[1]);
			printf("id %u %u", sendData[2], sendData[3]);
			printf("num vowels %u %u", sendData[4], sendData[5]);
      

      if ((numbytesSent = sendto(sockfd, &sendData, packetSize, 0,
         (struct sockaddr *)&their_addr, addr_len))==-1) {
         perror("serverUDP: sendto");
         exit(1);
      }
   }
   close(sockfd);

   return 0;
}
