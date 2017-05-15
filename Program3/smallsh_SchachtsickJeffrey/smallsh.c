/*
 * Author: Jeffrey Schachtsick
 * Course: CS344 - Operating Systems
 * Assignment: Program 3
 * Overview: A shell program to run command line instructions and return similar results to that of other shells
 * Details: This shell will allow redirection of standard input and standard output and it will support both the
 *    foreground and background processess.  There will be three built in commands: 'exit', 'cd', and 'status'.
 *    It will also support comments, which are the lines begginning with the '#' character.  
 *
 *    Last Updated: 05/14/2016
 *
 *    Among the many citations here, I should cite the following with helping me with the logic portion as I 
 *    was really stuck on how to handle this program.  Marta Wegner, smallsh.c program, 02/18/2016.  
 *    https://github.com/martamae/CS344/blob/master/Prog3/smallsh.c
 */

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>	// Header for setting signal handler
#include <sys/types.h>	// 3 headers for opening and closing files
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Description: Function for handling other commands using fork
 * Parameters: in and output file names, arguments, and background flag 
 * 	Signal and status of fork.
 * Pre: User input has been tokenized and found a command that doesn't fit
 *    the built in commands and arguments.
 * Post: Executes commands or throws commands in background depending on the
 *    user input.  Returns status of executed line.
 */
int otherCmd(char *in_file, char *out_file, char *args[513], int back_ground, struct sigaction signal, int f_fork) {
	// Set variables
	int f_pid;		// Process ID when fork() is run
	int file_in;		// Returned number from open() of file input
	int file_out;		// Returned number from open() of file output

	// Get process ID for fork()
	f_pid = fork();
	// If the pid from process is 0, handle as a child
	if (f_pid == 0) {
		// handle if the process is to not be in the background
		//    so that it can be interrupted by signal
		if (back_ground == 1) {
			// Set signal as default when executing
			signal.sa_handler = SIG_DFL;
			signal.sa_flags = 0;
			sigaction(SIGINT, &signal, NULL);			
		}		
		// Check if input file is something besides NULL
		if (in_file != NULL) {
			// There is an input file, make it read only in file stream
			file_in = open(in_file, O_RDONLY);
			// Print if the file doesn't exist and exit
			if (file_in == -1) {
				fprintf(stderr, "cannot open %s for input\n", in_file);
				fflush(stdout);
				exit(1);
			}
			// Print error if there is an issue with redirction of input
			else if (dup2(file_in, 0) == -1) {
				fprintf(stderr, "Error in dup2\n");
				fflush(stdout);
				exit(2);
			} 
			// Close input file stream at exec - Lecture 12, slide 8
			fcntl(file_in, F_SETFD, 1);
		} 
		// Handle if the process is in the background
		else if (back_ground == 0) {
			// Redirect process to /dev/null
			// Source: stackoverflow.com/questions/19955260/what-is-dev-null-in-bash
			file_in = open("/dev/null", O_RDONLY);
			// Print if there was an error opening the location
			if (file_in == -1) {
				fprintf(stderr, "cannot open /dev/null for input\n");
				fflush(stdout);
				exit(1);
			}
			// Print error if there is an issue with redirection of input
			else if (dup2(file_in, 0) == -1) {
				fprintf(stderr, "Error in dup2\n");
				fflush(stdout);
				exit(2);
			}
			// Close the input file stream on exec
			//close(file_num);
			fcntl(file_in, F_SETFD, 1);
		}
		// Check to see if there is an output file
		if (out_file != NULL) {
			// Open output file for write, create and even if it already exists at chmod 644
			file_out = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			// Print error if there was an issue creating outfile
			if (file_out == -1) {
				fprintf(stderr, "cannot open %s for output\n", out_file);
				fflush(stdout);
				exit(1);
			}
			// Print if there is an error with redirection with the file
			if (dup2(file_out, 1) == -1) {
				fprintf(stderr, "Error in dup2\n");
				fflush(stdout);
				exit(2);
			}
			// Close the output file stream on exec
			//close(file_num);
			fcntl(file_out, F_SETFD, 1);
		}
		// Execute command from 1st token using exec
		// Source: Lecture 9 slide 25
		if (execvp(args[0], args)) {
			// Print error if command is not recognized
			fprintf(stderr, "%s: no such file or directory\n", args[0]);
			fflush(stdout);
			exit(1);
		}
	}
	// Print an error if there was an issue with the fork
	else if (f_pid < 0) {
		fprintf(stderr, "Error: in fork");
		f_fork = 1;
		exit(1);
	}
	// Otherwise this is a parent process
	else {
		// Check to see if this is to be a background process
		//   if not, wait for the current foreground process to complete
		if (back_ground == 1) {
			do {
				waitpid(f_pid, &f_fork, 0);
			} while (!WIFEXITED(f_fork) && !WIFSIGNALED(f_fork));
		
			// In case the parent process gets Ctrl-C, print out a statement
			if (!(WIFEXITED(f_fork))) {
				printf("terminated by signal %d\n", f_fork);
				fflush(stdout);
			}
		}
		// Else it is in background, then print process ID of background job
		else {
			printf("background pid is %d\n", f_pid);
			fflush(stdout);
		}
	}
	// Return f_fork
	return f_fork;
}


/*
 * Description: Function for determining user entered command
 * Parameters: string of user line
 * Pre: User entered a line
 * Post: Returns the exit variable.  exit_var = 1 will exit the program 
 * 	Retunrs the status value to keep track when doing next command iteration
 */
int gatherCmnd(char *user_input, struct sigaction signal, int status)
{
	// Set variables
	int exit_var = 0;	// Remains at 0, unless user has 'exit'
	char *args[513];	// Array of arguments, Specs say max of 512 arguments
	char* in_file = NULL;	// Input file
	char* out_file = NULL;	// Output file
	char* tok;		// Handles individual tokens
	int back_ground = 1;	// Set to false, but when true, it will put the command in background
	int f_pid;		// holds the pid of a process
	int n;			// Counter for getting rid of stuff in array

	// Split up the user_input into tokens
	int num_args = 0;		// Counts number of args and resets the count	
	// Start with initial token from user_input
	tok = strtok(user_input, " \n");
	// Loop through the user_input gathering individual arguments
	while (tok != NULL) {
		// Check to see if token is an input file
		if (strcmp(tok, "<") == 0) {
			// Pass this token with newline
			tok = strtok(NULL, " \n");
			// Get the input file name from next token
			in_file = strdup(tok);
			//printf("Input file is %s\n", in_file);
			//fflush(stdout);
			// Move onto next argument
			tok = strtok(NULL, " \n");
		}	
		// Check to see if token is for output file
		else if (strcmp(tok, ">") == 0) {
			// Pass this token with newline
			tok = strtok(NULL, " \n");
			// Get the out file name from the next token
			out_file = strdup(tok);
			//printf("Output file is %s\n", out_file);
			//fflush(stdout);
			// Move onto next argument
			tok = strtok(NULL, " \n");
		}
		// Check to see if token is for background process
		else if (strcmp(tok, "&") == 0) {
			// Set the flag for background process
			back_ground = 0;
			break;
		}
		// Otherwise, this is an argument or some command
		// Sore this in an array
		else {
			args[num_args] = strdup(tok);
			// Increment number of arguments
			num_args++;
			// Move onto next token
			tok = strtok(NULL, " \n");
		}
	}
	
	// Make the last argument NULL for 
	args[num_args] = NULL;

	// Conditionals for determining command for args[0]
	// If begins with '#' or is nothing, do nothing
	//if (!(strncmp(args[0], "#", 1)) || args[0] == NULL) {
	if (args[0] == NULL || !(strncmp(args[0], "#", 1))) {
		//printf("I'm a comment or an empty line\n");
		//fflush(stdout);
		// Do nothing here
	}
	// If I'm an exit, set flag to exit the program
	else if (strcmp(args[0], "exit") == 0) {
		// Set the exit flag
		exit_var = 1;
		exit(0);
		//printf("Flag set to leave the prgram\n");
		//fflush(stdout);
	}
	// If I'm a 'cd', chage directory
	else if (strcmp(args[0], "cd") == 0) {
		// Change directory
		//printf("Ready to change directory\n");
		//fflush(stdout);
		// Check the second argument if it's Null, if so go HOME in env
		if (args[1] == NULL) {
			// Source: stackoverflow.com/questions/9493234/chdir-to-home-directory
			chdir(getenv("HOME"));
		}
		// Otherwise change directory to the second argument
		else {
			chdir(args[1]);
		}
	}
	// If input is 'status', handle status command
	else if (strcmp(args[0], "status") == 0) {
		// Get status of last execution
		//printf("Get status of last execution\n");
		//fflush(stdout);
		// Check and print exit status.
		// Lecture9-Processes, slide 20
		if (WIFEXITED(status)) {
			//printf("The process exited normally\n");
			int exit_status = WEXITSTATUS(status);
			printf("exit value %d\n", exit_status);
			fflush(stdout);
		}
		// Otherwise show which signal terminated
		else {
			printf("terminated by signal %d\n", status);
			fflush(stdout);
		}
	}
	// Else, this could be some other command
	else {
		//printf("Some other command???\n");
		//fflush(stdout);
		// Use function to handle the other commands
		// Returns status value from executing command
		status = otherCmd(in_file, out_file, args, back_ground, signal, status);
	}
	// Check to see if any processes have finished
	f_pid = waitpid(-1, &status, WNOHANG);	
	while (f_pid > 0) {
		// Print any background processes that complete
		printf("background pid %d is done: ", f_pid);
		// Dicide if it terminated successfully or by a signal
		// Lecture 9 - Processes slide 20, Checking the exit status
		if (WIFEXITED(status)) {
			printf("exit value %d\n", WEXITSTATUS(status));
			fflush(stdout);
		}
		else {
			printf("terminated by signal %d\n", status);
		}
		// Decrement to the next process id
		f_pid = waitpid(-1, &status, WNOHANG);
	}
	// Clean up for next iteration of commands
	// Set input and output files to NULL
	in_file = NULL;
	out_file = NULL;

	// Clear out the arguments array
	for (n = 0; n < num_args; n++) {
		args[n] = NULL;
	}

	// Return exit_var and status
	return exit_var, status;
}


/*
 * Main function for managing entry and exit of the shell.
 */
int main(void)
{
	// Set any variables
	int exit = 0;		// For main loop, exits when set to '1'
	char *user_input = NULL;	// User input, max length 2048 characters per Specs
	//char buff_input[2048];	// Initail user input then copied to user_input
	char *pos;		// removing the newline character in user entry
	int status = 0;		// Holds status value in WIFEXITED

	// Set the signal handler to ignore interrupts
	// Source: Lecture 13-Signals
	struct sigaction signal;
	signal.sa_handler = SIG_IGN;
	sigaction(SIGINT, &signal, NULL);

	// Loop until user selects 'exit'
	while (!exit) {
		

		// Specs: prompt for each command line
		printf(": ");
		// Flush for each output
		fflush(stdout);

		// Get the line from the user
		// Source: stackoverflow.com/questions/12252103/how-to-read-a-line-from-stdin-blocking-until-the-newline-is-found
		size_t size = 0;
		if (!(getline(&user_input, &size, stdin))) {
			return 0;
		}

		// Find if command is comment, nothing, a supported command or something else
		// Return exit value and keep track of the status
		exit, status = gatherCmnd(user_input, signal, status);
	}

	// Exit the shell program
	return 0;
}
