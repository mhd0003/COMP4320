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

int isInvalidMove(char* buf) {
   int invalid = 0;

   if(buf[3] == 0xFF && buf[4] == 0xFF)
      invalid = 1;
   
   return invalid;
}

void playTurn(char* buf, int* nim) {
      int row = 0;
      int token = 0;
      
      printf("\nRow#\t:Number of tokens\n");
      printf("1\t:%d\n", nim[0]);
      printf("2\t:%d\n", nim[1]);
      printf("3\t:%d\n", nim[2]);
      printf("4\t:%d\n\n", nim[3]);
      printf("Choose row: ");
      scanf("%d", &row);
      printf("Enter amount of token: ");
      scanf("%d", &token);
      
      if(validMove(row, token, nim)) {
         nim[row-1] = nim[row-1] - token;
               
         if((nim[0] + nim[1] + nim[2] + nim[3]) == 1) {
            printf("\nRow#\t:Number of tokens\n");
            printf("1\t:%d\n", nim[0]);
            printf("2\t:%d\n", nim[1]);
            printf("3\t:%d\n", nim[2]);
            printf("4\t:%d\n\n", nim[3]);
            printf("Game Over! You Won!\n");
            exit(0);
         }
      }
      
      buf[0] = 0x12;
      buf[1] = 0x34;
      buf[2] = (char) GID;
      buf[3] = (char) row;
      buf[4] = (char) token;
}

// returns 1 if the move is valid, else it returns 0
int setBoard(char* buf, int* nim) {
   int row = (uint8_t) buf[3];
   int token = (uint8_t) buf[4];
   int bool = 1;
   
   if (checkMagic(buf)) {
   
      if (!validMove(row, token, nim)) {
         bool = 0;
      }
      else {
         nim[row-1] = nim[row-1] - token;
      }
   }
   return bool;
}

//returns 1 if the move is valid, else it returns 0
int validMove(int row, int token, int* nim) {
   int bool = 1;
   
   if (row < 1 || row > 4){
      bool = 0;
   }
   else if ((nim[row-1] - token) < 0 || token < 1) {
      bool = 0;
   }
   
   return bool;
}

// converts the given IP into a string from the buffer
void getIP(char* buf, char* ip) {
   int ip1 = (uint8_t) buf[3];
   int ip2 = (uint8_t) buf[4];
   int ip3 = (uint8_t) buf[5];
   int ip4 = (uint8_t) buf[6];
   
   sprintf(ip, "%u.%u.%u.%u", ip1, ip2, ip3, ip4);
   printf("%s\n",ip);
}

// checks the magic number, returns 1 if it matches else 0
int checkMagic(char* buf) {
   int bool = 0;
   int tmp = buf[0];
   tmp = tmp << 8;
   tmp = tmp | buf[1];
   
   if(tmp == 4660)
      bool = 1;
      
   return bool;
}

// prints  an error message
void getError(char* buf) {
   int tmp = (uint8_t) buf[4];
   
   if (tmp == 1)
      printf("No magic number\n");
   else if (tmp == 2)
      printf("Incorrect length\n");
   else if (tmp == 4)
      printf("Port out of range\n");
   else 
      printf("Invalid magic number\n");
}

// returns an int that tells whether the client is invalid (0), server (2), or client (1)
int getClientType(char* buf) {
   int clientOrServer = -1;
   
   if (checkMagic(buf)) {
   
      if (isInvalid(buf)==1) 
         clientOrServer = 0;
    
      else if (getLength(buf) == 9) 
         clientOrServer = 2;
      
      else if (getLength(buf) == 5)
         clientOrServer = 1;
   }
   
   return clientOrServer;
   
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

// tells if the packet contains an error message
int isInvalid(char* buf) {
   int invalid = 0;
   int tmp = (uint8_t) buf[3];
   
   if (tmp == 0)
      invalid  = 1;
      
   return invalid;
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

// packs the packet with info
void packSendData(char* buf, int gid, int portNum) {
	buf[0] = 0x12;
	buf[1] = 0x34;
	buf[2] = gid & 0x000000FF;
   buf[3] = (portNum >> 8) & 0x000000FF;
   buf[4] = (portNum & 0x000000FF);
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
   char ip[7];
   
   struct sockaddr_in gameServer, clientAddr;
   int listenfd, connfd;
   socklen_t clilen;

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
   memset(rBuf, 0, MAX_BUF_LEN);
	packSendData(buf, GID, myPort);
   
   // send packet
   if ((numbytes = sendto(sockfd, buf, sizeof(buf), 0,
		(struct sockaddr *)&remaddr, slen)) == -1) {
		   perror("client: sendto");
	}
      
	// receive packet
   if ((numbytes = recvfrom(sockfd, rBuf, MAX_BUF_LEN , 0,
      (struct sockaddr *)&remaddr, &slen)) ==-1)           {
         perror("recvfrom");
         exit(1);
   }
   
   close(sockfd);
   
   if(getClientType(rBuf) == 1)
   {
      listenfd=socket(AF_INET,SOCK_STREAM,0);
      
      bzero(&gameServer,sizeof(gameServer));
      gameServer.sin_family = AF_INET;
      gameServer.sin_addr.s_addr=htonl(INADDR_ANY);
      gameServer.sin_port=htons(myPort);
      
      bind(listenfd,(struct sockaddr *)&gameServer,sizeof(gameServer));
      
      listen(listenfd,1024);
      
      int nim[4] = {1,3,5,7};
      int row = 0;
      int token = 0;
      
      printf("\nRow#\t:Number of tokens\n");
      printf("1\t:%d\n", nim[0]);
      printf("2\t:%d\n", nim[1]);
      printf("3\t:%d\n", nim[2]);
      printf("4\t:%d\n", nim[3]);
      printf("Waiting for player to connect...\n\n");
      
      clilen = sizeof(clientAddr);
      connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clilen);
      printf("Player connected\n");
      
      playTurn(buf, nim);    
              
      numbytes = send(connfd, buf, MAX_BUF_LEN, 0);
      if (numbytes < 0)
         error("ERROR writing to socket");

      while(1) {
         
         memset(buf, 0, MAX_BUF_LEN);
         memset(rBuf, 0, MAX_BUF_LEN);
         
         numbytes = recv(connfd,rBuf,MAX_BUF_LEN, 0);
         if (numbytes < 0)
      	   error("ERROR reading from socket");
               
         if (isInvalidMove(rBuf)) {
            playTurn(buf, nim);
            
            numbytes = send(connfd, buf, MAX_BUF_LEN, 0);
            if (numbytes < 0)
               error("ERROR writing to socket");
            
         }
         else if(!setBoard(rBuf, nim)){
            memset(buf, 0, MAX_BUF_LEN);
                     
            buf[0] = 12;
            buf[1] = 34;
            buf[2] = GID;
            buf[3] = 0xFF;
            buf[4] = 0xFF;
                     
            numbytes = send(connfd, buf, MAX_BUF_LEN,0);
      	   if (numbytes < 0)
      	      error("ERROR writing to socket");
         } 
         else if((nim[0] + nim[1] + nim[2] + nim[3]) == 1){
            printf("\nRow#\t:Number of tokens\n");
            printf("1\t:%d\n", nim[0]);
            printf("2\t:%d\n", nim[1]);
            printf("3\t:%d\n", nim[2]);
            printf("4\t:%d\n\n", nim[3]);
            printf("Game Over! You lost!\n");
            exit(0);
         }
         else {
         
            playTurn(buf, nim);
            
            numbytes = send(connfd, buf, MAX_BUF_LEN,0);
      	   if (numbytes < 0)
      	      error("ERROR writing to socket");
         }
      }

   } 
   else if(getClientType(rBuf) == 2) 
   {
      getIP(rBuf, ip);
      
      listenfd=socket(AF_INET,SOCK_STREAM,0);
      
      bzero(&gameServer,sizeof(gameServer));
      gameServer.sin_family = AF_INET;
      printf("Attempting to put ip into gameServer\n");
      inet_pton(AF_INET, ip, &(gameServer.sin_addr));
      printf("Set ip\n");
      gameServer.sin_port=htons(myPort);
      
      printf("trying to connect\n");
      connect(listenfd, (struct sockaddr *)&gameServer, sizeof(gameServer));
      printf("connected\n");
      int nim[4] = {1,3,5,7};
      
      while(1) {
         
         memset(buf, 0, MAX_BUF_LEN);
         memset(rBuf, 0, MAX_BUF_LEN);
         
         numbytes = recv(listenfd,rBuf,MAX_BUF_LEN, 0);
         if (numbytes < 0)
      	   error("ERROR reading from socket");
               
         if (isInvalidMove(rBuf)) {
         
            playTurn(buf, nim);
            
            numbytes = send(listenfd, buf, MAX_BUF_LEN,0);
      	   if (numbytes < 0)
      	      error("ERROR writing to socket");
         }
         else if(!setBoard(rBuf, nim)){
         
            memset(buf, 0, MAX_BUF_LEN);
                     
            buf[0] = 12;
            buf[1] = 34;
            buf[2] = GID;
            buf[3] = 0xFF;
            buf[4] = 0xFF;
                     
            numbytes = send(listenfd, buf, MAX_BUF_LEN,0);
      	   if (numbytes < 0)
      	      error("ERROR writing to socket");
         } 
         else if((nim[0] + nim[1] + nim[2] + nim[3]) == 1){
            printf("\nRow#\t:Number of tokens\n");
            printf("1\t:%d\n", nim[0]);
            printf("2\t:%d\n", nim[1]);
            printf("3\t:%d\n", nim[2]);
            printf("4\t:%d\n\n", nim[3]);
            printf("Game Over! You lost!\n");
            exit(0);
         }
         else {
                        
            playTurn(buf, nim);   
            
            numbytes = send(listenfd, buf, MAX_BUF_LEN,0);
      	   if (numbytes < 0)
      	      error("ERROR writing to socket");
         }
      }
   } 
   else {
      getError(rBuf);
      exit(1);
   }
}
