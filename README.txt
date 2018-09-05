Author: James Hippler (HipplerJ)
Oregon State University
CS 372-400 (Winter 2018)
Introduction to Computer Networks

Description: Programming Assignment #2 (Project 2)
Due: Sunday, March 11, 2018

Filenames:
- ftclient.py
- ftserver.c
- README.txt

Objectives:
1.) Implement 2-connection client-server network application
2.) Practice using the sockets API
3.) Refresh programming skills

EXTERNAL RESOURCES

Learned how to program some of the Python Client side code from the below websites
https://docs.python.org/release/2.6.5/library/socketserver.html
https://docs.python.org/2/library/ftplib.html
https://stackoverflow.com/questions/4005507/simple-code-for-ftp-server-in-python
http://twistedmatrix.com/documents/current/core/examples/#auto8
https://stackoverflow.com/questions/5214578/python-print-string-to-text-file
https://stackoverflow.com/questions/11555468/how-should-i-read-a-file-line-by-line-in-python

Learned how to program some of the C/C++ Client side code from the below website
SOME OF THE CODE BELOW WAS TAKEN DIRECTLY FROM THE BEEJ SOCKET GUIDE
https://beej.us/guide/bgnet/html/multi/index.html
https://stackoverflow.com/questions/504810/how-do-i-find-the-current-machines-full-hostname-in-c-hostname-and-domain-info
ftp://gaia.cs.umass.edu/pub/kurose/ftpserver.c
https://cboard.cprogramming.com/c-programming/169902-getnameinfo-example-problem.html
https://codethecode.wordpress.com/2013/04/25/simulation-of-ls-command/
http://www.sanfoundry.com/c-program-list-files-directory/
https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
https://stackoverflow.com/questions/41653419/c-store-a-list-of-files-in-a-directory-into-an-array
https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
https://www.geeksforgeeks.org/c-program-print-contents-file/
https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c
https://stackoverflow.com/questions/7856741/how-can-i-load-a-whole-file-into-a-string-in-c
https://www.programiz.com/c-programming/examples/read-file
http://www.cplusplus.com/reference/cstdio/fgets/
https://www.ietf.org/rfc/rfc959.txt

TESTING
Tested the ftp server application on flip1 (flip1.engr.oregonstate.edu) with the client application running
initially on flip1 as well.  Client FTP application also works on flip3 (flip3.engr.oregonstate.edu) and can communicate
to with the server on flip1.

INSTRUCTIONS
1.) Compile the ftserver.c file to create an executable.  You can use the following command which
    will name the output ftserver

      gcc ftserver.c -o ftserver

2.) Execute the server side application so that it will listen for client connections

    ./ftserver <port_number>

    Example:
    ./ftserver 3737

3.) Make the python program executable and start the ftp client application with the command below.
    Specify the hostname and a port number used when you started the server side.  Also include a command
    line argument (either -g to get a file or -l to list directory contents), a file name (if you selected
    the -g option), and a data port for the FTP socket connection.  Example commands listed below.

      chmod +x ftclient.py

      To List Present Working Directory Contents
      ./ftclient.py <server_hostname> <port_number> -l <data_port>

      Example:
      ./ftclient.py flip3.engr.oregonstate.edu 3737 -l 3739

      To Transfer a File from Server to your current directory
      ./ftclient.py <server_hostname> <port_number> -g <file_name> <data_port>

      Example:
      ./ftclient.py flip3.engr.oregonstate.edu 3737 -g filename.txt 3739

4.) The FTP Server will sit idle until a connection is established from a client.
    Once this happens, The FTP server will process the information you Entered into
    the command line and will either send the directory contents (if -l was input)
    or the contents of a file (if -g was selected).  If an incorrect cli argument
    was input, the server will return an error message.

5.) After processing request the client application will terminate and the server
    will return to waiting on another connection.

6.) To end the server application, use the keyboard interrupt command (Crtl + C)
