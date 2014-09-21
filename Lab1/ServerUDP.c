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

struct rPacket {
uint16_t length;
uint16_t id;
uint8_t op;
char* str;
};

typedef rPacket rPacket_t;

struct nvPacket {
   uint16_t length;
   uint16_t id;
   uint16_t numVowel;
};

typedef nvPacket nvPacket_t;

struct dvPacket {
   uint16_t length;
   uint16_t id;
   char* dv;
};

typedef dvPacket dvPacket_t;

uint16_t vLength(char* str) {
	int i,numVowel = 0;

   for(i; i < sizeOf(str); i++) {
    if(check_vowel(str[i]) == 1) {       //is a vowel
      numVowel++;
    }
  }
}

char* disemvowel(char* str){
    char* dv;
    int i,j = 0;

    for(i; i < sizeOf(str); i++) {
    if(check_vowel(str[i]) == 0) {       //not a vowel
      dv[j] = str[i];
      j++;
    }
  }

  return dv;
}

int check_vowel(char c)
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


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytesRecv, numbytesSent;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char their_addr_str[INET6_ADDRSTRLEN];
	rPacket_t receiveData;
   int myPort;
   char* sendData = NULL;

   if (argc != 2)
      exit(1);
   else
      myPort = (int)argv[1];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, myPort, &hints, &servinfo)) != 0) {
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
			(struct sockaddr *)&their_addr, &addr_len)) ==-1) 			{
			perror("recvfrom");
			exit(1);
		}

		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				their_addr_str, sizeof their_addr_str));

		receiveData.length = buf[0];
		receiveData.length = test.length << 8;
		receiveData.length = test.length | buf[1];
		receiveData.id = buf[2];
		receiveData.id = test.id << 8;
		receiveData.id = test.id | buf[3];
		receiveData.op = buf[4];
		int j = 0;
		for (int i = 5; i <sizeOf(buf); i++) {
			receiveData.str[j] = buf[i];
		}

		if (receiveData.op == 0x55){
         nvPacket_t sendPacket;
         sendPacket.length = receiveData.length
         sendPacket.id = receiveData.id;
			sendPacket.numVowel = vLength(receiveData.str);
      }

		if (receiveData.op == 0xAA) {
			dvPacket_t sendPacket;
         sendPacket.length = receiveData.length
         sendPacket.id = receiveData.id;
			sendPacket.dv = disemvowel(receiveData.str);
      }

      sendData = (char)sendPacket;

		if ((numbytesSent = sendto(sockfd, sendData, sizeOf sendData, 0,
      	   (struct sockaddr *)&their_addr, addr_len))==-1) {
			perror("serverUDP: sendto");
			exit(1);
		}



	}
	close(sockfd);

	return 0;
}
