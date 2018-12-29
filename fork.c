#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
pid_t	pid;

/* fork a child process */
pid=fork();
if (pid < 0) 		/* error occurred */
	{		
	fprintf (stderr, "Fork Failed");
	exit (-1);
	}
else if (pid == 0)	/* Child Process */
	{
	execlp("/bin/ls","ls",NULL);
  	}
else  		/* parent process */
	{
	wait(NULL); 	/* parent will wait for the child to complete */
	printf("Child Complete\n");
	exit(0);
	}
}
