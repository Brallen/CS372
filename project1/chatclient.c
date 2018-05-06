#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char* argv[]){
	char username[12], recText[550], sendText[550], buffer[550];
	int socketFD, portNumber, charsWritten, charsRead, i = 1;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	
	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(1); } // Check usage & args
	
	printf("Enter username: ");
	fgets(username, 11, stdin);
	if(!strchr(username, '\n')) //newline does not exist
	    while(fgetc(stdin)!='\n');//discard until newline
	username[strcspn(username, "\n")] = ':'; //replace newline with colon
	strcat(username, " "); //add a space
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
	
	//loop sending and receiving
	while(i){
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer for use

		//create and send message
		printf("%s", username);
		fgets(buffer, 501, stdin); //501 cause of newline
		//make input only 500 characters and if longer get rid of it
		if(!strchr(buffer, '\n')) //newline does not exist
		    while(fgetc(stdin)!='\n');//discard until newline
		strcpy(sendText,username); //add username
		strcat(sendText, buffer); //add message
		sendText[strcspn(sendText, "\n")] = '\0'; //replace newline with null
		charsWritten = send(socketFD, sendText, strlen(sendText), 0);
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(sendText)) printf("CLIENT: WARNING: Not all data written to socket!\n");
		//if there is a \quit end the connection
		if (strstr(sendText, "\\quit") != NULL) {
			printf("You have left the chat\n");
			break;
		}
		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer for use
		charsRead = recv(socketFD, buffer, 549, 0); // Read data from the socket, leaving \0 at end
		strcpy(recText, buffer);
		//if there is a \quit then exit the connection
		if (strstr(recText, "\\quit") != NULL) {
			printf("Connection ended by server\n");
			break;
		}
		printf("%s\n", recText); //print what was received
	}
	
	close(socketFD); //close the socket
	return 0;
}