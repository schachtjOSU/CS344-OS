/*
Author: Jeffrey Schachtsick
Course: CS344 - Operating Systems
Assignment: Program 2
Overview: A simple game to move from room to room utilizing reading and writing of files.
Details: This is a simple game that randomly creates 7 rooms from a total of already
   hardcoded 10 rooms.  The object is to have the user be able to move between the rooms
   from start to end.  Connections between for each room will be randomly selected, but each
   room will have between 3 and 7 connections.  When the user reaches the end room, a 
   congratulatory message will appear along with a listing of all the rooms entered.

Last Update: 04/20/2016
*/

// Includes
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/* Description: Return the string of the room name
 * Parameters: integer that points to the room name
 * Pre: None
 * Post: Returns string of room name
 */
const char * getRoomName(int num) {
	// Set variables
	const char * room_name;
	
	// Switch and Case to reveal room name
	switch ( num ) {
	case 0:
		room_name = "Hallway";
		break;
	case 1:
		room_name = "Library";
		break;
	case 2:
		room_name = "The_Study";
		break;
	case 3:
		room_name = "Greenhouse";
		break;
	case 4:
		room_name = "Dinning_ Room";
		break;
	case 5:
		room_name = "Kitchen";
		break;
	case 6:
		room_name = "Wine_Cellar";
		break;
	case 7:
		room_name = "Ballroom";
		break;
	case 8:
		room_name = "Billiard_Room";
		break;
	default:
		room_name = "Lounge";
		break;
	}
	return room_name;
}

/* Desicription: Creates a random number within a certain range
 * Parameters: beginning range and ending range
 * Pre: None
 * Post: random number
*/ 
int randomizer( int beg_range, int end_range ) {
	// variable for random number
	int rand_num;

	// Use random function from stdlib header
	rand_num = rand() % end_range + beg_range;

	return rand_num;
}

/*
 * Description: Create 7 room files and place them in the created directory
 * Parameters: Name of the directory
 * Pre: Already created the directory
 * Post: 7 Room files with random names and connections
*/
void createRooms(char *dir_name) {
	// Set variables
	int all_array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; // Will be assigned to a room
	int all_size = 10;
	int orig_array[7];
	int copy_array[7];
	int n;		// counter for creating 7 files
	int orig_size = 0;  // Indicates the size of orig_array	
	char file_path[38];  // String for file name
	int file_descriptor;
	const char * the_room;  // string for room name
	char type_string[32];   // String to append to the file
	FILE *room_file;		// Room file to append string too
	int room_num;		// Figuratively points to room string
	int new_size;		// Size of poss_connect_1
	char room_type[32];	// String to assign room type
	int m;			// Counter for going through each connection
	int j;              // Counter for finding the original rooms
	char temp_name[16];	// string for temp file name
	FILE *temp_file;		// temp file object
	char connections_line[38];	// String for writing connections to room file
	const char * connect_room; 	// String for having the name of the connecting room.

        // Fill the original array for all possible rooms.
     	for (j = 0; j < 7; j = j + 1) {
          	// Get a random number from the randomizer
          	int random_num = randomizer(0, all_size);
          	orig_array[j] = all_array[random_num];
          	// Move the last number to the replaced number
          	if (random_num != all_array[all_size])
               		all_array[random_num] = all_array[all_size];
          	all_size = all_size - 1;
     	}

	// Get the size of the original array
	orig_size = sizeof(orig_array) / sizeof(int);

	// Loop until 7 room files are created
	for( n = 0; n < 7; n = n + 1) {
		for (j=0; j<7; j = j + 1){
			//printf("%d ", orig_array[j]);
			copy_array[j] = orig_array[j];
		}
		//printf("\n");
		// Get a random number from the array within range of 0th element to size of array
		int rand_orig = randomizer(0, orig_size);

		// Using current number, create the new file.
		sprintf(file_path, "%s/room%d", dir_name, n);
		file_descriptor = open(file_path, O_WRONLY | O_CREAT, 0644);
		// In case something out of the ordinary occurs
		if (file_descriptor == -1)
			exit (1);
		// Close the file for now
		close(file_descriptor);

		// Get the element from orig_array using the random number
		room_num = orig_array[rand_orig];
		// Assign a new size when we condense the original array
		int new_size = orig_size - 1;
		// Unless the last element was choosen, stuff the last element into the choosen element.
		if (room_num != orig_array[orig_size - 1]) {
			orig_array[rand_orig] = orig_array[orig_size - 1];
			copy_array[rand_orig] = copy_array[orig_size - 1];
		}
		// Assign new size to original size	
		orig_size = new_size;

		// Get the room name
		the_room = getRoomName(room_num);

		// Append the room name to the file
		sprintf(type_string, "ROOM NAME: %s\n", the_room);

		// Print to room file
		room_file = fopen(file_path, "a");
		// In case the file doesn't open correctly
		if (room_file == NULL )
			exit(1);

		// Print Room name
		fprintf(room_file, type_string);
		//printf("%s", type_string);
		// Make connections
		
		int beg_connect = 3;		// Beginning of range
		int end_connect = 4;		// End of range, Spec says between 3 and 6 connections
		int copy_size = orig_size;	// Keep the original size intact for later
		int number;			// Captures the int when reading the temp file
		char ch;			// Captures the space when reading the temp file
		int prev_number = -1;		// Captures the previously used number when reading temp files
		int total_connect = 0;		// total connections already made
		int k;				// Counter for going through the copy array and find the match number

		// Check if file exists
		sprintf(temp_name, "%s/temp%d", dir_name, room_num);
		temp_file = fopen(temp_name, "r");
		if (temp_file != NULL ) {
			// Read contents of file
			while (fscanf(temp_file, "%d%c", &number, &ch) != EOF) {
				// If the previous number is the same as number, skip it.
				if (prev_number != number) {
					prev_number = number;
					total_connect = total_connect + 1;
					// Get the string for the room name from the number
					connect_room = getRoomName(number);
					// Append to current file
					sprintf(connections_line, "CONNECTION %d: %s\n", total_connect, connect_room);
					fprintf(room_file, connections_line);
					//printf("%s", connections_line);
					// Go through each element in the copy array and find the one that matches number
					for ( k = 0; k < copy_size; k = k + 1) {
						if (number == copy_array[k]) {
							copy_array[k] = copy_array[copy_size - 1];
							copy_size = copy_size - 1;
						}
					}
				}
			}
			fclose(temp_file);
		}
		// Remove the temp file, as we are done with it.
		remove(temp_name);

		// Use randomizer to get a random number of connections
		int rand_connections = randomizer(beg_connect, end_connect);
		// Loop through each connection
		for ( m = total_connect; m < rand_connections; m = m + 1) {
			// Use the randomizer to pick out any new connections
			int connect = randomizer(0, copy_size);
			// Get the room number
			int connect_room_num = copy_array[connect];
			// Unless teh last element was choosen, stuff the last element into the choosen element
			if (connect_room_num != copy_array[copy_size - 1]) {
				copy_array[connect] = copy_array[copy_size - 1];
			}
			copy_size = copy_size - 1;
			// Append to temp file this current room number
			sprintf(temp_name, "%s/temp%d", dir_name, connect_room_num);
			temp_file = fopen(temp_name, "a");
			fprintf(temp_file, "%d ", room_num);
			fclose(temp_file);
			// Get the room name
			connect_room = getRoomName(connect_room_num);
			// Append to current room number file the connections
			sprintf(connections_line, "CONNECTION %d: %s\n", m + 1, connect_room);
			fprintf(room_file, connections_line);
			//printf("%s", connections_line);
		}

		// Append the room type to the file, based on conditionals below
		// If first picked, it's the start room.
		if( n == 0 )
			sprintf(room_type, "ROOM TYPE: START_ROOM\n");
		// If last picked, it's the end room.
		else if ( n == 6 )
			sprintf(room_type, "ROOM TYPE: END_ROOM\n");
		// Otherwise, it's a middle room.
		else
			sprintf(room_type, "ROOM TYPE: MID_ROOM\n");
		
		// Append Room type (i.e. End, Mid, Start)
		fprintf(room_file, room_type);
		//printf("%s", room_type);
		// Close the current file
		fclose(room_file);
	}
}

/*
Description: Create directory and manage room files
Parameters: None
Pre: None
Post: dirctory created along with room files.
*/
void createDirectory(char * dir_name) {
     // Set any variables
	struct stat st = {0};
	char temp_path[38];  // String for path of temp files

	// Check to make sure directory does not already exist
	// Source for below at: stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
	if (stat(dir_name, &st) == -1) {
		mkdir(dir_name, 0755);
	}
	// Start creating the room files
	createRooms(dir_name);
}

/*
 * Description: Starts the actual game
 * Parameters: Passing the directory path to get to room files
 * Pre: Should have already created room files and placed in a room directory
 * Post: Returns number of steps taken
 */
int runGame(char * dir_name) {
	// Set variables
	FILE * curr_file;	// Current file open
	FILE * vic_file;	// File to contain the path taken by the user on the way to victory.
	char vic_path[38];	// file path for the victory file
	char vic_line[12];	// Line to be entered into the victory file
	char file_path[38];	// file path for current room
	char name_line[38];	// Capture the room name line
	char room_connect[38];	// Acquiring the room of the connection
	int lines_count;	// A count for number of lines
	char ch;		// Capture the new line in counting lines
	int a;			// Counter for getting first file connections
	char type_line[12];	// Capture the room type
	char end_line[12];	// Compared with type_line to stop the main loop here and essentially ending game.
	char buff_line[100];	// Captures the stdin buffer
	char user_line[100];	// Copied from buffer and used for comparisons
	int b;			// Counter for comparing user string and connections
	char *pos;		// For removing the newline character in user entry
	int c;			// Counter for going through each file in the directory
	int num_steps = 0;	// Counter for number of steps taken

	// Quick Welcome message
	printf("\nWelcome to another Adventure Game\n");
	printf("How many steps will it take you to get to the end?\n\n");

	// Create the path for the victory file
	sprintf(vic_path, "%s/tempVic", dir_name);

	// Assign room0 as the first room and get it's data
	sprintf(file_path, "%s/room0", dir_name);

	// Start loop until user finds the END_ROOM
	strcpy (end_line, "END_ROOM");
	while (strcmp(type_line, end_line) != 0) {

		printf("\n");
		curr_file = fopen(file_path, "r");
		// Count the number of lines in the file
		// Source: stackoverflow.com/questions/12733105/c-function-that-counts-lines-in-file
		lines_count = 0;
		while(!feof(curr_file)) {
			ch = fgetc(curr_file);
			if(ch == '\n')
				lines_count++;
		}	

		//printf("Number of lines: %d\n", lines_count);

		// Get the room name, Print CURRENT LOCATION per Specs
		fseek(curr_file, 11, SEEK_SET);
		fscanf(curr_file, "%s", &name_line);
		printf("CURRENT LOCATION: %s\n", name_line);

		// Get the connections, Print POSSIBLE CONNECTIONS per Specs
		printf("POSSIBLE CONNECTIONS:");
		for ( a = 2; a < lines_count; a++) {
			fseek(curr_file, 14, SEEK_CUR);
			fscanf(curr_file, "%s", &room_connect);
			printf(" %s", room_connect);
			// Per specs, print comma after each connection.  Except for last one gets a period
			if ( a != lines_count -1)
				printf(",");
			else
				printf(".\n");
		}	
	 
		// Get the Room Type
		fseek(curr_file, 12, SEEK_CUR);
		fscanf(curr_file, "%s", &type_line);
		//printf("Room Type: %s\n", type_line);

		// Ask user where to, then look up if connection exists
		printf("WHERE TO?>");
		// Source: stackoverflow.com/questions/3919009/how-to-read-from-stdin-with-fgets
		while ( fgets (buff_line, 100, stdin) ) {
			strcpy( user_line, buff_line);
			break;
		}
		// Replace the last newline with a null terminating
		// Source: stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		if ((pos = strchr(user_line, '\n')) != NULL)
			*pos = '\0';
		//printf("User's line: %s", user_line);
		// Check connections to make sure user_line exists
		fseek(curr_file, 11, SEEK_SET);
		fscanf(curr_file, "%s", &name_line);
		//printf("CURRENT LOCATION: %s\n", name_line);
		int no_match = 1;
		b = 0;
		// Loop through connections, if match, break.  Otherwise, don't understand
		for ( b = 2; b < lines_count; b++) {
			fseek(curr_file, 14, SEEK_CUR);
			fscanf(curr_file, "%s", &room_connect);
			//printf(" %s\n", room_connect);
			if ( strcmp(room_connect, user_line) == 0) { 
				//printf("It's a match!!!");
				no_match = 0;
				break;
			}
		}
		if (no_match == 1) {
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n");
			fclose(curr_file);
			continue;
		}

		// Close the current file
		fclose(curr_file);

		// Go through each file, find the ROOM NAME that matches the user_line
		for ( c = 0; c < 7; c++ ) {
			// Create the path to room<c>
			sprintf(file_path, "%s/room%d", dir_name, c);
			// Open the file from path
			curr_file = fopen(file_path, "r");

			// Get th room name
			fseek(curr_file, 11, SEEK_SET);
			fscanf(curr_file, "%s", &name_line);
			//printf("Seeking ROOM: %s\n", name_line);
			if ( strcmp(name_line, user_line) == 0) {
				//printf("File Found!!!");
				break;
			}
			else
				fclose(curr_file);
		}

		// Add another step to victory
		num_steps++;

		// Add the name_line into the victory file
		sprintf(vic_line, "%s\n", name_line);
		// Print vic_line to vic_file
		vic_file = fopen(vic_path, "a");
		fprintf(vic_file, vic_line);
		fclose(vic_file);

		// Does this curr_file have a type_line of END_ROOM?
		// Get to end of file, capture the last string
		fseek(curr_file, -9, SEEK_END);
		fscanf(curr_file, "%s", &type_line);
		//printf("Captured Type: %s\n", type_line);

		// End the loop for testing
		//strcpy (type_line, "END_ROOM");
	}
	// Return number of steps taken
	return num_steps;
}

/*
Main function for managing other functions in the other parts of the program.
*/
int main(void)
{
	// Set any variables
	char dir_name[32];	// String for directory path
	int num_steps;		// Number of steps taken in the game to get to end
	FILE * vic_file;	// File containing the path to victory
	char vic_path[38];	// file path for the victory file
	char vic_line[12];	// Display the lines in victory file

	// Get the process ID
	int proc_ID = getpid();
	//printf("My proc ID is: %d\n", proc_ID);

	// Make the directory with processID, SPECS: <my_username>.rooms.<process id>
	// Combine the hardcoded dir name with the process ID
	sprintf(dir_name, "schachtj.rooms.%d", proc_ID);
	//printf("The Created Dirctory is: %s\n", dir_name);
     	// Create directory and room files
     	createDirectory(dir_name);

	// Start the game
	num_steps = runGame(dir_name);
	printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);

	// Go through the victory file, display each line
	sprintf(vic_path, "%s/tempVic", dir_name);
	vic_file = fopen(vic_path, "r");
	while (fscanf(vic_file, "%s", &vic_line) != EOF) {
		printf("%s\n", vic_line);
	}
	printf("%\n");
	fclose(vic_file);

	// Remove the tempEnd file
	remove(vic_path);

     	// Exit the program
     	exit(0);
}
