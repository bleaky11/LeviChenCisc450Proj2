#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define ECHOMAX 255   /* Longest string to echo */
#define MAX_NICKNAME_LEN 32 // arbitrarily chosen max name length

struct MessageHeader{
	uint16_t type;
    uint16_t length;
};


void DieWithError(char *errorMessage);

void* receive(void *sock){
	int socked = *((int *)sock); 

	for(;;){//Loop forever
		char buffer[ECHOMAX];
		struct sockaddr_in fromAddr;
		socklen_t fromLen = sizeof(fromAddr);
		int respStringlen;
		if((respStringlen = recvfrom(socked, buffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromLen)) < 0){
			DieWithError("Message wasn't received properly");
		}
		buffer[respStringlen] = '\0';

		if(strcmp(buffer, "GOODBYE") == 0){
			printf("[Server]: GOODBYE\n");
			break;
		}else
			printf("%s\n", buffer);
	}
}

int main(int argc, char *argv[])
{
	int sock; /*Socket descriptor*/
	struct sockaddr_in echoServAddr; /* Echo server address */
	unsigned short echoServPort;
	char *servIP;
	char nickName[MAX_NICKNAME_LEN];
	unsigned int echoStringLen;

	if ((argc<2)||(argc>3))
	{
			printf("Usage: ./objectname <Server IP> [<Echo Port>]\n");
			exit(1);
	}

	servIP = argv[1];

	//Creating join message
	do{
		printf("Choose a nickname(max %d characters): ", MAX_NICKNAME_LEN);
		fgets(nickName, sizeof(nickName), stdin);

		nickName[strcspn(nickName, "\n")] = '\0';
    	echoStringLen = strlen(nickName);
	}while(echoStringLen == 0 || echoStringLen > MAX_NICKNAME_LEN); /* Check input length */

	struct MessageHeader header;
	header.type = htons(1);  // JOIN
	header.length = htons(strlen(nickName));
	
	char message[4 + MAX_NICKNAME_LEN]; // 4 for header, 256 for nickname
	memcpy(message, &header, 4);
	memcpy(message + 4, nickName, strlen(nickName));

	//Port stuff
	if(argc == 3)
		echoServPort = atoi(argv[2]);
	else
		echoServPort = 7;

	/* Create a datagram/UDP socket */
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET;   /* Internet addr family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	echoServAddr.sin_port = htons(echoServPort);  /* Server port */

	//Making pthread
	pthread_t receiver;
	pthread_create(&receiver, NULL, receive, (void*)&sock);


	/*send the JOIN message to the server*/
	if (sendto(sock, message, echoStringLen + 4, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoStringLen + 4)
		DieWithError("send() sent a different number of bytes than expected");

	printf("\tType \"/quit\" to leave at anytime\n");
	for(;;){//For loop to keep sending messages
		char buffer[ECHOMAX];
		struct MessageHeader newHeader;
		memset(buffer, 0, sizeof(buffer));
		fgets(buffer, ECHOMAX, stdin);
		buffer[strcspn(buffer, "\n")] = '\0';
		char newMessage[4 + strlen(buffer)]; // 4 for header, rest for message
		if(strncmp(buffer, "/quit", 5) == 0){
			newHeader.type = htons(3);//Leave type
			newHeader.length = htons(0);
			memcpy(newMessage, &newHeader, 4);
			memcpy(newMessage + 4, buffer, strlen(buffer));
		}else{
			newHeader.type = htons(2);//Chat type
			newHeader.length = htons(strlen(buffer));
			memcpy(newMessage, &newHeader, 4);
			memcpy(newMessage + 4, buffer, strlen(buffer));
		}if (sendto(sock, newMessage, strlen(buffer) + 4, 0,(struct sockaddr *)&echoServAddr,
		 sizeof(echoServAddr)) != strlen(buffer) + 4)
			DieWithError("send() sent a different number of bytes than expected");
		if(strncmp(buffer, "/quit", 5) == 0){
			sleep(1);
			break;
		}
		
	}

	close(sock);
	exit(0);
}