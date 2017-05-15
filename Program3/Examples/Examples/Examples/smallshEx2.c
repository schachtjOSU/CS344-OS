#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

//prints the given numerical status along with a fuller explanation in English
void printStatus(int status);

int main() {
     char input[513];
     char* args[513];
     char* token;
     char* ifname = NULL;  //input file name
     char* ofname = NULL;  //output file name
     int fi = -1;          //input file descriptor
     int fo = -1;          //input file descriptor
     int isForeground;     //foreground/background boolean
     int status = 0;
     int cpid;
     struct sigaction act;

     //set up the signal handler to ignore SIGINT
     act.sa_handler = SIG_IGN;
     act.sa_flags = 0;
     sigfillset(&(act.sa_mask));
     sigaction(SIGINT, &act, NULL);

     //main command prompt loop
     while (1) {
          isForeground = 1;

          printf(": ");       //print prompt
          fflush(stdout);     //flush prompt to keep things tidy

          //get user input
          if (fgets(input, 513, stdin) == NULL) {
               //we've reached the end of the testfile, so end gracefully
               return 0;
          }

          //tokenize input and parse it
          int k = 0;
          token = strtok(input, " \n");
          while (token != NULL) {
               if (strcmp(token, "<") == 0) {
                    //parse the next token as an input file
                    token = strtok(NULL, " \n");
                    ifname = strdup(token);
                    token = strtok(NULL, " \n");
               }
               else if (strcmp(token, ">") == 0) {
                    //parse the next token as an output file
                    token = strtok(NULL, " \n");
                    ofname = strdup(token);
                    token = strtok(NULL, " \n");
               }
               else if (strcmp(token, "&") == 0) {
                    //make it so this command will run in the background
                    isForeground = 0;
                    break;
               }
               else {
                    //this token is the command name or an argument
                    args[k] = strdup(token);
                    token = strtok(NULL, " \n");
                    ++k;
               }
          }
          args[k] = NULL;

          //figure out what command we're dealing with
          if (args[0] == NULL || *(args[0]) == '#') {
               //it's either a blank line or a comment.
               //Do nothing and just reprint the prompt
               ;
          }
          else if (strcmp(args[0], "status") == 0) {
               printStatus(status);
          }
          else if (strcmp(args[0], "cd") == 0) {
               if (args[1] == NULL) {
                    chdir(getenv("HOME"));
               }
               else {
                    chdir(args[1]);
               }
          }
          else if (strcmp(args[0], "exit") == 0) {
               exit(0);
          }
          else {
               //fork and exec the command
               cpid = fork();
               switch (cpid) {
               case 0:     //I am the child
                    //make it so that foreground commands can be interrupted
                    if (isForeground) {
                         act.sa_handler = SIG_DFL;
                         sigaction(SIGINT, &act, NULL);
                    }

                    if (ifname != NULL) {
                         //handle input redirection
                         fi = open(ifname, O_RDONLY);
                         if (fi == -1) {
                              printf("smallsh: cannot open %s for input\n", ifname);
                              fflush(stdout);
                              _Exit(1);
                         }
                         if (dup2(fi, 0) == -1) {
                              perror("dup2");
                              _Exit(1);
                         }
                         close(fi);
                    }
                    else if (!isForeground) {
                         //make sure background processes don't try to 
                         //get input from keyboard
                         fi = open("/dev/null", O_RDONLY);
                         if (fi == -1) {
                              perror("open");
                              _Exit(1);
                         }
                         if (dup2(fi, 0) == -1) {
                              perror("dup2");
                              _Exit(1);
                         }
                    }

                    if (ofname != NULL) {
                         //handle output redirection
                         fo = open(ofname, O_WRONLY | O_CREAT | O_TRUNC, 0744);
                         if (fo == -1) {
                              printf("smallsh: cannot open %s for output\n", ofname);
                              fflush(stdout);
                              _Exit(1);
                         }
                         if (dup2(fo, 1) == -1) {
                              perror("dup2");
                              _Exit(1);
                         }
                         close(fo);
                    }
                    //execute the command
                    if (execvp(args[0], args)) {
                         //exec failed
                         printf("smallsh: command \"%s\" not found\n", args[0]);
                         fflush(stdout);
                         _Exit(1);
                    }
                    break;

               case -1:    //error with fork()
                    perror("fork");
                    status = 1;
                    break;

               default:    //I am the parent
                    if (isForeground) {
                         //wait for the foreground process to complete
                         waitpid(cpid, &status, 0);
                    }
                    else {
                         //print background process pid and move on
                         printf("background pid is %i\n", cpid);
                         break;
                    }
               }

          }
          //free the char pointers that we'll reuse
          for (int i = 0; args[i] != NULL; ++i) {
               free(args[i]);
          }
          free(ifname);
          ifname = NULL;
          free(ofname);
          ofname = NULL;

          //check for finished background processes
          cpid = waitpid(-1, &status, WNOHANG);
          while (cpid > 0) {
               printf("background pid %i is done: ", cpid);
               printStatus(status);
               cpid = waitpid(-1, &status, WNOHANG);
          }
     }

     return 0;
}

void printStatus(int status) {
     if (WIFEXITED(status)) {
          printf("exit value %i\n", WEXITSTATUS(status));
     }
     else {
          printf("terminated by signal %i\n", status);
     }
}