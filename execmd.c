

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators

void errmsg(char *, char *);                   // error message printout
void syserrmsg(char *, char *);                // system error message printout
void execute(char **, bool shellstatus);            // execute command from arg array

/*******************************************************************/

int main (int argc, char ** argv)
{
    FILE * ostream = stdout;                   // (redirected) o/p stream
    char linebuf[MAX_BUFFER];                  // line buffer
    char cwdbuf[MAX_BUFFER];                   // cwd buffer
    char * args[MAX_ARGS];                     // pointers to arg strings
    char ** arg;                               // working pointer thru args
    char * prompt = "==>" ;                    // shell prompt
    bool bgstatus;                             // background or foreground
    int execargs;                              // execute command in args flag
    int i;                                     // working index

    // keep reading input until "quit" command or eof of redirected input 
    while (!feof(stdin)) {

        bgstatus = false;   // foreground execution

// set up prompt
if(getcwd(cwdbuf, sizeof(cwdbuf)))   // read current working directory
            printf("%s%s", cwdbuf,prompt);   // output as prompt
        else
            printf("getcwd ERROR %s",prompt);
        fflush(stdout);

// get command line from input
  if (fgets(linebuf, MAX_BUFFER, stdin )) { // read a line
    
            // Backgound Execution ?
    if (linebuf[strlen(linebuf)-2] == '&') {
               bgstatus = true;
       linebuf[strlen(linebuf)-2] = ' ';
    }

            // tokenize the input into args array 
    arg = args;
            *arg++ = strtok(linebuf,SEPARATORS);   // tokenize input
            while ((*arg++ = strtok(NULL,SEPARATORS)));
                                               // last entry will be NULL 
 
            if (args[0]) {                     // if there's anything there
// "quit" command
                if (!strcmp(args[0],"quit")) {
                    break;
                }
                execute(args, bgstatus);
            }
        }
    }
    return 0; 
}

/***********************************************************************
  void execute(char ** args, shellstatus sstatus);

  fork and exec the program and command line arguments in args
  if foreground flag is TRUE, wait until pgm completes before
    returning
***********************************************************************/

void execute(char ** args, bool bgstatus)
{
    int status;
    pid_t child_pid;

    switch (child_pid = fork()) {
        case -1:
            syserrmsg("fork",NULL);
            break;
        case 0:

   // execution in child process
    execvp(args[0], args);
            syserrmsg("exec failed -",args[0]);
            exit(127);
    }

    // continued execution in parent process
    if (!bgstatus) waitpid(child_pid, &status, WUNTRACED);
}


/********************************************************************
void errmsg(char * msg1, char * msg2);

print an error message (or two) on stderr

if msg2 is NULL only msg1 is printed
if msg1 is NULL only "ERROR: " is printed
*******************************************************************/

void errmsg(char * msg1, char * msg2)
{
    fprintf(stderr,"ERROR: ");
    if (msg1)
        fprintf(stderr,"%s; ", msg1);
    if (msg2)
        fprintf(stderr,"%s; ", msg2);
    return;
    fprintf(stderr,"\n");
}

/********************************************************************
  void syserrmsg(char * msg1, char * msg2);

  print an error message (or two) on stderr followed by system error
  message.

  if msg2 is NULL only msg1 and system message is printed
  if msg1 is NULL only the system message is printed
 *******************************************************************/

void syserrmsg(char * msg1, char * msg2)
{
    errmsg(msg1, msg2);
    perror(NULL);
    return;
}



