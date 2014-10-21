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

#define MAX_BUF_LEN 4098

void printIP(char* buf) {
   int pointer = 5;
   int numIP = 0;
   int length = (uint16_t) ( ((buf[0] << 8) | (buf[1]))& 0xFF);
   int counter = 1;
   length = length - 5;
   numIP = length / 4;
   
   while (numIP != 0) {
      printf("%f: %s.%s.%s.%s\n", counter, buf[pointer], buf[pointer+1], buf[pointer+2], buf[pointer+3]);
      counter++;
      numIP--;
      pointer = pointer +4;
   }
}

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

int testLength(char* buf) {
	uint16_t claimedLen = (uint16_t) ( ((buf[0] << 8) | (buf[1]))& 0xFF);
	int actualLen = 0;
	int i = 0;
	for (i; i < MAX_BUF_LEN; i++) {
		actualLen++;
		if(buf[i] == '\0')
			break;
	}

	return claimedLen == actualLen;
}

int testValidResponse(char* buf) {
	if (!buf[3] == 0x00 || !buf[4] == 0x00)
		return 1;
	else
		return 0;
}

int testChecksum(char* buf) {
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

	if (checksum == 0xFF)
		return 1;
	else
		return 0;
}

// b0 b1 = packet length
// b2 = checksum
// b3 = groupID
// b4 = reqID
// b5 = '~' = 0x7E
void packSendData(char* buf, char* tempBuff[], int numHosts, uint8_t gid, uint8_t ID) {
	uint16_t totalLen;
	uint8_t checksum;
	int hostLen;
	char* host;

	totalLen = 5;
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
	char buf[MAX_BUF_LEN];
	char rBuf[MAX_BUF_LEN];
	int tmp;

	if (argc < 5) {
		fprintf(stderr,"usage: UDPClient ServerName PortNum reqeustID hostname1 hostname2...\n");
		exit(1);
	}

	portNum = atoi(argv[2]);
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

	packSendData(buf, tempBuf, numHosts, GID, requestID);


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
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

	int valid = 0;
	int numAttempts = 0;

	while (!valid && (numAttempts < 7)) {
		memset(rBuf, 0, MAX_BUF_LEN);

		// send packet
		if ((numbytes = sendto(sockfd, buf, sizeof(buf), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
			perror("UDPClient: sendto");
			continue;
		}

		// receive packet
		if ((numbytes = recv(sockfd, rBuf, MAX_BUF_LEN-1, 0)) == -1) {
			perror("recvfrom");
			continue;
		}
      printf("got packet back\n");
		// validate packet's length and checksum
		if (testLength(rBuf) && testChecksum(rBuf)) {
			valid = 1;
		} else {
			numAttempts++;
		}
	}

	if (valid) {
		printIP(rBuf);
	} else {
		perror("UDPClient: Timeout");
	}

	close(sockfd);

	return 0;
}
