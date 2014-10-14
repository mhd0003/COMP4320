

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


char calcChecksum(char* buf) {
	int checksum = 0;
   int tmp = 0;
	int i = 0;
	for (i; i < sizeof(buf); i++) {
		checksum += buf[i];
      while (checksum > 255) {
         tmp = checksum & 0x000000FF;
         tmp++;
         checksum = tmp;
      }
	}

	checksum = ~checksum & 0x000000FF;
	return (char)checksum;
}

void packSendData(char* buf, char* tempBuff[], uint8_t gid, uint8_t ID) {
	uint16_t length;
	uint8_t checksum;

	length = 5;
	buf[3] = gid;
	buf[4] = ID;
	buf[5] = 0x7E;

	int i = 6;
	int j = 0;
	int k = 0;
	
	for (j; j < sizeof(tempBuff); j++){
		char* tmpHost = tempBuff[j];
		for (k; k < sizeof(tmpHost); k++) {
			length++;
			buf[i++] = tmpHost[k];
		}
		if (sizeof(tempBuff) - 1 != j) {
			buf[i++] = 0x7E;
			length++;
		}
	}

	buf[0] = (length & 0xFF00) >> 8;
	buf[1] = length & 0x00FF;
	buf[2] = calcChecksum(buf);
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	int portNum;
	uint8_t requestID;
	uint8_t GID;
	char buf[1024];
   char rBuf[1024];
	int tmp;

	if (argc < 5) {
		fprintf(stderr,"usage: UDPClient ServerName PortNum reqeustID hostname1 hostname2...\n");
		exit(1);
	}

	portNum = stoi(argv[2]);
	GID = (uint8_t) portNum - 10010;

	tmp = stoi(argv[3]);
	if (tmp <= 127)
		requestID = (short)tmp;
	else
		exit(0);

	int i = 4;
	int j = 0;
	int size = argc - 5;
	char* tempBuf[size];
	for (i; i < argc-1 && j < 7; i++) {
		tempBuf[j] = argc[i];
		j++;
	}

	fillBuffer(buf, tempBuf, GID, requestID);


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], portNum, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}
   
   freeaddrinfo(servinfo);

   bool valid = false;
   int numAttempts = 0;
   
   while (!valid && (numAttempts < 7)) {
   	if ((numbytes = sendto(sockfd, buf, sizeof(buf), 0,
   			 p->ai_addr, p->ai_addrlen)) == -1) {
   		perror("UDPClient: sendto");
   		exit(1);
   	}
      
      if ((numbytes = recvfrom(sockfd, rBuf, sizeof(rBuf)-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) ==-1){
            perror("recvfrom");
            exit(1);
      }
      
      //validate checksum and length, check if got a valid response
      //if valid set valid to true, else increment numAttempts
   }

	if (valid) {
      //print out the ips
   } else
      perror("UDPClient: Timeout");
      
	close(sockfd);

	return 0;
}
