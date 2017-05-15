/*********************************
* Program Filename:  smallsh
* Author:  Brian Stamm
* Date:  8.3.15
* Description:  This is a simple, small shell that runs some basic commands.
*
* Notes:  No real notes.  Anything of importance is listed in the comments
* throughout the program.
*
* Citation:  www.stephen-brennan.com/2015/01/16/write-a-shell-in-c/; youtube - DrBFraser
* and his videos on fork() & exec().  I also read alot of man pages, read the text book
* and followed the posting in our class discussion (the one with Matthew Meyn was really
* helpful).  There were alot of stacksoverflow pages read to understand the process as well.
* *****************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#define DELIMITER " \t\n"	//Used in the parseFunction
int MAX_ARG = 512;		//Max amount of arguments allowed, per assignment
int MAX_LEN = 2048;		//Max length of characters allowed

/***************
* Function:  parseFunction
* Variables:  an array of char's, or the command line arguments
* Outcome:  It creates an array houing each of the argument passed in through
* the command line.  It returns the array.
* ***********/
char **parseFunction(char *line){
     int i = 0;
     char *word;
     char **parse = malloc(MAX_ARG * sizeof(char*));	//Creates the array to hold each word
     if (parse == NULL){
          perror("Unable to parse.\n");
          exit(1);
     }
     word = strtok(line, DELIMITER);			//Goes through each word, parsing each out
     while (word != NULL){					//using DELIMITER to tell when new words start
          parse[i] = word;
          i++;
          word = strtok(NULL, DELIMITER);
     }
     parse[i] = NULL;					//At last spot, insert NULL value, for exec later
     return parse;
}
/***************
* Function:  assessment
* Variables:  It takes the array of command line arguments.
* Outcome:  It goes through the array and searches for various characters.  If it contains
* those, it returns various number values, used in the if/else if statement in main.
* ************/
int assessment(char **array){
     int i = 0;
     if (array[0] == NULL){			//Nothing entered in command line.
          return 5;
     }
     else if (strcmp(array[0], "#") == 0){		//Comments
          return 5;
     }
     else if (strcmp(array[0], "exit") == 0){	//Exit command
          return 0;
     }
     else if (strcmp(array[0], "cd") == 0){	//Change directory command
          return 1;
     }
     else if (strcmp(array[0], "status") == 0){	//Status command
          return 2;
     }
     else{
          while (array[i] != NULL){   		//Anything else
               if (strcmp(array[i], "&") == 0){	//Like a background process
                    array[i] = NULL;
                    return 4;
               }
               i++;
          }
          return 3;
     }
}
/***************
* Function:  refit
* Variables:  takes the array of command line words, and an int, which is a position
* Outcome:  Starting at the position passed in, it shifts all the values down one
* value.  Used for both > and <.  Returns the new array.
* ************/
char** refit(char **array, int num){
     while (array[num] != NULL){
          array[num] = array[num + 1];
          num++;
     }
     return array;
}
/***************
* Function:
* Variables:
* Outcome:
* ************/
void signalCatcher(int signo){
     printf("Caught an interrupt:  %d\n", signo);
}

int main(int argc, char **argv){
     char cwd[1024];			//Works for getting the directory
     getcwd(cwd, sizeof(cwd));

     struct sigaction morse; 		//Sets up the initial values for sigaction
     morse.sa_handler = SIG_IGN;
     morse.sa_flags = 0;
     sigfillset(&(morse.sa_mask));
     sigaction(SIGINT, &morse, NULL);

     int truth = 42;			//If it's the meaning of life, why not truth too?   
     do {					//Start of do while, while truth

          int status, BGstatus;
          char *line;
          size_t lineSize = 50;
          size_t length;
          /**********
          * First block to catch signals.  Used to check if any background processes have returned,
          * and if so, informs user about PID value and status.
          * *********/
          pid_t bgCheck = waitpid(-1, &BGstatus, WNOHANG);
          if (bgCheck > 0){
               if (WIFEXITED(BGstatus)){
                    printf("Process %d exited normally.  Exit status: %d\n", (int)bgCheck, WEXITSTATUS(BGstatus));
               }
               else{
                    int extSig = WTERMSIG(BGstatus);
                    printf("Process %d exited abnoramlly.  Exit signal: %d\n", (int)bgCheck, extSig);
               }
          }

          line = (char*)(malloc)(lineSize * sizeof(char));	//This block gets the command line arguments
          if (line == NULL){
               perror("Unable to readline.\n");
               exit(1);
          }
          printf(": ");
          fflush(stdout);
          length = getline(&line, &lineSize, stdin);

          char **lineArray = parseFunction(line);		//Parse the line into an array
          int j = assessment(lineArray);			//Assesses the array values

          if (j == 0){					//Exit command, exits do while loop.
               truth = 0;
          }

          else if (j == 1){					//CD command
               //First, if no other arguments, moves to home directory
               if (lineArray[1] == NULL){
                    chdir(getenv("HOME"));
               }
               else{
                    //Else, gets present directory and combines with command line arg
                    char present[1024];
                    getcwd(present, sizeof(present));
                    char preDir[1024];
                    snprintf(preDir, 1024, "%s/%s", present, lineArray[1]);
                    if (chdir(preDir) == -1){
                         printf("Goo.  Directory error!\n");
                    }
               }
          }

          else if (j == 2){					//Status command
               printf("Status:  %d\n", WEXITSTATUS(status));
          }

          else if (j == 3){					//Foreground Processes
               int w = 0;
               //First goes through to see if there's either > or <
               while (lineArray[w] != NULL){
                    //If there is, does the fork
                    if (strcmp(lineArray[w], ">") == 0){
                         pid_t childPID = fork();
                         //Sets flags to be default
                         morse.sa_handler = SIG_DFL;
                         morse.sa_flags = 0;
                         sigaction(SIGINT, &morse, NULL);
                         if (childPID < 0){
                              perror("Fail.\n");
                              exit(1);
                         }
                         else if (childPID == 0){
                              //Redirects the file to the right stream
                              int fd = open(lineArray[w + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                              lineArray = refit(lineArray, w);
                              if (fd == -1){
                                   perror("open.\n");
                                   exit(1);
                              }
                              int fd2 = dup2(fd, 1);
                              if (fd2 == -1){
                                   perror("dup2.\n");
                                   exit(2);
                              }
                              execlp(lineArray[0], lineArray[1], NULL);
                         }
                         else{
                              pid_t endedPID = waitpid(childPID, &status, 0);	//Standard waitpid
                              if (endedPID == -1){
                                   perror("Air or.\n");
                                   exit(1);
                              }
                              j = 2;	//Changes j value for later, so don't run exec again.
                              break;
                         }
                    }
                    //Same set up, but stream's switched and slightly different open
                    else if (strcmp(lineArray[w], "<") == 0){
                         pid_t childPID = fork();
                         morse.sa_handler = SIG_DFL;
                         morse.sa_flags = 0;
                         sigaction(SIGINT, &morse, NULL);
                         if (childPID < 0){
                              perror("Fork fail.\n");
                              exit(1);
                         }
                         else if (childPID == 0){
                              int fd = open(lineArray[w + 1], O_RDONLY);	//Read only for the open
                              lineArray = refit(lineArray, w);
                              if (fd == -1){
                                   perror("open.\n");
                                   exit(1);
                              }
                              int fd2 = dup2(fd, 0);
                              if (fd2 == -1){
                                   perror("dup2.\n");
                                   exit(2);
                              }
                              execlp(lineArray[0], lineArray[1], NULL);
                         }
                         else{
                              pid_t endedPID = waitpid(childPID, &status, 0);
                              if (endedPID == -1){
                                   perror("Air or.\n");
                                   exit(1);
                              }
                              j = 2;	//Changed for later
                              break;
                         }
                    }
                    w++;
               }
               if (j != 2){	//Used to see if j value was changed, if it was, then had already done process
                    //Same set up as previous, but it doesn't do anything with files
                    pid_t childPID = fork();
                    morse.sa_handler = SIG_DFL;
                    morse.sa_flags = 0;
                    sigaction(SIGINT, &morse, NULL);
                    if (childPID < 0){
                         perror("Fork fail.\n");
                         exit(1);
                    }
                    else if (childPID == 0){
                         execvp(lineArray[0], lineArray);
                    }
                    else{
                         pid_t endedPID = waitpid(childPID, &status, 0);
                         if (endedPID == -1){
                              perror("Air or.\n");
                              exit(1);
                         }
                    }
               }
          }

          else if (j == 4){							//Background processes.
               pid_t childBG = fork();
               if (childBG < 0){
                    perror("Fail.\n");
                    exit(1);
               }
               else if (childBG == 0){
                    execvp(lineArray[0], lineArray);
               }
               else{
                    pid_t endBG = waitpid(childBG, &BGstatus, WNOHANG);		//Same, but has WNOHANG
                    if (endBG == -1){
                         perror("WaitPID errored.\n");
                         exit(1);
                    }
                    printf("Background process ID:  %d\n", (int)childBG);	//Prints PID for bg process
               }
          }

          else if (j == 5){							//All other cases, where nothing needs to happen
          }

          free(line);
          free(lineArray);

     } while (truth);							//And do it again...

     return 0;
}