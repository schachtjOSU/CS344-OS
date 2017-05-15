/**********************************************************
* Name: Marta Wegner
* Assignment #3
* CS 344
* Filename: smallsh.c
* Date: 2/8/16
***********************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<sys/stat.h>

int main() {
     char* line = NULL;
     char* args[513];
     int numArgs;
     char* token;
     char* iFile = NULL;
     char* oFile = NULL;
     int fd;
     int bg;
     int status = 0;
     int  pid;
     int exited = 0;
     int i;

     //signal handler to ignore SIGINT
     struct sigaction act;
     act.sa_handler = SIG_IGN; //set to ignore
     sigaction(SIGINT, &act, NULL);

     while (!exited) {
          bg = 0;//background or not?

          //Print prompt
          printf(": ");
          fflush(stdout);

          //Read in a line 
          ssize_t size = 0;
          if (!(getline(&line, &size, stdin))) {
               return 0; //end if this is the end of the file
          }

          //split line into tokens
          numArgs = 0;
          token = strtok(line, "  \n"); //get initial token

          while (token != NULL) {
               if (strcmp(token, "<") == 0) {
                    //input file
                    //Get file name
                    token = strtok(NULL, " \n");
                    iFile = strdup(token);//copy token(file name) 
                    //to iFile variable

                    //Get next arg
                    token = strtok(NULL, " \n");
               }
               else if (strcmp(token, ">") == 0) {
                    //output file
                    //Get the file name
                    token = strtok(NULL, " \n");
                    oFile = strdup(token); //copy token(file name)
                    //to oFile variable

                    //Get next arg
                    token = strtok(NULL, " \n");
               }
               else if (strcmp(token, "&") == 0) {
                    //command in background
                    bg = 1; //set bg variable to indicate 
                    //it is in background
                    break;
               }
               else {
                    //this is a command or arg - store in array
                    args[numArgs] = strdup(token);

                    //get next token
                    token = strtok(NULL, " \n");
                    numArgs++; //increment # args
               }
          }

          //End array of args with NULL
          args[numArgs] = NULL;

          //Determine command
          //If it is comment or NULL
          if (args[0] == NULL || !(strncmp(args[0], "#", 1))) {
               //if comment or null - do nothing
          }
          else if (strcmp(args[0], "exit") == 0) { //exit command
               exit(0);
               exited = 1;
          }
          else if (strcmp(args[0], "status") == 0) { //status command
               //Print status
               if (WIFEXITED(status)) {//print exit status
                    printf("Exit status: %d\n", WEXITSTATUS(status));
               }
               else { //else print terminating signal
                    printf("Terminating signal %d\n", status);
               }
          }
          else if (strcmp(args[0], "cd") == 0) { //cd command
               //If there is not arg for cd command
               if (args[1] == NULL) {
                    //Go to HOME directory
                    chdir(getenv("HOME"));
               }
               else {//Else change to specified dir
                    chdir(args[1]);
               }
          }
          else { //other commands
               //fork command
               pid = fork();

               if (pid == 0) {  //Child 
                    if (!bg) { //If the process is not background
                         //command can be interrupted
                         act.sa_handler = SIG_DFL; //set to default
                         act.sa_flags = 0;
                         sigaction(SIGINT, &act, NULL);
                    }

                    if (iFile != NULL) { //If file to input is given
                         //open specified file read only
                         fd = open(iFile, O_RDONLY);

                         if (fd == -1) {
                              //Invalid file, exit
                              fprintf(stderr, "Invalid file: %s\n", iFile);
                              fflush(stdout);
                              exit(1);
                         }
                         else if (dup2(fd, 0) == -1) { //redirect input 
                              //Error redirecting input
                              fprintf(stderr, "dup2 error");
                              exit(1);
                         }

                         //close the file stream
                         close(fd);
                    }
                    else if (bg) {
                         //Else the process is in the background
                         //redirect input to /dev/null 
                         //if input file not specified
                         fd = open("/dev/null", O_RDONLY);

                         if (fd == -1) {
                              //error opening
                              fprintf(stderr, "open error");
                              exit(1);
                         }
                         else if (dup2(fd, 0) == -1) { //redirect input
                              //Error redirecting
                              fprintf(stderr, "dup2 error");
                              exit(1);
                         }

                         //Close file stream		
                         close(fd);
                    }
                    else  if (oFile != NULL) { //If file to output to given
                         //open specified file
                         fd = open(oFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                         if (fd == -1) {
                              //Error opening file, exit
                              fprintf(stderr, "Invalid file: %s\n", oFile);
                              fflush(stdout);
                              exit(1);
                         }

                         if (dup2(fd, 1) == -1) { //Redirect output
                              //Error redirecting
                              fprintf(stderr, "dup2 error");
                              exit(1);
                         }

                         //close file stream
                         close(fd);
                    }

                    //execute command stored in arg[0]
                    if (execvp(args[0], args)) { //execute
                         //Command not recognized error, exit
                         fprintf(stderr, "Command not recognized: %s\n", args[0]);
                         fflush(stdout);
                         exit(1);
                    }
               }
               else if (pid < 0) { //fork() error if pid < 0
                    //print error msg
                    fprintf(stderr, "fork error");
                    status = 1;
                    break;
               }
               else { //Parent
                    if (!bg) { //if not in background
                         //wait for the foreground process to complete
                         do {
                              waitpid(pid, &status, 0);
                         } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                    }
                    else {
                         //print pid if process is in bg
                         printf("background pid: %d\n", pid);
                    }
               }
          }

          //empty arg array to reuse for next line
          for (i = 0; i < numArgs; i++) {
               args[i] = NULL;
          }

          //Set files to null to be reused for future commands
          iFile = NULL;
          oFile = NULL;

          //bg processes finished?
          //Check to see if anything has died
          pid = waitpid(-1, &status, WNOHANG);
          while (pid > 0) {
               //Print that process is complete
               printf("background pid complete: %d\n", pid);

               if (WIFEXITED(status)) { //If the process ended successfully
                    printf("Exit status: %d\n", WEXITSTATUS(status));
               }
               else { //If the process was terminated by a signal
                    printf("Terminating signal: %d\n", status);
               }

               pid = waitpid(-1, &status, WNOHANG);
          }
     }

     return 0;
}