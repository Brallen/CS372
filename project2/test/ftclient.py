#### CODE WAS USED FROM THE SLIDES ####
#### THIS PROGRAM WAS TESTED ON FLIP ####
from socket import *
from sys import argv
import os

def clientConnector(hostname, serverPort, command, filename, dataPort):
	#set up the client to connect to the server
	clientSocket = socket(AF_INET, SOCK_STREAM)
	clientSocket.connect((hostname, serverPort))
	clientSocket.send(str(dataPort) + '@')
	bit = clientSocket.recv(1)
	clientSocket.send(str(command) + '@')
	bit = clientSocket.recv(1)
	clientSocket.send(filename + '@')

	if command == "-l":
		printDir(dataPort)
	else:
		getFile(dataPort, filename)
	print 'done'
	clientSocket.close()

def printDir(dataPort):
	#set up the socket to listen
	serverSocket = socket(AF_INET, SOCK_STREAM)
	serverSocket.bind(('',dataPort))
	serverSocket.listen(1)
	print 'The server is ready on port %d' % dataPort

	connectionSocket, addr = serverSocket.accept() #block and wait for connection
	print 'connection established' #let user know a connection has been made

	buffer = connectionSocket.recv(20) #get first file or @@ if there are no .txt files
	while "@@" not in buffer: #while we don't have just @@
		print buffer #print what we got. Printing first means we don't print out the @@
		buffer = connectionSocket.recv(20)

	connectionSocket.close() #close the connection and then go back to listening
	print 'closing data socket'

def getFile(dataPort, filename): #function still  struggles with README.txt but handles all others fine?
	#set up the socket to listen
	serverSocket = socket(AF_INET, SOCK_STREAM)
	serverSocket.bind(('',dataPort))
	serverSocket.listen(1)
	print 'The server is ready on port %d' % dataPort
	
	connectionSocket, addr = serverSocket.accept() #block and wait for connection
	print 'connection established' #let user know a connection has been made

	if os.path.exists(filename):
		if raw_input('File already exists!!! overwrite? Y/n: ').lower() != 'y': #this line will always cancel
			connectionSocket.send('0')
			connectionSocket.close() #close the connection and then go back to listening
			return
	
	connectionSocket.send('1')
	file = open(filename, 'w+')
	fileContents = ""
	buffer = connectionSocket.recv(1) #get first file or @@ if there are no .txt files
	while "@" not in buffer: #while we don't have just @@
		fileContents = fileContents + buffer #add what we got to the total
		file.write(buffer)
		buffer = connectionSocket.recv(1)

	print fileContents
	file.close()
	connectionSocket.close() #close the connection and then go back to listening
	print 'closing data socket'

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
