#!/usr/bin/env python
#### CODE WAS USED FROM THE SLIDES ####
#### THIS PROGRAM WAS TESTED ON FLIP3 ####
from socket import *
from sys import argv

if len(argv) > 1: #if a port was specified use that one
	serverPort = int(argv[1])
else:
   	serverPort = 42069 #otherwise use this one

#get the username and make sure its not too long
username = raw_input('Enter username: ')
if len(username) > 10:
	print 'name is too long'
	exit(1)

#set up the socket to listen
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('',serverPort))
serverSocket.listen(1)
print 'The server is ready on port %d' % serverPort

while(1): #start listening for connections
	connectionSocket, addr = serverSocket.accept() #block and wait for connection
	print 'connection established' #let user know a connection has been made
	recText = ""
	while 1: #while we have not send or received a \quit do stuff
		recText = connectionSocket.recv(550) #receive message
		if recText.find("\quit") == -1: #check to see if it is a disconnect message
			#if its not do all this 
			print recText
			
			#creat and send message to client
			sendText = raw_input(username + ': ')
			if len(sendText) > 500:
				sendText = sendText[:500]
			connectionSocket.send(username + ': ' + sendText)
			#look to see if we are sending a disconnect
			if sendText.find("\quit") != -1:
				print 'connection ended'
				break #if we are then break out of the loop to close connection
		else:
			#if it was a disconnect message then break out the loop
			print 'client has left the chat' 
			break
			
	connectionSocket.close() #close the connection and then go back to listening
