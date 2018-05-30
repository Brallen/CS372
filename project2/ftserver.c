#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void encryptString(char*, char*, char*);
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
   int listenSocketFD, establishedConnectionFD, portNumber, charsRead, spawnPid, childExitMethod, j, i;
   socklen_t sizeOfClientInfo;
   char buffer[70005], key[70005], message[70005], encryptedString[70005];
   struct sockaddr_in serverAddress, clientAddress;

   if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

   // Set up the address struct for this process (the server)
   memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

   // Set up the socket
   listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if (listenSocketFD < 0) error("ERROR opening socket");

   // Enable the socket to begin listening
   if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
      error("ERROR on binding");
   listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

   //don't kill the server after just one connection
   while(1){ //could change this later so server doesn't have to be killed with ^C 
      // Accept a connection, blocking if one is not available until one connects
      sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
      establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
      if (establishedConnectionFD < 0) error("ERROR on accept");
      //fork when get a connection
      spawnPid = fork();
      switch(spawnPid){ 
	 case -1: //error case
	    perror("Hull Breach!");
	    exit(1);
	    break;
	 case 0: // child case (this is where all communication happens)      
	    j = 0;
	    i = 1;
	    while(i){ //keep running till have sent and received everything 
	       // Get the message from the client and display it
	       memset(buffer, '\0', 70005);
	       charsRead = recv(establishedConnectionFD, buffer, 1, 0); // Read the client's message from the socket
	       if (charsRead < 0) error("ERROR reading from socket");
	       //see what we got
	       if(strcmp(buffer, "0") == 0){
		  charsRead = send(establishedConnectionFD, "wrong server@", 13, 0);
		  i = 0; //end the loop since its the wrong connection
	       }else if(strcmp(buffer, "1") == 0){
		  charsRead = send(establishedConnectionFD, "correct server@", 15, 0);
	       }else{
		  //see if should encrypt
		  if(j != 0){
		     memset(message, '\0', 70005);
		     strcpy(message, buffer);
		     while(strstr(message, "@") == NULL){
			charsRead = recv(establishedConnectionFD, buffer, 1, 0); // Read the client's message from the socket
			strcat(message, buffer);
		     }
		     message[strcspn(message, "@")] = '\0'; // Remove the trailing @@ from string
		     encryptString(message, key, encryptedString); //encrypt the string
		     strcat(encryptedString, "@\0");
		     charsRead = send(establishedConnectionFD, encryptedString, strlen(encryptedString), 0); //send the ecryption back
		     if (charsRead < 0) error("ERROR reading from socket");
		     i = 0;
		  }
		  //if its the first arguement put it in the key
		  if(j == 0){
		     strcpy(key, buffer);
		     while(strstr(key, "@") == NULL){
			charsRead = recv(establishedConnectionFD, buffer, 1, 0); // Read the client's message from the socket
			strcat(key, buffer);
		     }
		     key[strcspn(key, "@")] = '\0'; // Remove the trailing @@ from string
		     j = 1; //say we got the key now
		  }
	       }
	    }
	    close(establishedConnectionFD); // Close the existing socket which is connected to the client
	    exit(0);
	 default: //parent, just hang out and clean up child
	    waitpid(spawnPid, &childExitMethod, 0);
      }
   }

   close(listenSocketFD); // Close the listening socket
   return 0; 
}

void encryptString(char* buffer, char* key, char* encryptedString){
   int i = 0, num1, num2, loc;
   char alpha[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   memset(encryptedString, '\0', 70005); //clear out the buffer
   for(i; i < strlen(buffer); i++){
      //buffer spacing
      if(buffer[i] == ' ') num1 = 26; //if space just be this character
      else num1 = buffer[i] - 'A';
      //key spacing
      if(key[i] == ' ') num2 = 26; //if spcace just be this character
      else num2 = key[i] - 'A';

      //get loc by addition
      loc = num1 + num2; 
      encryptedString[i] = alpha[loc % 27]; //no need to worry about negatives here
   }
}
