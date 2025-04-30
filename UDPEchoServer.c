#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ECHOMAX 255   /* Longest string to echo */
#define MAXCLIENT 10 // Array size for number of clients
#define MAX_NICKNAME_LEN 32 // arbitrarily chosen max name length

struct MessageHeader {
    uint16_t type;     // 1 = JOIN, 2 = CHAT, 3 = LEAVE
    uint16_t length;   // number of bytes in data (nickname or message)
};

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{

	int sock; /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr[MAXCLIENT]; /* Client addresses list*/
	char nicknames[MAXCLIENT][MAX_NICKNAME_LEN]; // Client username list

	memset(echoClntAddr, 0, sizeof(echoClntAddr)); 

	unsigned int cliAddrLen; /* Length of incoming message */
	char buffer[ECHOMAX]; /* Buffer for echo string */
	unsigned short echoServPort; /* Server port */
	int recvMsgSize; /* Size of received message */

	if (argc != 2) /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]) ;
		exit(1);
	}

	echoServPort = atoi(argv[1]); /* First arg: local port */

	/* Create socket for incoming connections */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError( "socket () failed") ;

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort); /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError ( "bind () failed");


	for (;;) /* Run forever */
	{
		struct sockaddr_in tempClient;
		/* Set the size of the in-out parameter */
		cliAddrLen = sizeof(tempClient);
		int bool = 0;
		/* Block until receive message from a client */
		if ((recvMsgSize = recvfrom(sock, buffer, ECHOMAX, 0, (struct sockaddr *) &tempClient, &cliAddrLen)) < 4){
			DieWithError("recvfrom() failed") ;
		}//No need for timeout we have leave messages
			
		struct MessageHeader header;
		memcpy(&header, buffer, sizeof(header));// copying received header
		header.type = ntohs(header.type);
		header.length = ntohs(header.length);

		if(header.type == 1){// If message type is JOIN
			printf("Received JOIN from %s - %s\n", inet_ntoa(tempClient.sin_addr), buffer + 4);
			for(int i = 0; i < MAXCLIENT; i++){
				if(echoClntAddr[i].sin_addr.s_addr == 0 && echoClntAddr[i].sin_addr.s_addr == 0){//if theres space in the list
					echoClntAddr[i] = tempClient;
					memcpy(nicknames[i], buffer + 4, header.length);
					nicknames[i][header.length] = '\0';
				} 
				else if(echoClntAddr[i].sin_addr.s_addr == tempClient.sin_addr.s_addr &&
					echoClntAddr[i].sin_family == tempClient.sin_family &&
					echoClntAddr[i].sin_port == tempClient.sin_port){//If address already in list
						nicknames[i][header.length] = '\0'; //Change nickname I guess
				}
			}
		}
		else if(header.type = 2){// Message type = CHAT
			int sender = -1;
			for(int i = 0; i < MAXCLIENT; i++){//To find sender
				if(echoClntAddr[i].sin_addr.s_addr == tempClient.sin_addr.s_addr &&
					echoClntAddr[i].sin_family == tempClient.sin_family &&
					echoClntAddr[i].sin_port == tempClient.sin_port){
						sender = i;
					}
			}if(sender == -1) continue; // Sender not found? Next loop

			char message[MAX_NICKNAME_LEN + header.length];
			sprintf("[%s]: %s\n", nicknames[sender], buffer+4);
			printf("Handling message from client %s\n", inet_ntoa(tempClient.sin_addr));

			/* Send received message to every client */
			for(int i = 0; i < MAXCLIENT; i++){
				if (sendto(sock, message, MAX_NICKNAME_LEN + header.length, 0, (struct sockaddr *) &echoClntAddr[i], sizeof(echoClntAddr[i])) != recvMsgSize)
					DieWithError("sendto() sent a different number of bytes than expected");
			}
		}
	}	
		
			
		
/* NOT REACHED */
}