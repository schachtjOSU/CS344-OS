/*
 * File otp_dec.c
 * Author: Jeffrey Schachtsick
 * Course: CS344 - Operating Systems 1
 * Assignment: Program 4
 * Overview: Connects with a daemon (otp_dec_d), to perform a one-time pad style
 * 	decryption of an encrypted file and key to decrypted file.  This program
 * 	also checks to be sure the encrypted file has valid characters (spaces 
 * 	and A-Z), key file is shorter than the encrypted file, reports bad 
 * 	connection port to the daemon, and output the decryption to stdout.
 * Last Update: 06/03/2016
 * Sources: Operating Systems Lectures 15, 16, and 17.
 *   Beej's Guide to Network Programming - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 *   Some logic based on the program used here from https://github.com/kevinto/cs344-prog4
 *   Sockets Tutorial by Rober Ingalls - http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 */

// Include Libraries
#include <stdio.h>	// General IO, including printf to redirect files
#include <stdlib.h>	// General purpose functions
#include <string.h>	// Manipulation of C strings and arrays
#include <signal.h>	// Handle signals reported during program execution
#include <sys/types.h>	// For networking with sockets
#include <sys/stat.h>	// Returning data with the sockets
#include <fcntl.h>	// File descriptors with a socket
#include <sys/wait.h>	// Used for such things as waitpid
#include <unistd.h>	// Provides access to the POSIX API
#include <sys/socket.h>	// Makes available for the use of sockets
#include <netinet/in.h>	// Makes available access to network addresses
#include <netdb.h>	// Defines the hostent structure
#include <arpa/inet.h>	// Makes available ports

/* Function: validateChars
 * Parameters: size of file, int of open file
 * Overview: Goes through a file searching for bad characters.  A valid
 * 	character is one that is a space or A-Z
 * Pre: A file exists
 * Post: Doesn't return anything, but if invalid character is found it will exit
 */
void validateChars(int size, int file)
{
	// Set variables
	char *text_string;	// memory for string
	int read_result;	// Result of reading file
	int i;			// For the looping
	
	// Set the file to the beginning
	lseek(file, 0, SEEK_SET);

	// Allocate memory for the text string using the size
	text_string = malloc(size + 1);
	// Read from file into text string
	read_result = read(file, text_string, size);
	// Report an error if there is issue reading the string
	if (read_result == -1)
	{
		fprintf(stderr, "Error: reading file\n");
		exit(1);
	}

	// Null terminate the string
	text_string[size] = 0;

	// Go through each character in string and validate
	for (i = 0; i < size; i++)
	{
		// Check if the character in string is alpha or space
		if (isalpha(text_string[i]) || isspace(text_string[i]))
		{
			// Do nothing because it's one of these cases
			//printf("%c", text_string[i]);
		}
		// Otherwise it's an invalid character and error needs to be printed
		else
		{
			fprintf(stderr, "Error: File has invalid char\n");
			exit(1);	
		}
	}
	// Free up the text string
	free(text_string);

	// Set the file to the beginning
	lseek(file, 0, SEEK_SET);
}

/* Function: recvConf
 * Parameters: client socket, port number argument
 * Overview: Confirm with the server we can successfully communicate
 * Pre: client socket
 * Post: No return, but makes confirmation with server
 */
void recvConf(int socket_fd, char *port_num)
{
	// Set variables
	int msg_length = 512;
	char send_msg[msg_length];	// sent message to server
	int sent_size = 3;		// equal to size of "enc"
	char recv_string[2];		// Recieved string

	// Get a message ready to send
	strncpy(send_msg, "dec", msg_length);

	if (send(socket_fd, send_msg, sent_size, 0) < 0)
	{
		fprintf(stderr, "otp_dec Error: Failed to make initial confirmation with server\n");
	}	

	// Recieve confirmation from server
	recv(socket_fd, recv_string, 1, 0);

	// Check the response
	// If the response is an 'S', than we connected successfully, do nothing more
	if (strcmp(recv_string, "S") == 0)
	{
		// Nothing more to do here
		//printf("I got something here");
	}
	// If the response is a 'M', then server hit the max of processes. send message and exit 2
	else if (strcmp(recv_string, "M") == 0)
	{
		fprintf(stderr, "otp_dec Error: Server has max number of processes\n");
		exit(2);
	}
	// If the response is something else, then there is probably some unauthorization
	else
	{
		fprintf(stderr, "otp_dec Error: Client could not connect to otp_dec_d on port %s\n", port_num);
		exit(2);
	}	
}

/* Function: sendFile
 * Parameters: socket, file to be sent
 * Overview: Sends file to server
 * Pre: Established connection with server with client
 * Post: File sent to server
 */
void sendFile(int socket_fd, int send_file)
{
	// Set Variables
	int msg_length = 512;		// Length of packets sent
	char send_msg[msg_length];	// Message to be sent
	int size_sent;			// Size of the sent message

	// Loop to read file
	while ((size_sent = read(send_file, send_msg, sizeof(send_msg))) > 0)
	{
		// Send the message, but check to be sure it didn't fail to send, break
		if (send(socket_fd, send_msg, size_sent, 0) < 0)
		{
			fprintf(stderr, "otp_dec Error: Sent file failed\n");
			break;
		}
		// Put null characters into send message
		bzero(send_msg, msg_length);
	}
	if (size_sent < 0)
	{
		fprintf(stderr, "otp_dec ERROR: recv failed\n");
		exit(1);
	}
}

/* Function: recvFile
 * Parameters: socket
 * Overview: Recieves a file and prints to stdout
 * Pre: Both the encrypted and key files have been sent to server
 * Post: Sends decrypted file to stdout
 */
void recvFile(int socket_fd)
{
	// Set variables
	int msg_length = 512;		// Length of recieved msg
	char recv_msg[msg_length];	// received string piece
	int recv_size = 0;		// initialize to 0 of recieved msg

	// Loop to receive message
	while ((recv_size = recv(socket_fd, recv_msg, msg_length, 0)) > 0)
	{
		// print out decryption from server
		printf("%s", recv_msg);
		// Fill the recv_msg with null characters
		bzero(recv_msg, msg_length);
		// Break out of loop if at the end of the message or nothing more to recv
		if (recv_size != 512 || recv_size == 0)
			break;
	}
	// print newline to encryption file
	printf("\n");
}


/* Function: connToDaemon
 * Parameters: From the 3 char * arguments: plaintext; key; and port number.
 * 	Also, encrypted and key files
 * Overview: Setup connection to daemon, send encrypted file to daemon, and
 * 	recieve decrypted file from daemon.  Send decrypted file to stdout.
 * Pre: validated both files
 * Post: decrypted file is sent to stdout
 */
void connToDaemon(char *enc_name, char *key_name, char *port_name, int file_enc, int file_key)
{
	// Set variables
	int socket_fd;			// socket file descriptor
	struct sockaddr_in server_addr;	// Server's address structure
	//struct hostent *server_ip_addr;	// server's IP
	int port_num;			// Conversion of string from arg to int
	
	// Ensure a socket file descriptor can be setup
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "otp_dec Error: Failed to setup socket file descriptor\n");
		exit(2);
	}

	// Convert the port number from string to integer
	port_num = atoi(port_name);

	// Stuff server_addr struct with info to connect to it
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_num);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
	bzero(&(server_addr.sin_zero), 8);

	// Setup the Server IP address
	//server_ip_addr = gethostbyname("localhost");

	// Should the IP address not be resolved
	//if (server_ip_addr == NULL)
	//{
	//	fprintf(stderr, "Error: could not resolve server host name\n");
	//	exit(2);
	//}
	// Stuff server_addr struct with info to connect to it
	//server_addr.sin_family = AF_INET;
	//server_addr.sin_port = htons(port_num);
	//memcpy(&server_addr.sin_addr, server_ip_addr->h_addr, server_ip_addr->h_length);

	// Try to make a connection with the server port
	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "otp_dec Error: could not contact otp_dec_d on port %s\n", port_name);
		exit(2);
	}
	
	// Function to confirm there is a connection with the daemon
	recvConf(socket_fd, port_name);

	// Function to send both the plaintext and key file to the server
	// Send the encrypted file first
	sendFile(socket_fd, file_enc);
	// Send the key file
	sendFile(socket_fd, file_key);
	// Function to recieve the decrypted text
	recvFile(socket_fd);

	// Close the socket
	close(socket_fd);	
} 


/* Function: main
 * Parameters: number of arguments, the arguments
 * Overview: Handles agruments, management of functions and cipher to stdou
 */
int main(int argc, char ** argv)
{
	// Set variables
	int file_encrypt;	// encrypted file text
	int file_key;		// key file generated by keygen program
	int size_encrypt;	// size of the encrypted file
	int size_key;		// size of the key file

	// Check to be sure there are 4 arguments, otherwise print error of usage
	if (argc != 4)
	{
		fprintf(stderr, "otp_dec Usage: otp_dec <encrypted file> <key> <port>\n");
		exit(1);
	}	

	// -- Open both key and encrypted files and make some basic checks --
	// Try to see if encrypted file is available
	file_encrypt = open(argv[1], O_RDONLY);
	// If it doesn't exist ouput error and exit with 1
	if (file_encrypt == -1)
	{
		fprintf(stderr, "Error: encrypted file does not exist\n");
		exit(1);
	}

	// Try to see if key file is available
	file_key = open(argv[2], O_RDONLY);
	// If it doesn't exist output error and exit with 1
	if (file_key == -1)
	{
		fprintf(stderr, "Error: key file does not exist\n");
		exit(1);
	}

	// Check key file is greater than the encrypted file
	// Get size of encrypted file
	size_encrypt = lseek(file_encrypt, 0, SEEK_END);
	// Get size of key file
	size_key = lseek(file_key, 0, SEEK_END);
	// Verify the condition matches criteria
	if (size_key < size_encrypt)
	{
		// Send error the key used is to short
		fprintf(stderr, "Error: key file is too short\n");
		exit(1);
	}
	
	// Function to check for bad characters
	validateChars(size_encrypt, file_encrypt);
	validateChars(size_key, file_key);	
	
	// Function to connect to the daemon where it will send and recieve a file
	connToDaemon(argv[1], argv[2], argv[3], file_encrypt, file_key);

	// Close both files
	close(file_encrypt);
	close(file_key);

	// Exit the program
	return 0;
}
