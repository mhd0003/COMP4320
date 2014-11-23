#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUF_LEN 1028
#define GID 28

void getIP(char* str, unsigned char* dest) {
   unsigned char value[4] = {0};
   char* str2;
   size_t index = 0;

   str2 = str; /* save the pointer */
   while (*str) {
      if (isdigit((unsigned char)*str)) {
         dest[index] *= 10;
         dest[index] += *str - '0';
      } else {
         index++;
      }
      str++;
   }
}

// gets the length of the packet
int getLength(char* buf) {

	int length = 0;
	int i = MAX_BUF_LEN-1;
   int start = 0;
   
	for (i; i >= 0; i--) {
      if (start)
		   length++;
      else if (buf[i] != 0) {
         start = 1;
         length++;
      }
	}
   
   return length;
}

int valid(char* buf, int xy) {
   int valid = 1;
   uint16_t portNum = 0;
   uint8_t gid = (uint8_t) (buf[2] & 0xFF);
   uint16_t magicNum = 0;
   
   portNum = (uint16_t) (buf[3] &0xFF) << 8;
   portNum = ((uint16_t) (buf[4] & 0xFF)) | portNum;
   
   magicNum = (uint16_t) (buf[0] &  0xFF) << 8;
   magicNum = ((uint16_t) (buf[1] & 0xFF)) | magicNum;
   
   if (magicNum != 0x1234) {
      printf("Invalid Magic Numbert\n");
      valid = 0;
      xy = 1;
   }
   else if (getLength(buf) != 5) {
      printf("Invalid Length\n");
      valid = 0;
      xy = 2;
   } 
   else if ((portNum < (10010 + (gid *5))) || (portNum > (10014 + (gid * 5)))) {
      printf("Invalid port\n");
      valid = 0;
      xy = 4;
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
   int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   int numbytesRecv, numbytesSent;
   struct sockaddr_storage their_addr;
   char buf[MAX_BUF_LEN];
   socklen_t addr_len;
   char their_addr_str[INET6_ADDRSTRLEN];
   char rBuf[MAX_BUF_LEN];
   
   uint8_t gid = 28;
   int waiting = 0;
   uint16_t clientPort = 0;
   unsigned char clientIP[4] = {0};
   uint8_t clientGID = -1;
   uint8_t xy = 0;
   


   if (argc != 2) {
	   printf("Usage: ServerUDP PortNum\n");
      exit(1);
	}
   
   if (atoi(argv[1]) > 10154 || atoi(argv[1]) < 10150) {
      printf("Port: Invalid range 10150-10154");
      exit(1);
   }
	
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
      memset(&rBuf, 0, sizeof rBuf);
      memset(&buf, 0, sizeof buf);
      printf("\n\nWaiting for packet...\n");

      addr_len = sizeof their_addr;
      if ((numbytesRecv = recvfrom(sockfd, rBuf, MAX_BUF_LEN , 0,
         (struct sockaddr *)&their_addr, &addr_len)) ==-1)           {
         perror("recvfrom");
         exit(1);
      }
            
      printf("Got packet from %s. ",
         inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            their_addr_str, sizeof their_addr_str));
            
      if (valid(rBuf, xy)) {
         if (waiting == 0) {
            waiting = 1;
            printf("sending wait\n");
            
            getIP((char*)their_addr_str, clientIP);
            
            clientPort = (uint16_t) (rBuf[3] &0xFF) << 8;
            clientPort = ((uint16_t) (rBuf[4] & 0xFF)) | clientPort;
            clientGID = (uint8_t) (rBuf[2] & 0xFF);
            
            buf[0] = 0x12;
            buf[1] = 0x34;
            buf[2] = (char) gid;
            buf[3] = rBuf[3];
            buf[4] = rBuf[4];
         }
         else {
            memset(&buf, 0, sizeof buf);
            printf("Sending client info\n");
            
            buf[0] = 0x12;
            buf[1] = 0x34;
            buf[2] = (char) clientGID;
            buf[3] = (char) clientIP[0];
            buf[4] = (char) clientIP[1];
            buf[5] = (char) clientIP[2];
            buf[6] = (char) clientIP[3];
            buf[7] = (char) (clientPort >> 8);
            buf[8] = (char) (clientPort & 0x00FF);
            
            memset(&clientIP, 0, sizeof clientIP);
            clientPort = 0;
            clientGID = 0;
            waiting = 0;
         }
      }
      else {
         buf[0] = 0x12;
         buf[1] = 0x34;
         buf[2] = (char) gid;
         buf[3] = 0x00;
         buf[4] = xy;
      }
      
      // send packet
      if ((numbytesSent = sendto(sockfd, buf, sizeof buf, 0,
         (struct sockaddr *)&their_addr, addr_len))==-1) {
         perror("serverUDP: sendto");
         exit(1);
      }
      
      printf("Server sent packet\n");
   }
   close(sockfd);

   return 0;
}
