/* Sample TCP client */

<<<<<<< HEAD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h> 

  
void packSendData(char* buf, uint16_t length, uint16_t id, char op, char* message) {
   int i;
   buf[0] = (length & 0xFF00) >> 8;
   buf[1] = length & 0x00FF;
   buf[2] = ((id & 0xFF00) >> 8) & 0xFF;
   buf[3] = (id & 0x00FF) &0xFF;
	buf[4] = op;

   for (i = 5; i < length; i++) {
      buf[i] = message[i-5];
   }
}
uint16_t getLength(char* buf) {
      return (uint16_t) ( ((buf[0] << 8) | (buf[1]))& 0xFF);
   }
uint16_t getID(char* buf) {
	return (uint16_t) (((buf[2] << 8) | (buf[3])) & 0xFF);
}

uint16_t getVowels(char* buf) {
	return (uint16_t) (((buf[4] << 8) | (buf[5])) & 0xFF);
}

void printMessage(char* buf, uint16_t length) {
	int i = 4;
	for (i; i < length; i++)
	{
		printf("%c", buf[i]);
	}
	printf("\n\n");
}


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

int main(int argc, char **argv)
{
	struct timeval before, after;
   int sockfd, nBytes, rv;
   struct sockaddr_in serveraddr;
	struct hostent *server;
   char sendData[1029];
   char recvData[1029];
	uint16_t length = 0;
	uint16_t id;
	char op;
	int opp;
	char *message;
	char *hostname;
	char dv[1024];
	int port;
	
   if (argc != 5)
   {
      printf("usage:  ClientTCP hostname PortNum OP Message\n");
      exit(1);
   }
	hostname = argv[1];
	port = atoi(argv[2]);
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
    serveraddr.sin_port = htons(port);
	
	/* connect: create a connection with the server */
	if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

	
	id = 1;
	op =  (char) atoi(argv[3]);
	opp = (int) atoi(argv[3]);
	message = argv[4];
	length = strlen(message) + 5;

	bzero(sendData, 1029);
	packSendData(sendData, length, id, op, message);
	
	gettimeofday(&before, NULL);
   nBytes = write(sockfd, sendData, length);
	if (nBytes < 0)
		error("ERROR writing to socket");
			
	bzero(recvData, 1029);
   nBytes = recv(sockfd,recvData,1029, 0);
	if (nBytes < 0)
		error("ERROR reading from socket");
		
   gettimeofday(&after, NULL);
		
	printf("\nTime taken: %.3f microseconds, ", 
		(float)(((after.tv_sec - before.tv_sec)*1000000) + (after.tv_usec - before.tv_usec))/1000);
	printf("Request ID: %u, ", getID(recvData)); 
	if (opp == 85)
		printf("Number of vowels: %u\n", getVowels(recvData));
	else if (opp == 170){
		printf("Response: ");
		printMessage(recvData, getLength(recvData));
	}
=======
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define PORTNUM 10038

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   char sendline[1029];
   char recvline[1029];
	uint16_t length;
	uint16_t id;
	uint8_t op;
	char* message;
	
   if (argc != 2)
   {
      printf("usage:  ClientTCP hostname ID OP Message\n");
      exit(1);
   }
	
	id = argv[2];
	op = argv[3];
	message = argv[4];
	length = strlen(message) + 5;
	
	
   sockfd=socket(AF_INET,SOCK_STREAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(PORTNUM);

   connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

   while (fgets(sendline, 10000,stdin) != NULL)
   {
      sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
      n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
      recvline[n]=0;
      fputs(recvline,stdout);
   }
>>>>>>> b30ac148210da33acff52a7205675b329825a348
}
