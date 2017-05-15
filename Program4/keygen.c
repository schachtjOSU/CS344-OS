/*
 * File: keygen.c
 * Author: Jeffrey Schachtsick
 * Course: CS344 - Operating Systems 1
 * Assignment: Program 4
 * Overview: Create a key file from a specified length.  Characters will be
 * 	randomly generated from 27 allowed characters, A-Z and the space
 * 	character.  
 * Last Update: 05/31/2016
 * Sources: Random number generator - www.cplusplus.com/reference/cstdlib/srand/
 * 	Allocate block for string - www.cplusplus.com/reference/cstlib/malloc/
 */

// Include Libraries
#include <stdio.h>		// General IO, including printf to redirect file
#include <stdlib.h>		// Randomization and exit
#include <time.h>		// Seeding the time

/*
 * Function: getKeyString
 * Parameters: int of string length, key string of a block of memory
 * Overview: Puts random characters into the key string
 * Pre: defined the size of the string and allocated a block of memory for the string
 * Post: Returns nothing, but fills block of memory with random characters
 */
void getKeyString(int str_length, char *key_string)
{
	// Set variables
	int i;			// Helps with loop
	static const char poss_chars[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
				// Per specs, possible chars available, Space and A-Z
	
	// Loop to fill key string with random characters from array of possible chars
	for (i = 0; i < str_length; i++)
	{
		key_string[i] = poss_chars[rand() % (sizeof(poss_chars) - 1)];
	}

	// Fill the last element with null terminator
	key_string[str_length] = 0; 
}

/*
 * Function: main
 * Parameters: number of arguments, arguments
 * Overview: handles arguments, prints characters, no return
 */
int main(int argc, char** argv)
{
	// Set variables
	char *key_string;	// Randomly generated string from length
	int str_length;		// Length of the string for the key
	
	// Initialize random number generator
	srand(time(NULL));

	// Check to be sure there are only 2 arguments, otherwise print error and exit
	if (argc !=2)
	{
		fprintf(stderr, "keygen usage: keygen <number_of_characters>\n");
		exit(1);
	}	
	else
	{
		// Get the length of the string	
		str_length = atoi(argv[1]);
		// Allocate a block of size bytes of memeory for the key string
		key_string = (char*)malloc(sizeof(char)*(str_length + 1));
		// Function to get teh key string
		getKeyString(str_length, key_string);
		// Print the generated key and with a newline char
		printf("%s\n", key_string);
	}

	// Unallocate (free) the block of memory for the key string
	free(key_string);

	// Exit the program
	return 0;
}
