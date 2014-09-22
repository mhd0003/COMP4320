/* Sample TCP client */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define PORT 10038
  
void packSendData(char* buf, uint16_t length, uint16_t id, uint8_t op, char* message) {
   int i;
   buf[0] = (length & 0xFF00) >> 8;
   buf[1] = length & 0x00FF;
   buf[2] = (id & 0xFF00) >> 8;
   buf[3] = id & 0x00FF;
	buf[4] = op;

   for (i = 5; i < length; i++) {
      buf[i] = message[i-5];
   }
}

uint16_t getID(char* buf) {
	return (uint16_t) (((buf[2] << 8) | (buf[3])) & 0xff);
}

uint16_t getVowels(char* buf) {
	return (uint16_t) (((buf[4] << 8) | (buf[5])) & 0xff);
}

void getMessage(char* buf, char* message) {
	int i = 4;
	for (i; i < strlen(buf); i++)
	{
		message[i-4] = buf[i];
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


int main(int argc, char **argv)
{
	double startTime;
   int sockfd, nBytes, rv;
   struct sockaddr_in serveraddr;
	struct hostent *server;
   char sendData[1029];
   char recvData[1029];
	uint16_t length = 0;
	uint16_t id;
	uint8_t op;
	char *message;
	char *hostname;
	char dv[1024];
	char *port = "10038";
	
   if (argc != 5)
   {
      printf("usage:  ClientTCP hostname ID OP Message\n");
      exit(1);
   }
	hostname = argv[1];

	/* create the socket*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("Error opening socket");
	
	/* gethostbyname: get server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host as %s\n", hostname);
		exit(0);
	}
	
	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(PORT);
	
	/* connect: create a connection with the server */
	if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

	
	id = (uint16_t) atoi(argv[2]);
	op = (uint8_t) atoi(argv[3]);
	message = argv[4];
	length = strlen(message) + 5;
	printf("Length: %u, ID: %u, OP: %u, Message: %s \n", length, id, op, message);
	bzero(sendData, 1029);
	packSendData(sendData, length, id, op, message);
	
		//startTime = Now();
     	nBytes = write(sockfd, sendData, length);
		if (nBytes < 0)
			error("ERROR writing to socket");
			
		bzero(sendData, 1029);
      nBytes = read(sockfd,recvData,1029);
		if (nBytes < 0)
			error("ERROR reading from socket");
		
      //startTime = Now() - startTime;
		
		//printf("Time taken: %f\n", startTime);
		printf("Request ID: %s\n", getID(recvData));
		if (op == 55)
			printf("Number of vowels: %u\n", getVowels(recvData));
		else if (op == 170)
			getMessage(recvData, dv);
			printf("Response: %s\n",dv);

}
