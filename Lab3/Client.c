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

#define MAX_BUF_LEN 1028
#define GID 28

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

// b0 b1 = packet length
// b2 = checksum
// b3 = groupID
// b4 = reqID
// b5 = '~' = 0x7E
// b6-b... = hostnames

void packSendData(char* buf, int gid, int portNum) {
	buf[0] = 0x12;
	buf[1] = 0x34;
	buf[2] = gid & 0x000000FF;
   buf[3] = (portNum & 0x0000FF00);
   buf[4] = (portNum & 0x000000FF);
}



int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in myaddr, remaddr;
   struct hostent *he;
   struct in_addr **addr_list;
	int rv;
	int numbytes;
	int serverPort;
   int myPort;
   int slen=sizeof(remaddr);
	uint8_t requestID;
	char buf[MAX_BUF_LEN];
	char rBuf[MAX_BUF_LEN];
	int tmp;

	if (argc < 4 || argc > 4) {
		fprintf(stderr,"usage: Client ServerName ServerPort MyPort\n");
		exit(1);
	}
   
	serverPort = atoi(argv[2]);
   myPort = atoi(argv[3]);
   
   if (serverPort != (10010 + (5*GID))) {
      fprintf(stderr,"Invalid Server Port: Must be equal to %d\n", (10010 + (5*GID)));
      exit(1);
   }
   if ((myPort < 10150) ||  (myPort > 10154)) {
      fprintf(stderr,"Invalid Client Port: Must be in the range 10150 to 10154\n");
      exit(1);
   }   
   if ((he = gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        return 0;
    }
    addr_list = (struct in_addr **)he->h_addr_list;
    
   /* create a socket */

	if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	/* bind it to all local addresses and pick any port number */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(serverPort);

	if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
   
   /* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */
   
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(serverPort);
	if (inet_aton(inet_ntoa(*addr_list[0]), &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
    
   memset(buf, 0, MAX_BUF_LEN);
	packSendData(buf, GID, myPort);
   
   // send packet
   if ((numbytes = sendto(sockfd, buf, sizeof(buf), 0,
		(struct sockaddr *)&remaddr, slen)) == -1) {
		   perror("UDPClient: sendto");
	}
      
	// receive packet
   if ((numbytes = recvfrom(sockfd, rBuf, MAX_BUF_LEN , 0,
      (struct sockaddr *)&remaddr, &slen)) ==-1)           {
         perror("recvfrom");
         exit(1);
  }
  printf("got packet\n");
  displayBuffer(rBuf, numbytes);
}