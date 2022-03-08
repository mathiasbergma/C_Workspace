/*
 * main.c
 *
 *  Created on: Feb 14, 2022
 *      Author: bergma
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFSIZE 32

int main(void)
{
	// Create some variables
	char input[BUFSIZE];
	char * input_ret;
	int pid_t;
	int child = 0;
	int pipefd[2];

	// Enter a loop
	while (1)
	{
		// Create a pipe for communication
		pipe(pipefd);
		// Start a child process
		pid_t = fork();
		// If i'm a child?
		if (!pid_t)
		{
			child = 1;
			// Close write
			close(pipefd[1]);
			break;
		}
		// Tell the world that a child has been born
		printf("main(): Created child with PID [%d]\n", pid_t);
		// Get input from terminal
		printf("Enter some input, that will be sent to child process: (CTRL^D to exit)\n");
		input_ret = fgets(input, BUFSIZE, stdin);
		// Check for exit statement
		if(!input_ret)
		{
			// Inform the user that program is shutting down
			printf("Received kill signal. Shutting down child and parent!\n");
			// Close read end of pipe, as this process is the parent
			close(pipefd[0]);
			// Close write end of pipe to send EOF
			close(pipefd[1]);
			// Clean up after child when it is dead by waiting until death
			wait(NULL);
			break;
		}
		// Close read end of pipe, as this process is the parent
		close(pipefd[0]);
		// Send input to child
		write(pipefd[1], input, strlen(input));
		// Close write end of pipe to send EOF
		close(pipefd[1]);
		// Clean up after child when it is dead by waiting until death
		wait(NULL);
		// Repeat
	}
	// Is the process a child?
	if (child)
	{
		// Close write end of pipe
		close(pipefd[1]);
		while (read(pipefd[0], input, BUFSIZE) > 0) // Read until EOF is received
		{
			printf("Child() PID [%d] has received: %s \n",getpid(),input);
			printf("Child() PID [%d] signing off\n", getpid());
		}
		exit(EXIT_SUCCESS);
	}

	return 0;
}
