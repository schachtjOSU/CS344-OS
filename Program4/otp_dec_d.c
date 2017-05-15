/*
 * File otp_dec_d.c
 * Author: Jeffrey Schachtsick
 * Course: CS344 - Operating Systems 1
 * Assignment: Program 4
 * Overview: This program will act as the daemon and be connected to by clients
 * 	running the otp_dec program and this program will run in the background
 * 	for each instance of itself.  The primary purpose is to recieve the 
 * 	encrypted and key files from the otp_dec program and decrypt using Open
 * 	Pad encryption.  It will then pass back to the client the decrypted file.
 * Last Update: 06/03/2016
 * Sources: Operating Systems Lectures 15, 16, and 17.
 *   Beej's Guide to Network Programming - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 *   Some logic based on the program used here from https://github.com/kevinto/cs344-prog4
 *   Sockets Tutorial by Rober Ingalls - http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 */

// Include Libraries
#include <stdio.h> 	// General IO, including printf to redirect files
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
#include <netdb.h>	// Defines the hostnet structure
#include <arpa/inet.h>	// Makes available ports

/* Function: sendMsg
 * Parameter: decrypted message and the client socket
 * Overview: Sends the decrypted message over to the client
 * Pre: Established the decrypted message
 * Post: Sends decrypted message out to client
 */
void sendMsg(char *decrypt_msg, int c_socket)
{
	// Set variables
	int msg_length = 1024;		// Length of each message
	char send_msg[msg_length];	// Message to be sent
	int size_sent;			// Size of the sent message
	bzero(send_msg, msg_length);
	//printf("my message is %s\n", encrypt_msg);
	// Loop to read string
	//while ((size_sent = read(enc_fd, send_msg, sizeof(send_msg))) > 0)
	//{
		//printf("Am I sending anything??????\n");
		size_sent = send(c_socket, decrypt_msg, msg_length, 0);
		// Send error if -1
		if (size_sent < msg_length)
		{
			fprintf(stderr, "otp_dec_d ERROR: Issue with sending data to client");
			exit(1);
		}
		
		// Send the message, but check to be sure it diden't fail to send, break
		/*if (send(c_socket, send_msg, size_sent, 0) < 0)
		{
			fprintf(stderr, "otp_enc_d ERROR: Sent file failed\n");
			//break;
		}
		// Put null characters into send message
		bzero(send_msg, msg_length);
		*/
	//}
}

/* Function: findLetter
 * Parameter: some value
 * Overview: Returns the letter in the possible chars
 * Pre: Established a value to interprut
 * Post: Returns the letter in the possible chars
 */
char findLetter(int some_value)
{
	// Set Variables
	static const char poss_chars[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	// Could the value be less than 0
	if (some_value < 0)
	{
		// Add the value from 27, so it wraps around to begninning of array
		some_value += 27;
	}
	return poss_chars[some_value];
}

/* Function: cypherLet
 * Parameter: some char
 * Overview: Converts a char to a value
 * Pre: Established a character
 * Post: Returns a value
 */
int findValue(char some_letter)
{
	// Set variables
	static const char poss_chars[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int j;			// For the loop

	// Loop to find match with letter
	for (j = 0; j < 27; j++)
	{
		// Does the array character match the letter
		if (poss_chars[j] == some_letter)
			return j;
	}
}  


/* Function: cypherLet
 * Parameters: encrypted, key, and decrypted character
 * Overview: Manages calculation for the decrypted letter
 * Pre: Established encrypted and key characters
 * Post: Returns the appropriate decrypted letter
 */
char cypherLet(char enc_char, char key_char, char dec_char)
{
	// Set variables
	int enc_num;		// encrypted value
	int key_num;		// key value
	int total_num;		// sum of plain and key

	// Find the values for both the encrypted and key numbers
	enc_num = findValue(enc_char);
	key_num = findValue(key_char);

	// Add both numbers to get total
	total_num = enc_num - key_num;	

	// Get the char associated with the total
	dec_char = findLetter(total_num);

	// Return the character
	return dec_char;
}


/* Function: encryptFile
 * Parameters: encrypted and key file names and size of encrypted, and client socket
 * Overview: Management of decryption
 * Pre: We have encrypted and key temp files available
 * Post: sends the string of encryption
 */
void encryptFile(char *enc_name, char *key_name, int enc_length, int c_socket, int dec_fd, char *dec_name)
{
	// Set variables
	char *decrypt_msg = malloc(enc_length * sizeof(char));	// Message to be sent back to client
	FILE *enc_file;			// Temp encrypted file
	FILE *key_file;			// Temp key file
	int i;				// Counter for loop
	char enc_char;			// encrypted char
	char key_char;			// key char
	char dec_char;			// decrypted char
	FILE *dec_file;			// decrypted file
	char dec_str[1];		// Character string

	// Open both files
	enc_file = fopen(enc_name, "r");
	key_file = fopen(key_name, "r");

	// Open the decryption file to append
	dec_file = fopen(dec_name, "w+");

	// Loop to gather each character
	for (i = 0; i < enc_length - 1; i++)
	{
		// Get both the encrypted and key char
		enc_char = (char)fgetc(enc_file);
		key_char = (char)fgetc(key_file);
		//enc_char = encrypt_msg[i];
		// Cypher the letter
		//enc_char = cypherLet(plain_char, key_char, enc_char);
		decrypt_msg[i] = cypherLet(enc_char, key_char, dec_char);
		
		//sprintf(enc_str, "%c", enc_char);

		//printf("enc_str is %s\n", enc_str);
		// Add char to the encrypted message
		//*encrypt_msg++ = enc_char;
		// Append char to enc_file
		//fprintf(enc_file, enc_str);
	}		
	fprintf(dec_file, "%s\n", decrypt_msg);
	//printf("My encrypted string %s\n", encrypt_msg);
	// Free up the encrypt_msg
	//free(encrypt_msg);
	
	// Close both files
	fclose(enc_file);
	fclose(key_file);
	fclose(dec_file);
	// Send encrypted file
	//sendMsg(enc_fd, c_socket);
	sendMsg(decrypt_msg, c_socket);
	
	free(decrypt_msg);
}


/* Function: recvFile
 * Parameters: string of the file name, client socket
 * Overview: Writes content sent by client into temp file
 * Pre: Created a temp file and client has established handshake
 * Post: Temp file is written, total length of plain text is accounted for
 */
int recvFile(char *file_name, int c_socket)
{
	// Set variables
	int msg_length = 512;		// Length of recieved msg
	char recv_msg[msg_length];	// received string piece
	int recv_size = 0;		// initialize to 0 of recieved msg
	FILE *the_file;			// File for writing stuff to
	int total_length = 0;		// Retains the total length of the file

	// Open the file to write stuff to
	the_file = fopen(file_name, "a");
	// In case the file doesn't open
	if (the_file == NULL)
	{
		fprintf(stderr, "otp_dec_d ERROR: %s could not be opened\n", file_name);
		exit(1);
	}

	// Loop to recieve the message
	while ((recv_size = recv(c_socket, recv_msg, msg_length, 0)) > 0)
	{
		//write recieved to file
		fprintf(the_file, recv_msg);
		// Add to the total length
		total_length += recv_size;
		// Fill the recv_msg with null characters
		bzero(recv_msg, msg_length);
		// Break out of loop if at the end of the message or nothing more to recv
		if (recv_size != 512 || recv_size == 0)
			break;
	}

	// Close the file
	fclose(the_file);

	return total_length;
}



/* Function: sendConf
 * Parameters: response string, client socket
 * Overview: Send confirmation message to client
 * Pre: Recieved message from client to confirm
 * Post: Client chooses whether to or not send files over
 */
void sendConf(char *reply, int sock)
{
	// Set variables

	if (send(sock, reply, 1, 0) < 0)
	{
		fprintf(stderr, "otp_dec_d ERROR: Failed to send confirmation");
		exit(1);
	}
}


/* Function: childProc
 * Parameters: child process socket
 * Overview: Handle client-server interaction with the child process
 * Pre: Already created child process
 * Post: Transmit decrypted file back to the client
 */
void childProc(int client_sock, int num_children)
{
	// Set variables
	char serv_reply[2];		// A reply back to the client on status
	char client_sent[4] = {0};	// Client's initial confirmation
	int msg_length = 512;		// Message length
	int proc_ID;			// Process ID of the child
	char enc_file[32];		// String for file name of encrypted file
	char key_file[32];		// String for file name of key
	struct stat st = {0};		// For checking existence of a file
	int enc_fd;			// encrypted filedescriptor
	int key_fd;			// key filedescriptor
	int enc_length;			// Total length of encrypted file
	int key_length;			// Total length of key file
	char dec_file[32];		// The decrypted file
	int dec_fd;			// Decryption file descriptor

	// Recieve initial confirmation from client
	recv(client_sock, client_sent, msg_length, 0);

	// Check that the message is dec
	if (strcmp(client_sent, "dec") == 0)
	{
		// Make sure we only have 5 or less current children going on
		if (num_children > 5)
		{
			strncpy(serv_reply, "M", 1);
			sendConf(serv_reply, client_sock);
			exit(0);  // Exit child
		}
		// This is a success and should send the conf to send over files	
		else
		{
			strncpy(serv_reply, "S", 1);
			sendConf(serv_reply, client_sock);
		}
	}
	// Otherwise recieved from some different client
	else
	{
		// Send a rejection response of "U"
		strncpy(serv_reply, "U", 1);
		sendConf(serv_reply, client_sock);
	}
	
	// Start to generate the files for encrypted and key
	
	// Get the process id of the child
	proc_ID = getpid();
	// Append the process ID with the encrypted and key files
	sprintf(enc_file, "encr_%d", proc_ID);
	sprintf(key_file, "keyr_%d", proc_ID);
	sprintf(dec_file, "dec_%d", proc_ID);

	// Open each of the files
	enc_fd = open(enc_file, O_WRONLY | O_CREAT, 0644);
	if (enc_fd == -1)
	{
		fprintf(stderr, "otp_dec_d ERROR: could not create encrypted temp\n");
		exit(1);
	}
	key_fd = open(key_file, O_WRONLY | O_CREAT, 0644);
	if (key_fd == -1)
	{
		fprintf(stderr, "otp_dec_d ERROR: could not create key temp\n");
		exit(1);
	}
	dec_fd = open(dec_file, O_WRONLY | O_CREAT, 0644);
	if (dec_fd == -1)
	{
		fprintf(stderr, "otp_dec_d ERROR: could not create dec temp\n");
		exit(1);
	}
	// Cose the files for now
	//close(plain_fd);
	//close(key_fd);
	//close(enc_fd);
	// Function to write recv stuff into the pliantext file
	// Also get the length of the plaintext to be used for encryption string later
	enc_length = recvFile(enc_file, client_sock);
	
	// Function to write recv stuff into the key file,
	// not really going to do anything with length
	key_length = recvFile(key_file, client_sock);

	// Function to encrypt using both plaintext and key files and send it
	encryptFile(enc_file, key_file, enc_length, client_sock, dec_fd, dec_file);
	
	// Close the encryption file_descriptor
	close(enc_fd);
	close(key_fd);
	close(dec_fd);
	//close(enc_fd);
	// Remove the temp files as we are done with them
	remove(enc_file);
	remove(key_file);
	remove(dec_file);	

}


/* Function: main
 * Parameters: number of arguments, the arguments
 * Overview: Handles arguments, management of functions for decryption
 */
int main(int argc, char *argv[])
{
	// Set variables
	int socket_serv_fd;		// server socket file descriptor
	struct sockaddr_in server_addr;	// Server's address structure
	int server_port;		// Server's port number
	int socket_client_fd;		// client socket file descriptor
	int sock_size;			// Size of sockaddr_in
	int pid;			// process IDs
	struct sockaddr_in client_addr;	// Client's address structure
	int sock_opt = 1;		// Sets the option in socket for reuse
	struct sigaction signal;	// Signal structure
	int num_children = 0;		// Count for number of children
	int f_pid;			// Process ID when fork() is run
	int status = 0;			// Status of fork

	// Check to make sure there is one argument of the port number
	if (argc < 2)	
	{
		fprintf(stderr, "otp_dec_d Usage: otp_dec_d <port_number>\n");
		exit(1);
	}

	// Set up the server socket
	if ((socket_serv_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "otp_dec_d ERROR: Failed to setup socket file descriptor\n");
		exit(1);
	}

	// Get the port number as an integer from the argument
	server_port = atoi(argv[1]);

	// Stuff the server socket with address information
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind the server address to the socket using conditional
	if (bind(socket_serv_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "otp_dec_d ERROR: Failed to bind address to socket\n");
		exit(1);
	}

	// Create a socket option where we can reuse the address on the server address
	setsockopt(socket_serv_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));

	// Start to listen on the port
	// 5 is the number availble spots for clients to contact
	if (listen(socket_serv_fd, 5) == -1)
	{
		fprintf(stderr, "otp_dec_d ERROR: Failed at listening on port");
		exit(1);
	}

	// Set the signal handler to ignore interrupts
	signal.sa_handler = SIG_IGN;
	sigaction(SIGINT, &signal, NULL);

	// Loop to accept clients
	while (1)
	{
		// Get the size of the sockaddr_in
		sock_size = sizeof(struct sockaddr_in);

		// Accept client connections with conditional in case something happens
		if ((socket_client_fd = accept(socket_serv_fd, (struct sockaddr *)&client_addr, &sock_size)) == -1)
		{
			fprintf(stderr, "otp_dec_d ERROR: Failed to accept client socket\n");
			exit(1);
		}
		
		// Add one to number of children
		num_children++;

		// Get the process ID for the fork
		f_pid = fork();
		// If teh pid from the process is 0, handle as a child
		if (f_pid == 0)
		{
			// Set signal as default when executing
			signal.sa_handler = SIG_DFL;
			signal.sa_flags = 0;
			sigaction(SIGINT, &signal, NULL);
			//close the server socket
			close(socket_serv_fd);
			// Function to handle the child process
			childProc(socket_client_fd, num_children);
			exit(0);
		}
		// Print an error if there was an issue with the fork
		else if (f_pid < 0) 
		{
			fprintf(stderr, "otp_dec_d ERROR: Failed in fork\n");
			status = 1;
			exit(1);
		}
		// Otherwise this is a parent process
		else
		{
			// Wait for a process to complete
			do {
				waitpid(f_pid, &status, 0);
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));

			// In case the parent process gets Ctrl-C, print out a statement
			if (!(WIFEXITED(status))) 
			{
				printf("terminated by singal %d\n", status);
				fflush(stdout);
			}	
			// Close the client socket
			close(socket_client_fd);
			num_children--;
		}

	}

	// Exit the program
	return 0;
}
