/*********************************************************************
** Author: James Hippler (HipplerJ)
** Oregon State University
** CS 372-400 (Winter 2018)
** Introduction to Computer Networks
**
** Description: Programming Assignment #2 (Project 2)
** Due: Sunday, March 11, 2018
**
** Filename: ftserver.c
**
** Objectives:
** 1.) Implement 2-connection client-server network application
** 2.) Practice using the sockets API
** 3.) Refresh programming skills
**
** EXTERNAL RESOURCES
** Learned how to program some of the C/C++ Client side code from the below website
** SOME OF THE CODE BELOW WAS TAKEN DIRECTLY FROM THE BEEJ SOCKET GUIDE
** https://beej.us/guide/bgnet/html/multi/index.html
** https://stackoverflow.com/questions/504810/how-do-i-find-the-current-machines-full-hostname-in-c-hostname-and-domain-info
** ftp://gaia.cs.umass.edu/pub/kurose/ftpserver.c
** https://cboard.cprogramming.com/c-programming/169902-getnameinfo-example-problem.html
** https://codethecode.wordpress.com/2013/04/25/simulation-of-ls-command/
** http://www.sanfoundry.com/c-program-list-files-directory/
** https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
** https://stackoverflow.com/questions/41653419/c-store-a-list-of-files-in-a-directory-into-an-array
** https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
** https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
** https://www.geeksforgeeks.org/c-program-print-contents-file/
** https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c
** https://stackoverflow.com/questions/7856741/how-can-i-load-a-whole-file-into-a-string-in-c
** https://www.programiz.com/c-programming/examples/read-file
** http://www.cplusplus.com/reference/cstdio/fgets/
** https://www.ietf.org/rfc/rfc959.txt
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// Global variable declarations
#define BACKLOG 10
#define MESSAGE_BUFFER 1024
#define DATA_BUFFER 10000
#define FLAG_NUMBER 0

// Function prototyping for all function called throughout operations
void confirmExecution(int);
void displayConnectionDetails(char **);
struct addrinfo * getAddrResults(char *);
struct addrinfo * getFTPAddrResults(char *, char *);
int createSock(struct addrinfo *);
void bindSock(int, struct addrinfo *);
void igniteSock(int, struct addrinfo *, char *);
void connectSock(int, struct addrinfo *);
void connectionPending(int, char **);
void *get_in_addr(struct sockaddr *);
void listOrShipFiles(int, char *, char *, char **);
int createFTPSocket(char *, char *);
void sendFiles(int, char *, char *, char *, char *, int);
void listFiles(int, char *, char *, char *, int);
void incorrectArgument(int, char *, char *, char *, char **);
void FTPsockIgnition(int, struct addrinfo *);
void printPWD(int);
void listPWD(int);
void printFile(int, char *, int);


/************************************************************************
* Description: main function
* Calls the necessary function to orchestrate the program,
* connect with client application, and list/transfer files.
*************************************************************************/

int main(int argc, char *argv[]) {
	char * portNumber = argv[1];
	confirmExecution(argc);
	displayConnectionDetails(argv);
	struct addrinfo * results = getAddrResults(portNumber);
	int sockfd = createSock(results);
	bindSock(sockfd, results);
	igniteSock(sockfd, results, portNumber);                                  		// Create the socket and prepare for TCPconnnection
	connectionPending(sockfd, argv);																							// Wait for connection
	return 0;																																			// End the program
}

/************************************************************************
* Description: confirmExecution
* Function confirms that the user has input the correct amount of
* arguments into the command line.  Function is passed the argc value
* from main and has no return value.
*************************************************************************/

void confirmExecution(int argc) {
	// printf("\nYou are in the confirmExecution() Function\n");
	printf("You Provided %d command line arguments\n", argc);

	if(argc != 2) {                                                               // If the array does not equal 3 then print and error messages to the screen
    fprintf(stderr, "Invalid script execution!\n\n");
    printf("Expected Input:\n");
    printf("./<file_name> <port_number>\n");
    printf("\nExample:\n");                                                     // Provide user with a proper usage example
    printf("./ftserver 3737\n");
    printf("\nExiting the Program\n");                                          // Prompt user that the connection is being closed.
    printf("Please re-run with the correct parameters\n");
    exit(1);                                                                    // Exit the program and return the user to the command line
  } else {                                                                      // Else print a message saying the correct number of commands were issued.
    printf("Correct amount of arguments provided!\n\n");
  }
}

/************************************************************************
* Description: displayConnectionDetails
* Function receives the command line argument information from when
* the program was executed. Uses this information to display the
* connection details to the user.  No returned variables
*************************************************************************/

void displayConnectionDetails(char **argv) {
		// printf("\nYou are in the displayConnectionDetails() Function\n");
		char hostname[MESSAGE_BUFFER];
		gethostname(hostname, MESSAGE_BUFFER);

		printf("Script Name:\t%s\n", argv[0]);																			// Display the script name that was executed
		printf("Hostname:\t%s\n", hostname);																				// Display the server's hostname
		printf("Port Number:\t%s\n\n", argv[1]);																		// Display the communications port number where the server is listening for connections
}

/************************************************************************
* Description: struct addrinfo *getAddrResults
* Functions creates a linked list with the address and the port number.
* Function receives the address and the port number and return a linked
* list.
*************************************************************************/

struct addrinfo *getAddrResults(char *portnum){
	// printf("\nYou are in the getAddrResults() Struct\n");
 	struct  addrinfo *results,
          hints;
  int     status;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;                                                    // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;																							// TCP stream sockets
	hints.ai_flags = AI_PASSIVE;																									// Fill in my IP for me

	status = getaddrinfo(NULL, portnum, &hints, &results);

	if(status != 0){
 		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));               // If an error occurred.  Print the received error message by calling the gai_strerror function
 		exit(1);                                                                    // Exit the program and return the user to the command prompt
 	}

 	return results;                                                               // Return the results to the main function
}

/************************************************************************
* Description: struct addrinfo *getAddrResults
* Functions creates a linked list with the address and the port number.
* Function receives the address and the port number and return a linked
* list.
*************************************************************************/

struct addrinfo *getFTPAddrResults(char *input_addr, char *portnum){
  // printf("IN THE getAddrResults FUNCTION\n");                                // Used for testing.  DELETE once complete.
 	struct  addrinfo *results,
          hints;
  int     rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;                                                    // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;

  rv = getaddrinfo(input_addr, portnum, &hints, &results);
 	if(rv != 0){
 		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));               // If an error occurred.  Print the received error message by calling the gai_strerror function
 		exit(1);                                                                    // Exit the program and return the user to the command prompt
 	}
 	return results;                                                               // Return the results to the main function
}

/************************************************************************
* Description: createSocket
* Program calls the socket function and assigns the return value
* to an integer variable.  This is function is used to create the
* initial socket using the Socket API.  Function has error handling
* to confirm that the appropriate return message is received from the
* socket function.  Once complete, the sockfd variable with the socket
* return information is handed back to the main function.
*************************************************************************/

int createSock(struct addrinfo *results){
	// printf("\nYou are in the createSock() Function\n");
	int sockfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	if (sockfd == -1){                                                            // If return value from the socket function is -1
    fprintf(stderr, "ERROR: Failed CREATING Socket!\n");                        // Notify the user that the porgram was unable to create the socket
		close(sockfd);
		exit(1);                                                                    // Exit the program
  } else {
    printf("SUCCESS: Socket has been Created!\n");
  }
	return sockfd;                                                                // Return the sockfd information to main function for later use.
}

/************************************************************************
* Description: bindSock
* Function binds the socket information.  If there is a failure, an
* error will be executed and the program will terminate.
*************************************************************************/

void bindSock(int sockfd, struct addrinfo * res) {
	// printf("\nYou are in the bindSock() Function\n");
	int binding = bind(sockfd, res -> ai_addr, res -> ai_addrlen);
	if (binding == -1) {
		close(sockfd);																															// Close the communication socket, if opened
		fprintf(stderr, "ERROR: Failed BINDING Socket!\n");													// Print Error message
		exit(1);																																		// Terminate program
	} else {																																			// Else
		printf("SUCCESS: Socket Binding Established!\n");														// Print the success statement.
	}
}

/************************************************************************
* Description: igniteSock
* Function receives the socket information and the linked list
* in order to create a connection on the socket.  There's error handling
* incase the connection functionreturns an status code of -1.  In the
* event that this happens, the function will produce and error and
* terminate the program's execution.
*************************************************************************/

void igniteSock(int sockfd, struct addrinfo *res, char *portNumber){
	// printf("\nYou are in the igniteSock() Function\n");
	int statusCode = listen(sockfd, BACKLOG);
	if (statusCode == -1){                                                        // If the connection function returns a value of -1
		close(sockfd);
		printf("DO WE MAKE IT HERE!");
		fprintf(stderr, "ERROR: Failed CONNECTING Socket!\n");                      // Display an error message to the user saying the connection failed
		exit(1);                                                                    // Terminate the program
	} else {
    printf("SUCCESS: Socket Connection Established!\n");
		printf("Server open on %s\n\n", portNumber);
  }
}

/************************************************************************
* Description: FTPsockIgnition
* Function ignites the FTP Socket connection and ensures that it
* is established correctly.  If the FTP socket fails to connect,
* an error message will be presented.
*************************************************************************/

void FTPsockIgnition(int ftpsock, struct addrinfo *res){
	// printf("\nYou are in the FTPsockIgnition() Function\n");
	int statusCode = connect(ftpsock, res->ai_addr, res->ai_addrlen);
	if (statusCode == -1){																	                      // If the connection function returns a value of -1
		fprintf(stderr, "ERROR: Failed CONNECTING Socket!\n");                      // Display an error message to the user saying the connection failed
		close(ftpsock);																															// Close the data connection socket.
		// exit(1);                                                                    // Terminate the program
	} else {																																			// If the socket connection was successfully established
    printf("SUCCESS: Socket Connection Established!\n\n");                      // Print the username for the client side of the chat client
  }
}

/************************************************************************
* Description: connectionPending
* After initial communication socket is created, the server
* will sit in a loop until a connection is received from the client
* Once a connection is received, this function will call listOrShipFiles
* to determine how to process the client's request.
*************************************************************************/

void connectionPending(int sockfd, char **argv){
	// printf("\nYou are in the connectionPending() Function\n");
	int 		new_fd;																																// Integer to create a new connection
	char 		ipaddress[INET6_ADDRSTRLEN],																					// Character string to hp
					hostname[INET6_ADDRSTRLEN];
  struct 	sockaddr_storage their_addr; 																					// connector's address information

	socklen_t sin_size;

	printf("Awaiting for Client Connections...\n");
	while(1) {  																																	// main accept() loop
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {																													// If connection is established
			perror("accept");
			printf("Connection ACCEPTED from Client");																// Print to screen
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), ipaddress, sizeof ipaddress);
		getnameinfo((struct sockaddr *)&their_addr, sizeof(their_addr), hostname, sizeof(hostname), NULL, FLAG_NUMBER, NI_NAMEREQD);
		printf("\nConnection ESTABLISHED from %s (%s)\n", hostname, ipaddress);
		listOrShipFiles(new_fd ,hostname, ipaddress, argv);													// Call the listOrShipFiles function to process client request
		close(new_fd);																															// close communication socket with client
		// printf("\nClosing communication SOCKET in connectionPending\n");
		printf("\nConnection CLOSED from %s (%s)\n", hostname, ipaddress);					// Display notification that the connection was closed
		printf("\nWaiting for another client to connect...\n");											// Return to waiting for connection state.
	}
}

/************************************************************************
* Description: get_in_addr
* The below function was taken directly from the Beej Socket Networking
* guide and is used to translate the client's socket information into
* a useable IP Address
*************************************************************************/

void *get_in_addr(struct sockaddr *sa)																					// get sockaddr, IPv4 or IPv6:
{
	// printf("\nYou are in the get_in_addr() Function\n");
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/************************************************************************
* Description: listOrShipFiles
* Function handles the FTP aspects of the program and calls functions
* depending on arguments provided by the client.  Takes the socket connection
* information, the hostname, ip information, and command line arguments.
* No return value to main function.
*************************************************************************/

void listOrShipFiles(int new_fd, char *hostname, char *ipaddress, char **argv){
	// printf("\nYou are in the listOrShipFiles() Function\n");
	int 	sockftp;																																// Create integer variable that will be used when creating the second FTP Socket
	char 	dataPort[MESSAGE_BUFFER],																								// Character Array to store data port for FTP connection
				argument[MESSAGE_BUFFER],																								// Character Array to store command line argument
				fileName[MESSAGE_BUFFER],																								// Character Array to store File Name
				received[MESSAGE_BUFFER] = "received",																	// Character Array with Server Generic Response
				acknowledge[MESSAGE_BUFFER];

	memset(dataPort,0,sizeof(dataPort));																					// Clear dataPort Character Array
	recv(new_fd, dataPort, MESSAGE_BUFFER, FLAG_NUMBER);													// Receive message from client and assign to dataPort Character Array
	printf("Data Port: %s\n", dataPort);																					// Display Data Port information to the monitor
	send(new_fd, received, strlen(received), FLAG_NUMBER);												// Send response to client showing that their message was received

	memset(argument,0,sizeof(argument));																					// Clear argument Character Array
	recv(new_fd, argument, MESSAGE_BUFFER, FLAG_NUMBER);													// Receive message from client and assign to argument Character Array
	printf("CLI Argument: %s\n", argument);																				// Display CLI Argument to the monitor
	send(new_fd, received, strlen(received), FLAG_NUMBER);												// Send response to client showing that their message was received

	if(strcmp(argument,"-g") == 0){																								// If the CLI Argument is -g then do the next few steps
		memset(fileName,0,sizeof(fileName));																				// Clear fileName Character Array
		recv(new_fd, fileName, MESSAGE_BUFFER, FLAG_NUMBER);												// Receive message from client and assign to fileName Character Array
		printf("File Name: %s\n", fileName);																				// Display File Name to the monitor
		send(new_fd, received, strlen(received), FLAG_NUMBER);											// Send response to client showing that their message was received
	}
	printf("\n");

	if((strcmp(argument, "-l") == 0) || (strcmp(argument, "-g") == 0)){						// If the CLI Argument is either -l or -g then do the following steps
		sleep(1);																																		// For some reason this was not necessary on my desktop.  I couldn't find another way to give the python server enough time to create and FTP connection.
		sockftp = createFTPSocket(ipaddress, dataPort);															// Call function to create new FTP Socket connectio to transfer data
		// memset(acknowledge,0,sizeof(acknowledge));																	// Clear Acknowledgement Character Array
		// recv(new_fd, acknowledge, MESSAGE_BUFFER, FLAG_NUMBER);
		// send(new_fd, received, strlen(received), FLAG_NUMBER);
		if(strcmp(argument,"-l") == 0){																							// If the CLI Argument is -l
			listFiles(new_fd, hostname, ipaddress, dataPort, sockftp);								// Call the listFile Function where directory contents are presented
			close(sockftp);
			// printf("Closing FTP Socket in listOrShipFiles");
		}
		if(strcmp(argument,"-g") == 0){																							// If the CLI Argument is -g
			sendFiles(new_fd, hostname, ipaddress, fileName, dataPort, sockftp);			// Call the sendFiles Function where a files is transfered to the client
			close(sockftp);
			// printf("Closing FTP Socket in listOrShipFiles");
		}
	} else {																																			// If client did not send either -g or -l in their argument
		incorrectArgument(new_fd, argument, hostname, ipaddress, argv);							// Call the incorrectArgument function and send an error message to client
	}
	// close(sockftp);																																// Close the FTP Data socket connection with the client once actions are completed
}

/************************************************************************
* Description: createFTPSocket
* Function is called when a new FTP Socket is required.  It's only
* application is to call other functions in the appropriate order to
* create the data socket.  It returns the new socket information as
* an integer.  It receives the client specified dataport and the
* client's ip information.
*************************************************************************/

int createFTPSocket(char *ipaddress, char *dataPort){
	// printf("\nYou are in the createFTPSocket() Function\n");
	struct addrinfo * results = getFTPAddrResults(ipaddress, dataPort);						// Call getFTPAddrResults function and assign return to structure
	int sockftp = createSock(results);																						// Call the createSock function and assign return to integer
	FTPsockIgnition(sockftp, results);																						// Call the FTPsockIgnition function to kick start the FTP Socket
	return sockftp;																																// Return the FTP socket information to the calling function
}

/************************************************************************
* Description: listFiles
* Function prepares client and server to send directory information
* across the FTP Socket.  It receives the communication and data socket
* information the specified data port information, as well as the
* client's networking information.  No returned variables.
*************************************************************************/

void listFiles(int new_fd, char *hostname, char *ipaddress, char *dataPort, int sockftp){
	// printf("\nYou are in the listFiles() Function\n");
	char 	listing[] = "Listing Directory Files...";																// Character array initiailzed to server response
	char	respond[] = "received...";																							// Character array initialized to server response
	char 	received[MESSAGE_BUFFER];																								// Character array to store client's response
	printf("List directory requested by %s (%s) on port %s\n", hostname, ipaddress, dataPort);
	send(new_fd, listing, strlen(listing), FLAG_NUMBER);													// Send initial response
	printPWD(sockftp);																														// Call the printPWD() function to get name of PWD
	memset(received,0,sizeof(received));																					// reset character array for client reponse
	recv(new_fd, received, MESSAGE_BUFFER, FLAG_NUMBER);													// Receive client Acknowledgement
	send(new_fd, respond, strlen(respond), FLAG_NUMBER);													// Send server Acknowledgement
	listPWD(sockftp);																															// Call function to list files in PWD
	memset(received,0,sizeof(received));																					// reset character array for client response
	recv(new_fd, received, MESSAGE_BUFFER, FLAG_NUMBER);													// Receive client Acknowledgement
	send(new_fd, respond, strlen(respond), FLAG_NUMBER);													// Send server Acknowledgement
	// close(sockftp);
	// close(new_fd);
}

/************************************************************************
* Description: sendFiles
* Function prepares client and server to send file across the FTP
* Socket.  It receives the communication and data socket information
* the specified data port information and file names, as well as the
* client's networking information.  No returned variables.
*************************************************************************/

void sendFiles(int new_fd, char *hostname, char *ipaddress, char *fileName, char *dataPort, int sockftp){
	// printf("\nYou are in the sendFiles() Function\n");
	char sending[] = "Sending Requested File...";																	// Character array initialized with response
	char 	received[MESSAGE_BUFFER];																								// Character array to store client response
	printf("File %s requested by %s (%s) on port %s\n", fileName, hostname, ipaddress, dataPort);
	send(new_fd, sending, strlen(sending), FLAG_NUMBER);													// Send Acknowledgement to client
	memset(received,0,sizeof(received));																					// Reset client response character array
	recv(new_fd, received, MESSAGE_BUFFER, FLAG_NUMBER);													// Receive client response and store
	printFile(sockftp, fileName, new_fd);																					// Call the printFile() function to actually process sending the file.
}

/************************************************************************
* Description: incorrectArgument
* Function is called when the client server has sent a CLI argument
* that is not supported by the server.  The function receives the
* communication socket information the command line arguments, the argument
* from the client, and the client's networking information.  No return
* variables here.
*************************************************************************/

void incorrectArgument(int new_fd, char *argument, char *hostname, char *ipaddress, char **argv){
	// printf("\nYou are in the incorrectArgument() Function\n");
	char 	bye[MESSAGE_BUFFER],
				errorMessg[] = "\nThe command line argument is NOT valid\nThe Script Modifier must be either 'l' or '-g'\n";
	send(new_fd, errorMessg, strlen(errorMessg), FLAG_NUMBER);										// Send the error messages to the client on communication socket
	memset(bye,0,sizeof(bye));																										// Reset response character array
	recv(new_fd, bye, MESSAGE_BUFFER, FLAG_NUMBER);																// Receive confirmation from the client
	printf("Invalid Command input by %s (%s) on port %s\n", hostname, ipaddress, argv[1]);
}

/************************************************************************
* Description: printPWD
* Function collects the present working directory name and stores it
* as a character string.  The character string is returned to the calling
* function.  No variables recevied when called.
*************************************************************************/

void printPWD(int sockftp){
	// printf("\nYou are in the printPWD() Function\n");
	char pwd[MESSAGE_BUFFER];																											// Create a Character array
	memset(pwd,0,sizeof(pwd));																										// Reset the memory blocks for the character array
	if (getcwd(pwd, sizeof(pwd)) == NULL){																				// If the getcwd function returns NULL
		perror("ERROR: Could not determine Directory Name");												// Output an Error
	} else {																																			// Else
		send(sockftp, pwd, strlen(pwd), FLAG_NUMBER);																// Send the PWD Name to client via the FTP Data Connection
	}
}

/************************************************************************
* Description: listPWD
* Function reads the directory contents line by line and
* concatinates each line to the end of a string.  Once all the
* contents of the directory are read, the string is sent to the
* FTP client. Function takes the FTP Data Socket information and
* Returns nothing to the calling function.  Initially I tried to create
* an Array of Strings but I couldn't figure out how to
*************************************************************************/

void listPWD(int sockftp){
	// printf("\nYou are in the listPWD() Function\n");
	char pwdList[DATA_BUFFER];																										// Create Character Array
	struct dirent * ep;																														// Create Structure
	DIR * dp = opendir(".");																											// Open the directory
	memset(pwdList,0,sizeof(pwdList));																						// Clear memory blocks for character array
	if (dp){																																			// If the directory could be found
		while ((ep = readdir(dp)) != NULL){																					// and while the line are not null
			if (ep->d_type == DT_REG){
				strcat(pwdList, ep->d_name);																						// Copy directory line to string (Contactinate)
				strcat(pwdList, "\n");																									// Add break line to end of each string
			}
		}
		closedir(dp);																																// Close the directory
	}
	pwdList[strlen(pwdList)-1] = 0;																								// Remove the trailing break line from the string
	send(sockftp, pwdList, strlen(pwdList), FLAG_NUMBER);													// Send final string to client via FTP socket
}

/************************************************************************
* Description: printFile
* Function is passed the client specified file name and the Data and
* Communication sockets.  It returns no variables.  When the function
* is executed, it attempts to open the specified file.  If found, it
* reads through one character at a time and add it to a character
* array.  When the array reach 1024 bytes, it sent to the client and reset
*************************************************************************/

void printFile(int sockftp, char *fileName, int new_fd){
	// printf("\nYou are in the printFile() Function\n");
	char	fileNotFound[] = "Unable to locate input File Name!\nPlease Try again!";// Response Character array
	char	complete[] = "--File Transfer Complete--";															// Reponse Character array
	char  letter;																																	// Character variable to increment through file
	char 	fileArray[MESSAGE_BUFFER + 1];																					// Array for storing 1024 bytes from the file
	int		counter = 0;																														// Counter to increement through fileArray character array
	char	respond[] = "success";																									// Response Character array
	char 	received[MESSAGE_BUFFER];																								// Character array to store response from client

	FILE *readf = fopen(fileName, "r");																						// Open the specified file for reading
  if (readf == NULL){																														// If the file cannot be found
    printf("Cannot open file %s\n", fileName);																	// Output and error
		send(new_fd, fileNotFound, strlen(fileNotFound), FLAG_NUMBER);							// Let the client know as well so they can close down and restart
		memset(received,0,sizeof(received));
		recv(new_fd, received, MESSAGE_BUFFER, FLAG_NUMBER);
  } else {																																			// if the file is found and opened
		send(new_fd, respond, strlen(respond), FLAG_NUMBER);												// Let the client know
		memset(received,0,sizeof(received));																				// Clear response array memory blocks
		recv(new_fd, received, MESSAGE_BUFFER, FLAG_NUMBER);												// Wait for their response that they are ready to receive data
		letter = fgetc(readf);																											// Assign letter variable to first character in the file

		while (letter != EOF){																											// While we have not reached the end of the file
			do {
				fileArray[counter] = letter;																						// Assign the letter to the array
				counter ++;																															// Move forward in the array
				letter = fgetc(readf);																									// Move forward in the file
			} while (counter < MESSAGE_BUFFER && letter != EOF);											// Continue doing this until the array is full or the end of the file is reached
			send(sockftp, fileArray, strlen(fileArray), FLAG_NUMBER);									// Send the array to the client via the FTP socket
			memset(fileArray,0,sizeof(fileArray));																		// Reset the file array
			memset(received,0,sizeof(received));																			// Reset the received response array
			recv(new_fd, received, MESSAGE_BUFFER, FLAG_NUMBER);											// Receive the response from the client that they have received the data and are ready for more
			send(new_fd, respond, strlen(respond), FLAG_NUMBER);                      // Acknowledge response
			counter = 0;																															// Reset counter to the beginning of the array again.
		}
		memset(received,0,sizeof(received));																				// Reset the received response array
		recv(new_fd, received, MESSAGE_BUFFER, FLAG_NUMBER);												// Receive the response from the client that they have received the data and are ready for more
		send(new_fd, respond, strlen(respond), FLAG_NUMBER);												// Acknowledge response
		send(sockftp, "--File Transfer Complete--", sizeof(complete), FLAG_NUMBER);	// Send Sentinel attribute to let the client know that we are done sending file contents.
		fclose(readf);																															// Close the FTP Socket and return to waiting on connections
	}
}
