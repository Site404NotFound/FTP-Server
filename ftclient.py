#!/usr/bin/python
"""
/*********************************************************************
** Author: James Hippler (HipplerJ)
** Oregon State University
** CS 372-400 (Winter 2018)
** Introduction to Computer Networks
**
** Description: Programming Assignment #2 (Project 2)
** Due: Sunday, March 11, 2018
**
** Filename: ftclient.py
**
** Objectives:
** 1.) Implement 2-connection client-server network application
** 2.) Practice using the sockets API
** 3.) Refresh programming skills
**
** EXTERNAL RESOURCES
** Learned how to program some of the Python Client side code from the below websites
** https://docs.python.org/release/2.6.5/library/socketserver.html
** https://docs.python.org/2/library/ftplib.html
** https://stackoverflow.com/questions/4005507/simple-code-for-ftp-server-in-python
** http://twistedmatrix.com/documents/current/core/examples/#auto8
** https://stackoverflow.com/questions/5214578/python-print-string-to-text-file
** https://stackoverflow.com/questions/11555468/how-should-i-read-a-file-line-by-line-in-python
*********************************************************************/
"""

import socket  		   												 			# Import the python socket module
import select          	 														# Import the python select module
import sys              														# Import the python sys module
import os               														# Import the pythonj os module
from thread import *    														# Imported to implement threading

TOTALBYTES = 1024                                                               # Set a global variable for the total bytes

"""
/************************************************************************
* Description: confirmExecution
* Confirms that the user provided the correct amount of arguments
* with their command line input.  Also determines what modifiers
* were specified.  The function takes the sys.argv array from the system
* and no returned values to main.
*************************************************************************/
"""

def confirmExecution(argv):
    if len(argv) < 5 or len(argv) >6:                                           # If the length is too long or too short output and error message
        print("\nInvalid script execution!")
        print("Incorrect Number of Arguments")
        print("\nExpected Input:")
        print("python <script_name> <server_name> <server_port> <modifier> <data_port>")
        print("Example:")
        print("python ftclient.py flip.engr.oregonstate.edu 3737 -l 3738")
        print("\nExiting the program!")
        print("Please re-run with the correct parameters!")
        exit()                                                                  # terminate the program's execution

    if argv[3] == "-l" and len(argv) != 5:                                      # Confirm that the user input 4 arguments when implementing the -l modifier
        print("\nInvalid script execution!")                                    # Display error message if not
        print("Five arguments required when using the '-l' modifier to list files")
        print("\nExpected Input:")
        print("python <script_name> <server_name> <server_port> <modifier> <data_port>")
        print("Example:")
        print("python ftclient.py flip.engr.oregonstate.edu 3737 -l 3738")
        print("\nExiting the program!")
        print("Please re-run with the correct parameters!")
        exit()                                                                  # Terminate the program's execution

    if argv[3] == "-g" and len(argv) != 6:                                      # Confirm that the user input 5 arguments when implementing the -g modifier
        print("\nInvalid script execution!")                                    # Display error messag if not
        print("Six arguments required when using the '-g' modifier to download files")
        print("\nExpected Input:")
        print("python <script_name> <server_name> <server_port> <modifier> <file_name> <data_port>")
        print("Example:")
        print("python ftclient.py flip.engr.oregonstate.edu 3737 -g filename.txt 3738")
        print("\nExiting the program!")
        print("Please re-run with the correct parameters!")
        exit()                                                                  # Terminate the program's execution

    print("You entered {} arguments on the command line".format(len(argv)))     # Display the amount of arguments input by the user
    print("Script Name:     {}".format(argv[0]))                                # Output the input script name
    print("Server Hostname: {}".format(argv[1]))                                # Output the input server hostname
    print("Connection Port: {}".format(argv[2]))                                # Output the input connection port

    if argv[3] == "-l":                                                         # Check if user wants to list the files
        action = "List Server Files"
        dataPortArgument = argv[4]                                              # Set the desired data port to the correct argument
        print("Data Port:       {}".format(argv[4]))                            # Display the list Directories Data Port

    elif argv[3] == "-g":                                                       # Check if the user wants to get a file
        action = "Download Specified File"
        dataPortArgument = argv[5]                                              # Set the desired data port to the correct argument
        print("Data Port:       {}".format(argv[5]))                            # Display the get file data port
        print("Filename:        {}".format(argv[4]))                            # Display the requested filename
    else:
        action = "Invalid Script Modifier"
        dataPortArgument = argv[4]

    print("Script Modifier: {}  ({})".format(argv[3], action))                  # Diplay the script argument that was provided on the command line

    print("\nCorrect amount of arguments provided!")
    print("Starting the connection with FTP server....\n")

    return dataPortArgument                                                     # Return the correct Data Port to the calling function.

"""
/************************************************************************
* Description: sendSetupData
* Funciton sends the appropriate arguments to the server
* so that it can accurately process its request.  No return variable
* information
*************************************************************************/
"""

def sendSetupData(server, argv, dataPort, getOrList):
    print("ESTABLISHING TCP Connection on port {}\n".format(argv[2]))
    server.send(dataPort)
    received = server.recv(TOTALBYTES)
    server.send(getOrList)
    server.recv(TOTALBYTES)

    if getOrList == "-g":                                                       # if the command line option was -g to get a file
        server.send(argv[4])                                                    # send the appropriate command line argument for the data port
        server.recv(TOTALBYTES)

"""
/************************************************************************
* Description: createConnection
* Function creates the initial communications socket between the server
* and the client.  Takes the server hostname and comms port as arguments
* return connection information
*************************************************************************/
"""

def createConnection(argv1, argv2):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)                  # Call the necessary socket API to create the program socket
    server.connect((argv1, int(argv2)))                                         # Create the server connection using the server hostname and port number
    print("Connection successfully ESTABLISHED with {}".format(argv1))
    return server                                                               # Return the server information back to main

"""
/************************************************************************
* Description: createDataConnection
* Function creates the secondary FTP socket between the server and the
* client.  It receives the data port (depending on the CLI argument
* provided) and returns the FTP socket information.
*************************************************************************/
"""

def createDataConnection(dataPort, hostname):
    ftpserver = socket.socket(socket.AF_INET, socket.SOCK_STREAM)               # Call the necessary socket API to create the program socket
    ftpserver.bind(('',dataPort))							                    # Bind the hostname and the port number to the socket
    ftpserver.listen(1)                                                         # listen on socket for 1 connection maximum (additional connections rejected)
    ftpconnect,addr = ftpserver.accept()
    return ftpconnect													        # return the server object that was created.

"""
/************************************************************************
* Description: receiveDirectoryList
* Function receives the Directory listing from sent from the server
* and presents it to the screen.  The function receives/presents both
* the directory path as well as the directory contents.  It takes
* the communications and ftp socket information, the command line arguments
* the data port and the option to list or receive files
*************************************************************************/
"""

def receiveDirectoryList(sock, ftpserver, argv, dataPort, getOrList):
    print(sock.recv(TOTALBYTES))
    print("\nPresent Working Directory:")
    print(ftpserver.recv(TOTALBYTES))                                           # Receive the directory name
    sock.send("received")
    sock.recv(TOTALBYTES)
    print("\nDirectory Contents:")
    print(ftpserver.recv(TOTALBYTES))                                           # Receive the directory contents
    sock.send("received")
    sock.recv(TOTALBYTES)
    print(ftpserver.recv(TOTALBYTES))
    # ftpserver.close()                                                           # Close the FTP connection that was established
    # print("CLOSING OUR FTP CONNECTION")

"""
/************************************************************************
* Description: receiveFiles
* This function was a total nightmare and took up more time than
* any other element of the project. It receives the contents of a file in
* small 1024 bit sections and writes them to a file. It receives the usual
* communication and ftp socket information and all the command line argument
* materials.  No return variable information.
*************************************************************************/
"""

def receiveFile(sock, ftpserver, argv, dataPort, getOrList):
    newFilename = "copy_"+argv[4]                                               # make newfile name the same as the old file but with copy_ at the lead (I was tired of overwritting my files)
    print(sock.recv(TOTALBYTES));
    sock.send("received")
    fileFound = sock.recv(TOTALBYTES)
    sock.send("received")

    if "Unable to locate input File Name!" not in fileFound:                    # If the server could actually find the file
        receivedFile = open(newFilename, "w")                                   # open the new file to write
        incoming = ftpserver.recv(TOTALBYTES - 1)                               # receive the first file section
        sock.send("received")                                                   # Acknowledge
        sock.recv(TOTALBYTES)                                                   # Server Acknowledge
        while True:                                                             # Forever loop until Sentinel value is received
            while "--File Transfer Complete--" not in incoming:                 # While Sentinel value is not present in the received array
                receivedFile.write(incoming)                                    # Write array to file
                sock.send("received")                                           # Acknowledge
                sock.recv(TOTALBYTES)                                           # Server Acknowledge
                incoming = ftpserver.recv(TOTALBYTES - 1)                       # Receive next file transmission
            else:                                                               # If Sentinel message received
                break                                                           # Break out of the loop

        receivedFile.close()                                                    # Close the opened file (save)
        print("\nFile transfer complete")                                       # Tell user that file has been saved
        print("File has been SUCCESSFULLY stored as '{}'".format(newFilename))  # and where it can be found
    else:                                                                       # If file could not be found on server
        print(fileFound)                                                        # Print server error message and terminate
    ftpserver.close()                                                           # Close the FTP connection that was created for data transfer
    # print("CLOSING OUT FTP SOCKET?")

"""
/************************************************************************
* Description: invalidCommand
* Function tells user that they were careless and did not input the
* correct command line information.  FTP connection is never established
* here and it's all pretty simple
*************************************************************************/
"""

def invalidCommand(sock):
    print(sock.recv(TOTALBYTES))                                                # Receive server response and print
    sock.send("Good bye!")                                                      # Send Acknowledgement
    print("Exiting Program!  Please Try again!")                                # exit the program

"""
/************************************************************************
* Description: readytoRock
* Gives the python server time to create a FTP connection.
*************************************************************************/
"""
def readytoRock(sock):
    sock.send("Ready to Party")
    print(sock.recv(TOTALBYTES))

"""
/************************************************************************
* Description: main function
* Program starts here and calls functions as necessary
* Ensures that the user input the correct CLI arguments
* Assigns CLI Arguments to variables to pass to called functions
*************************************************************************/
"""

if __name__ == "__main__":
    dataPort = confirmExecution(sys.argv)                                       # Call the confirmExecution and assign return to variable
    getOrList = sys.argv[3]                                                     # Set the CLI modifier to a variable
    sock = createConnection(sys.argv[1], sys.argv[2]);
    sendSetupData(sock, sys.argv, dataPort, getOrList)                          # Call the sendSetupData and send the create connection returned variable
    if getOrList == "-l" or getOrList == "-g":
        ftpsock = createDataConnection(int(dataPort), sys.argv[1])              # Call createDataConneciton function to create a secnm
        # readytoRock(sock)
        if getOrList == "-l":
            receiveDirectoryList(sock, ftpsock, sys.argv, dataPort, getOrList)
        if getOrList == "-g":
            receiveFile(sock, ftpsock, sys.argv, dataPort, getOrList)
        ftpsock.close()
        # print("CLOSING OUT FTP SOCKET?")
    else:
        invalidCommand(sock)
    sock.close()                                                                # Close communication socket
