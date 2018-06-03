#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <dirent.h>

void sendDirectory(int, char*);
void sendfile(int, char*, char*);
void error(const char *msg){
	perror(msg);
	exit(1);
} // Error function used for reporting issues

int main(int argc, char *argv[]){
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[70005], message[70005], filename[21], dataPort[7], command[3];
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
		command[2] = '\0'; //set the last char to a null terminator instead of @
		printf("command: %s\n", command);

		charsRead = send(establishedConnectionFD, "1", 1, 0);

		// get the filename from the client and display it
		memset(filename, '\0', 21); //clear out the filename. since 20 characters in filename 21 is going to be \0
		while (strstr(filename, "@") == NULL){ //look for @ to know when client port number has ended
			charsRead = recv(establishedConnectionFD, buffer, 1, 0); // Read the client's port from the socket
			strcat(filename, buffer);
		}
		filename[strcspn(filename, "@")] = '\0'; //set the last bit to a null terminator instead of @
		printf("filename: %s\n", filename);

		//see what we got
		if (strncmp(command, "-l", 2) == 0){
			//send the list of the directory contents
			sendDirectory(strtol(dataPort, NULL, 10), inet_ntoa(clientAddress.sin_addr));
		}else if(strncmp(command, "-g", 2) == 0){
			//send the file that the client wanted on different port
			sendfile(strtol(dataPort, NULL, 10), inet_ntoa(clientAddress.sin_addr), filename);
		}
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
		printf("Connection with %s closed\n", inet_ntoa(clientAddress.sin_addr));
	}

	close(listenSocketFD); // Close the listening socket
	return 0;
}

void sendDirectory(int dataPort, char* hostname){
	char buffer[20];
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;	
	DIR* directory;
	struct dirent *file; 
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(dataPort); // Store the port number
	serverHostInfo = gethostbyname(hostname); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	usleep(10); //delay 10 milliseconds to allow for the client to set up its portion

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// get the file names and send them to the client
	// there can be a max file name length of 20 inluding the '.txt'
	directory = opendir ("./");

	if (directory != NULL){
		while ((file = readdir (directory)) != NULL) {
			if (strncmp(file->d_name + strlen(file->d_name) - 4, ".txt", 4) == 0) { //find out if the current file is a .txt
				memset(buffer, '\0', 20); //clear out the buffer
				strcpy(buffer, file->d_name); //put in the .txt file name
				printf("sending %s\n", buffer);
				charsWritten = send(socketFD, buffer, 20, 0); //send the file to the client
			}
		}
		//when we've sent all the .txt files let the client know and close the directory
		charsWritten = send(socketFD, "@@", 20, 0);
		(void) closedir (directory);
	}
}

void sendfile(int dataPort, char* hostname, char* filename){
	char buffer[11];
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;	
	FILE *file;
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(dataPort); // Store the port number
	serverHostInfo = gethostbyname(hostname); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	usleep(200); //delay 30 milliseconds to allow for the client to set up its portion

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	recv(socketFD, buffer, 1, 0); // Read the client's port from the socket
	if(buffer[0] == '1'){
		// get the file and send it to the client
		if (file = fopen(filename, "r")){
			do{
				memset(buffer, '\0', 11);
				fgets(buffer, 2, (FILE*)file);
				//printf("buffer: %s\n", buffer);
				//printf("the length is: %d\n", strlen(buffer));
				if(strlen(buffer) == 0) continue;
				charsWritten = send(socketFD, buffer, 1, 0);
			}while(!feof(file));


			fclose(file);
		}else{
			//send that there was an error getting the file
			return;
		}
		charsWritten = send(socketFD, "@", 1, 0);
	}
}