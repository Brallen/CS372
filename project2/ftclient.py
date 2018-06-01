#### CODE WAS USED FROM THE SLIDES ####
#### THIS PROGRAM WAS TESTED ON FLIP3 ####
from socket import *
from sys import argv

def clientConnector(hostname, serverPort, command, filename, dataPort):
	#set up the client to connect to the server
	clientSocket = socket(AF_INET, SOCK_STREAM)
	clientSocket.connect((hostname, serverPort))
	clientSocket.send(str(dataPort) + '@')
	bit = clientSocket.recv(1)
	clientSocket.send(str(command) + '@')
	print(bit)
	#clientSocket.close()



def dataServer(dataPort):
	#set up the socket to listen
	serverSocket = socket(AF_INET, SOCK_STREAM)
	serverSocket.bind(('',dataPort))
	serverSocket.listen(1)
	print 'The server is ready on port %d' % dataPort

	while(1): #start listening for connections
		connectionSocket, addr = serverSocket.accept() #block and wait for connection
		print 'connection established' #let user know a connection has been made
		recText = ""
		while 1: #while we have not send or received a \quit do stuff
			recText = connectionSocket.recv(550) #receive message
			if recText.find("\quit") == -1: #check to see if it is a disconnect message
				#if its not do all this 
				print recText
			break
				
		connectionSocket.close() #close the connection and then go back to listening

if __name__ == "__main__":
	#args: <SERVER_HOST>, <SERVER_PORT>, <COMMAND>, if -g:(<FILENAME>), <DATA_PORT>
	if len(argv) == 5 and argv[3] == '-l': #the number of commands for -l
		hostname = argv[1]
		serverPort = int(argv[2])
		command = argv[3] #not turned to an int because its the command
		dataPort = int(argv[4])
		filename = '!!!' #mark that there is no filename and want -l
	elif len(argv) == 6 and argv[3] == '-g': #the number of commands for -g
		hostname = argv[1]
		serverPort = int(argv[2])
		command = argv[3] #not turned to an int because its the command
		filename = argv[4] #not an int cause its the filename
		dataPort = int(argv[5])
	else:
		print 'Wrong arguments supplied'
		exit(1)

	clientConnector(hostname, serverPort, command, filename, dataPort) #setup initial connection to server
