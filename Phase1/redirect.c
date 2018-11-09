//gdb attache pid
#include "common_impl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

int main(int argc, char *argv[]){
	int tube_stdout[2];
	/* creation du tube pour rediriger stderr */
	int tube_stderr[2];
	int pid = fork();
	int num_procs_creat = 0;
	if(pid == -1) ERROR_EXIT("fork");

	if (pid == 0) { /* fils */
		int j;
		/* redirection stdout */
		//close(STDOUT_FILENO);
		dup2(STDOUT_FILENO,tube_stdout[0]);
		close(tube_stdout[0]); //suppression tube en lecture


		/* redirection stderr */
		//close(STDERR_FILENO);
		dup2(STDERR_FILENO,tube_stderr[0]);
		close(tube_stderr[0]);


		/* Creation du tableau d'arguments pour le ssh */
		printf(" fils: %d\n",pid);
		printf(" tub_stderr[1]: %d\n", tube_stdout[1]);




		/* jump to new prog : */
		/* execvp("ssh",newargv); */
		return 33;


	} else  if(pid > 0) { /* pere */
		/* fermeture des extremites des tubes non utiles */
		close(tube_stdout[1]);
		close(tube_stderr[1]);
		wait(NULL);
		num_procs_creat++;
	}


}
