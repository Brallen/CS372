#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void sendDirectory(int);
void error(const char *msg){
	perror(msg);
	exit(1);
} // Error function used for reporting issues

int main(int argc, char *argv[]){
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, spawnPid, childExitMethod, j, i;
	socklen_t sizeOfClientInfo;
	char buffer[70005], message[70005], file[70005], dataPort[7], command[3];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc != 2){
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	} // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]);									 // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET;							 // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);					 // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY;					 // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0)
		error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//don't kill the server after just one connection
	while (1){ //could change this later so server doesn't have to be killed with ^C

		//initialize variables to be nothing
		memset(dataPort, '\0', 7);
		memset(command, '\0', 3);
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		printf("client IP address is: %s\n", inet_ntoa(clientAddress.sin_addr));
		if (establishedConnectionFD < 0)
			error("ERROR on accept");

		printf("IN CHILD\n");
		// Get the dataPort from the client and display it
		memset(buffer, '\0', 70005);
		while (strstr(dataPort, "@") == NULL){ //look for @ to know when client port number has ended
			charsRead = recv(establishedConnectionFD, buffer, 1, 0); // Read the client's port from the socket
			strcat(dataPort, buffer);
		}	
		printf("dataPort: %d\n", atoi(dataPort));
		charsRead = send(establishedConnectionFD, "1", 1, 0);

		//get the command from the client and display it
		memset(buffer, '\0', 70005);
		while (strstr(command, "@") == NULL){ //look for @ to know when client port number has ended
			charsRead = recv(establishedConnectionFD, buffer, 1, 0); // Read the client's port from the socket
			strcat(command, buffer);
		}
		command[2] = '\0'; //set the last bit to a null terminator instead of @
		printf("command: %s\n", command);
		charsRead = send(establishedConnectionFD, "1", 1, 0);

		//see what we got
		if (strncmp(buffer, "-l", 2) == 0){
			//send the list of the directory contents
			//charsRead = send(establishedConnectionFD, "wrong server@", 13, 0);
			sendDirectory(strtol(dataPort, NULL, 10));
			i = 0; //once done with that end the loop
		}else if(strncmp(buffer, "-g", 2) == 0){
			//send the file that the client wanted on different port
			//charsRead = send(establishedConnectionFD, "wrong server@", 13, 0);
			
			i = 0; //once done with that end the loop
		}
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
		printf("Connection with %s closed\n", inet_ntoa(clientAddress.sin_addr));

	}

	close(listenSocketFD); // Close the listening socket
	return 0;
}

void sendDirectory(int dataPort){
	printf("Data Port: %d", dataPort);
	return;
}