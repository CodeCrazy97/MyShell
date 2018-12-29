/*
 *
 * Student Name: Ethan Vaughan
 * ID: 901572108
 * CSC 400, program 1
 *
 * September 17, 2018
 *
 */

#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#define clear() printf("\033[H\033[J");        // This is used to clear the screen.

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators

using namespace std; // Prevents the necessity of using std before cout/cin.

/////////////////////////////////////////////////////////////////////////////////////
// Define functions.

void errmsg(char *, char *); // error message printout
void syserrmsg(char *, char *, bool bgstatus); // system error message printout
void execute(char **, bool shellstatus); // execute command from arg array
int getdir(string dir, vector<string> &files); // Print out contents of a directory.

/*******************************************************************/


int main(int argc, char ** argv) {
    FILE * ostream = stdout; // (redirected) o/p stream
    char linebuf[MAX_BUFFER]; // line buffer
    char cwdbuf[MAX_BUFFER]; // cwd buffer
    char * args[MAX_ARGS]; // pointers to arg strings
    char ** arg; // working pointer thru args
    char * prompt = "==>"; // shell prompt
    bool bgstatus; // background or foreground
    int execargs; // execute command in args flag
    int i; // working index

    // keep reading input until "quit" command or eof of redirected input
    while (!feof(stdin)) {

        bgstatus = false; // foreground execution

        // set up prompt
        if (getcwd(cwdbuf, sizeof (cwdbuf))) // read current working directory
            printf("%s%s", cwdbuf, prompt); // output as prompt
        else
            printf("getcwd ERROR %s", prompt);
        fflush(stdout);

        // get command line from input
        if (fgets(linebuf, MAX_BUFFER, stdin)) { // read a line

            // Backgound Execution ?
            if (linebuf[strlen(linebuf) - 2] == '&') {
                bgstatus = true;
                linebuf[strlen(linebuf) - 2] = ' ';
            }

            // tokenize the input into args array
            arg = args;
            *arg++ = strtok(linebuf, SEPARATORS); // tokenize input
            while ((*arg++ = strtok(NULL, SEPARATORS)));
            // last entry will be NULL

            if (args[0]) { // if there's anything there
                // "quit" command
                if (!strcmp(args[0], "quit")) {
                    break;
                } else if (!strcmp(args[0], "clr")) { // User wants to clear the screen.
                    clear();
                } else if (!strcmp(args[0], "dir")) { // Either report contents of a directory...
                    if (args[1] != NULL) { // Directory argument was supplied. Report contents of the directory.
                        vector<string> files = vector<string>();

                        int x = 1; // Start the counter just past the "dir" command.
                        string directory = ""; // This variable will hold the directory to list files of.
                        while (args[x] != NULL) { // While there is still a directory name.
                            directory += args[x]; // Create the directory variable. EXAMPLE: suppose user wishes to list contents of 'Test Folder'. This will first add "Test" to directory and then add "Folder" to it.
                            if (args[x + 1] != NULL) // If there is more to the name of the direcotry, then add a space between the words.          
                                directory += " "; // Add a space between the directory words.
                            x++;
                        }

                        // Get the contents of the directory. Store them in files[].
                        getdir(directory.c_str(), files);

                        for (unsigned int i = 0; i < files.size(); i++) {
                            cout << files[i] << endl; // Iterate over the contents.
                        }
                    } else { // No directory argument was supplied. Report contents of current directory.
                        execute(args, bgstatus);
                    }
                } else if (!strcmp(args[0], "echo")) { // Print text to the screen.
                    int i = 1;
                    while (args[i] != NULL) { // Iterate through the comment(s).
                        printf("%s ", args[i]); // Print the comment.
                        i++;
                    }

                    // Print new lines after the comment.
                    printf("\n");
                    printf("\n");
                } else if (!strcmp(args[0], "cd")) {
                    // First, check if the <directory> argument is present.
                    if (args[1] == NULL) { // User entered "cd" exactly. Will now need to print the current directory.
                        string s(cwdbuf); // Turn the current working directory into a string to make iterating over it easier.
                        string currDir = ""; // currDir will hold the current directory.
                        int lastChar = s.length() - 1; // The length of the current working directory.
                        while (s.at(lastChar) != '/') { // Find the last occurrence of the forward slash. Every character after the last forward slash must be appended to the current directory. (The last forward slash indicates the start of the name of the current directory.)
                            currDir = s.at(lastChar) + currDir; // Builds the current directory.
                            lastChar--;
                        }
                        cout << "Current directory: " << currDir << "\n";
                    } else { // <directory> argument was supplied.
                        // Attempt to change directories to what the user entered. Generate an error if the directory entered by the user doesn't exist.
                        getcwd(cwdbuf, sizeof (cwdbuf));

                        int x = 1; // Start the counter just past the "dir" command.
                        string directory = ""; // This variable will hold the directory to cd into.
                        while (args[x] != NULL) { // While there is still a directory name.
                            directory += args[x]; // Create the directory variable. EXAMPLE: suppose user wishes to cd into 'Test Folder'. This will first add "Test" to directory and then add "Folder" to it.
                            if (args[x + 1] != NULL) // If there is more to the name of the direcotry, then add a space between the words.          
                                directory += " "; // Add a space between the directory words.
                            x++;
                        }

                        int rc = chdir(directory.c_str()); // Return code of changing directories.
                        //cout << "rc = " << rc << "\n";
                        if (rc != -1) { // If the directory change was successful, then create the new prompt.
                            // Set the current working directory (cwd).
                            if (getcwd(cwdbuf, sizeof (cwdbuf)) != NULL) { // Successful cd.
                                /* Nothing to do. Action was taken in line above. */
                            } else {
                                perror("Error getting the current working directory.");
                                return 1;
                            }
                        } else { // The program will reach this point only if the user typed "cd" followed by an invalid directory name.
                            cout << "Invalid directory name." << "\n";
                        }
                    }
                } else { // Attempt to execute a program. The program will reach this point if the user did not enter one of the cd, dir, echo, clr, or quit commands.
                    execute(args, bgstatus);
                }
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

void execute(char ** args, bool bgstatus) {
    int status;
    pid_t child_pid;

    switch (child_pid = fork()) {
        case -1:
            syserrmsg("fork", NULL, bgstatus);
            break;
        case 0:

            // execution in child process

            execvp(args[0], args);

            syserrmsg("exec failed -", args[0], bgstatus);

            if (bgstatus) { // If it was a background status, then return to the shell and print the current working directory.
                // set up prompt
                char cwdbuf[MAX_BUFFER]; // cwd buffer
                char * prompt = "==>"; // shell prompt
                if (getcwd(cwdbuf, sizeof (cwdbuf))) // read current working directory
                    printf("%s%s", cwdbuf, prompt); // output as prompt
                else
                    printf("getcwd ERROR %s", prompt);
                fflush(stdout);
            }
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

void errmsg(char * msg1, char * msg2, bool bgstatus) {
    if (bgstatus) { // Need to print a new line, since input with "&" at the end will be consumed.
        printf("\n");
    }
    fprintf(stderr, "ERROR: ");
    if (msg1)
        fprintf(stderr, "%s; ", msg1);
    if (msg2)
        fprintf(stderr, "%s; ", msg2);
    return;
    fprintf(stderr, "\n");
}

/********************************************************************
  void syserrmsg(char * msg1, char * msg2);

  print an error message (or two) on stderr followed by system error
  message.

  if msg2 is NULL only msg1 and system message is printed
  if msg1 is NULL only the system message is printed
 *******************************************************************/

void syserrmsg(char * msg1, char * msg2, bool bgstatus) {
    errmsg(msg1, msg2, bgstatus);
    perror(NULL);
    return;
}

/* Function to display the contents of a directory. */
int getdir(string dir, vector<string> &files) {
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}
