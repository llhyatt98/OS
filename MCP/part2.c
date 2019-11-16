#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> //For implementing signals

/* 
	- Read workload from input file (main)
	- Launch program to run as separate process using fork() and exec()
	- Once all are running, wait until all programs terminate using wait()
	- After all programs terminate, exit()
*/

int num_progs(char *file){
	FILE *fptr = fopen(file, "r");
	int ch = 0;
	int count = 0;
	ch  = fgetc(fptr);
	while(ch != EOF){
		if(ch == '\n'){ //If letter is \n, we hit the end of the line
			count ++; //Increase count
		}
		ch  = fgetc(fptr); //Move to next letter
	}
	fclose(fptr);
	return count;
};

void sig_handler(int signum, siginfo_t *siginfo, void *context){ 
	sigset_t set; //For use in sigwait()
	sigaddset(&set, 10);
	int sig;
	int *sig_p = &sig;
	int ret = 1;

	ret = sigwait(&set, sig_p);
	if(ret != 0){
		printf("Unsuccessful call to sigwait()\n");
	}
}

void sig_function(pid_t *pids, int signum, char *func_argv[]){ //Pass in argv so I can access file for loop
	int i, w_status;
	int num = num_progs(func_argv[1]); //The number of programs
	//Prints about status of child after signal should be here
	for(i = 0; i < num; i++){
		if(signum == SIGUSR1){
			printf("Child %d received SIGUSR1\n", pids[i]);
		}
		else if(signum == SIGSTOP){
			waitpid(pids[i], &w_status, WNOHANG); // Using waitpid to check if child is alive. 
			if(w_status == 0){
				 printf("Child %d received SIGSTOP with w_status: %d (still alive).\n", pids[i], w_status);
			}
			else{
				printf("Child not alive after SIGSTOP with w_status: %d (killed)\n", w_status);
			}
		}
		else if(signum == SIGCONT){
			waitpid(pids[i], &w_status, WNOHANG); // Using waitpid to check if child is alive. 
			if(w_status == 0){
				 printf("Child %d received SIGCONT with w_status: %d (still alive).\n", pids[i], w_status);
			}
			else{
				printf("Child terminated after SIGCONT with w_status: %d (killed)\n", w_status);
			}
		}
		kill(pids[i], signum);
	}
}

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("No file provided\n");
		exit(EXIT_FAILURE);
	}

	int num_prog = num_progs(argv[1]);
	pid_t pid[num_prog]; //Process pool initialization

	//Signal Usage
	struct sigaction act; 
	act.sa_sigaction = &sig_handler;
	act.sa_flags = SA_SIGINFO; 

	int i, count,w_status,  j = 0;
	char *line = NULL;
	size_t len = 0;

	sigset_t set; //For use in sigwait()
	sigaddset(&set, 10);
	int sig;
	int *sig_p = &sig;
	int ret = 1;

	FILE *fp = fopen(argv[1], "r");

	while(getline(&line, &len, fp) != -1){
		char *args[num_prog]; //Helper variable for adding to argument array.
		char *str = strtok(line, " ");
		i = 0;
		while(str){
			char *check = &str[strlen(str)-1];
			if(strcmp(check, "\n") == 0){ //Check for newline at the end
				str[strlen(str)-1] = 0;
				args[i] = strdup(str);
				str = strtok(NULL, " ");
				i++;
				args[i] = NULL;
			}
			else{ //No newline is present
				args[i] = strdup(str);
				str = strtok(NULL, " ");
				i++;
				args[i] = NULL;
			}
		}

		pid[j] = fork();
		sigaction(SIGUSR1, &act, NULL); //Initializing signal

		if(pid[j] < 0){
			perror("Forking failed...");
			exit(EXIT_FAILURE);
		}
		if(pid[j] == 0){ //This is the child process.
			// printf("We are in child.\n");
			ret = sigwait(&set, sig_p);
			if(ret != 0){
				printf("Child %d failed to wait!\n", pid[j]);
			}
			execvp(args[0], args);
			printf("Failed to exec: %s\n", args[0]);
			exit(-1);
		}
		j++;

		count = 0;
		while(args[count] != NULL){
			free(args[count]);
			count++;
		}
	}

	//Now the parent
	printf("Children Starting to Wait\n");
	sleep(1);
	printf("All Children waiting...\n");
	sleep(1);
	printf("Sending Signal to Resume Children...\n");
	sleep(2);
	sig_function(pid, SIGUSR1, argv);
	sleep(2);
	printf("Sending SIGSTOP to children...\n");
	sig_function(pid, SIGSTOP, argv);
	sleep(2);
	printf("Sending SIGCONT to children...\n");
	sleep(1);
	sig_function(pid, SIGCONT, argv);
	sleep(1);
	printf("Exiting...\n");
	sleep(1);

	for(i = 0; i < num_prog; i++){
		waitpid(pid[i], &w_status, 0);
	}

	fclose(fp);
	free(line);

	return 1;
}