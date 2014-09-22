/* Sample TCP client */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define PORTNUM 10038

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


int main(int argc, char**argv)
{
	double startTime;
   int sockfd,nBytes;
   struct sockaddr_in servaddr,cliaddr;
   char sendData[1029];
   char recvData[1029];
	uint16_t length = 0;
	uint16_t id;
	uint8_t op;
	char* message;
	char dv[1024];
	
   if (argc != 5)
   {
      printf("usage:  ClientTCP hostname ID OP Message\n");
      exit(1);
   }
	
   id = (uint16_t) (int) argv[2];
   op = (uint8_t) (int) argv[3];
   message = (char*)argv[4];
   length = strlen(message) + 5;
   packSendData(sendData, length, id, op, message);
		
   sockfd=socket(AF_INET,SOCK_STREAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(PORTNUM);

   connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

   while (fgets(sendData, 10000,stdin) != NULL)
   {
		//startTime = Now();
      sendto(sockfd,sendData,strlen(sendData),0,
         (struct sockaddr *)&servaddr,sizeof(servaddr));
      nBytes=recv(sockfd,recvData,1029,0);
		
      //startTime = Now() - startTime;
		
	//printf("Time taken: %f\n", startTime);
	printf("Request ID: %s\n", getID(recvData));
	if (op == 55)
		printf("Number of vowels: %u\n", getVowels(recvData));
	else if (op == 170)
		getMessage(recvData, dv);
		printf("Response: %s\n",dv);
   }
}
