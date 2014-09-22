/* Sample TCP client */

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
}
