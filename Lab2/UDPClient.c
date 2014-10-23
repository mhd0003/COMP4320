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

//prints out the ips from the buffer
void printIP(char* buf) {
   int pointer = 5;
   int numIP = 0;
   int length = (uint16_t) ( ((buf[0] << 8) | (buf[1]))& 0xFF);
   int counter = 1;
   length = length - 5;
   numIP = length / 4;
   
   printf("\n Returned IPs\n");
   while (numIP != 0) {
      printf("%d: ", counter);
      printf("%d.%d.%d.%d\n", (uint8_t)buf[pointer], (uint8_t)buf[pointer+1], 
         (uint8_t)buf[pointer+2], (uint8_t)buf[pointer+3]);
      counter++;
      numIP--;
      pointer = pointer + 4;
   }
   printf("\n");
}

//calculates the checksum and stores in an uint8_t
uint8_t calcChecksum(char* buf, int length) {
	int checksum = 0;
	int tmp = 0;
	int i = 0;
	for (i; i < length; i++) {
		checksum += buf[i];
		while (checksum > 255) {
			tmp = checksum & 0x000000FF;
			tmp++;
			checksum = tmp;
	      }
   }
   
	checksum = ~checksum & 0x000000FF;
	return (uint8_t)checksum;
}

// Tests the length by calculating actual length and testing against value in the buffer
int testLength(char* buf) {
	uint16_t claimedLen = (uint16_t) ( ((buf[0] << 8) | (buf[1]))& 0xFF);
	int actualLen = 0;
	int i = MAX_BUF_LEN-1;
   int start = 0;
	for (i; i >= 0; i--) {
      if (start)
		   actualLen++;
      else if (buf[i] != 0) {
         start = 1;
         actualLen++;
      }
	}
   if (claimedLen == actualLen)
	   return 1;
   else
      return 0;
}

// recalculates the checksum by ignoring the given checksum and compares it
int testChecksum(char* buf) {
	int checksum = 0;
	int tmp = 0;
	int i = 0;
	for (i; i < MAX_BUF_LEN; i++) {
      if (i != 2){
		   checksum += buf[i];
		   while (checksum > 255) {
			   tmp = checksum & 0x000000FF;
			   tmp++;
			   checksum = tmp;
	      }
      }
   }

	checksum = ~checksum & 0x000000FF;
   uint8_t claimed= buf[2];
   
	if (checksum == claimed)
		return 1;
	else
		return 0;
}

// b0 b1 = packet length
// b2 = checksum
// b3 = groupID
// b4 = reqID
// b5 = '~' = 0x7E
// b6-b... = hostnames
void packSendData(char* buf, char* tempBuff[], int numHosts, uint8_t gid, uint8_t ID) {
	uint16_t totalLen;
	uint8_t checksum;
	int hostLen;
	char* host;

	totalLen = 6;
	buf[3] = gid;
	buf[4] = ID;
	buf[5] = 0x7E;

	int i = 6, j, k;

	for (j = 0; j < numHosts; j++){
		host = tempBuff[j];
		hostLen = strlen(host);
		for (k = 0; k < hostLen; k++) {
			totalLen++;
			buf[i++] = host[k];
		}

		buf[i++] = 0x7E;
		totalLen++;
	}

	// remove last '~'
	buf[i-1] = 0;
	totalLen--;

	buf[0] = (totalLen & 0xFF00) >> 8;
	buf[1] = totalLen & 0x00FF;
	buf[2] = calcChecksum(buf, totalLen);
}



int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in myaddr, remaddr;
   struct hostent *he;
   struct in_addr **addr_list;
	int rv;
	int numbytes;
	int portNum;
   int slen=sizeof(remaddr);
	uint8_t requestID;
	uint8_t GID;
	char buf[MAX_BUF_LEN];
	char rBuf[MAX_BUF_LEN];
	int tmp;

	if (argc < 5) {
		fprintf(stderr,"usage: UDPClient ServerName PortNum reqeustID hostname1 hostname2...\n");
		exit(1);
	}
   
	portNum = atoi(argv[2]);
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
	myaddr.sin_port = htons(portNum);

	if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
   
   /* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */
   
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(portNum);
	if (inet_aton(inet_ntoa(*addr_list[0]), &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	GID = (uint8_t) portNum - 10010;

	tmp = atoi(argv[3]);
	if (tmp <= 127)
		requestID = (short)tmp;
	else
		exit(0);

	int i = 4;
	int j = 0;
	int numHosts = argc - 4; // If only 1 hostname, argc = 5
	char* tempBuf[numHosts];
	// for (i; i < argc-1 && j < 7; i++) {
	for (i = 4; i < argc; i++) {
		tempBuf[j] = argv[i];
		j++;
	}
   memset(buf, 0, MAX_BUF_LEN);
	packSendData(buf, tempBuf, numHosts, GID, requestID);
   
	int valid = 0;
	int numAttempts = 0;
   memset(rBuf, 0, MAX_BUF_LEN);
	while (!valid && (numAttempts < 7)) {
		

		// send packet
		if ((numbytes = sendto(sockfd, buf, sizeof(buf), 0,
				(struct sockaddr *)&remaddr, slen)) == -1) {
			perror("UDPClient: sendto");
			continue;
		}
      
		// receive packet
      if ((numbytes = recvfrom(sockfd, rBuf, MAX_BUF_LEN , 0,
         (struct sockaddr *)&remaddr, &slen)) ==-1)           {
         perror("recvfrom");
         exit(1);
      }
      
		// validate packet's length and checksum
		if (testLength(rBuf) && testChecksum(rBuf)) {
			valid = 1;
		} else {
			numAttempts++;
         memset(rBuf, 0, MAX_BUF_LEN);
		}
	}

	if (valid) {
		printIP(rBuf);
	} else {
		printf("UDPClient: Timeout\n");
	}

	close(sockfd);

	return 0;
}
