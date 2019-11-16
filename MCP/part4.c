#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> //For implementing signals
#include <time.h>
#include <errno.h>

/* 
	- Read workload from input file (main)
	- Launch program to run as separate process using fork() and exec()
	- Once all are running, wait until all programs terminate using wait()
	- After all programs terminate, exit()
*/

int k; //Global index
pid_t pid[10]; //Global array pool
int num_prog; 

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
}

void proc(pid_t *pids){ 
	//Takes in pids to know where to find info regarding process
	int i, num;
	num = num_prog;
	char paths[num][30]; //Variable to store paths to /proc
	FILE *fptr; 
	//Variables for getline()
	size_t len = BUFSIZ;
	char *line = NULL;
	int line_count, w_status, status;

	for(i = 0; i < num; i++){
		sprintf(paths[i], "/proc/%d/status", pid[i]);
		// printf("Paths at %d is %s\n", i, paths[i]);
	}

	for(i = 0; i < num; i++){
		line_count = 1; //Determines which lines to print from status. 
		fptr = fopen(paths[i], "r");
		printf("\n");
		if(fptr != NULL){
			while(!feof(fptr)){ //Read lines from the file.
				getline(&line, &len, fptr);
				// if (line_count == 1 || line_count == 2 || line_count == 5 || line_count == 11 || line_count == 16 ||  line_count == 31 ||  line_count == 33 ||  line_count == 35 ||  line_count == 36)
				if(line_count % 3 == 0)
				{
					printf("%s", line);
				}
				line_count++; 
			}
			fclose(fptr);
			free(line);
		}
	}
}

int is_alive(pid_t *pids){
	int nums = num_prog;
	int i = 0, w_status, flag = 1, status;
	printf("\n");
	while(1){
		if(i == nums){  //Checked all processes
			return flag;
		}
		status = waitpid(pid[i], &w_status, WNOHANG);
		if(status > -1){ //Process is alive.
			flag = 0;
			return flag;
		}
		else{ //Check next process. 
			i++;
		}
	}
	printf("\n");
}

void sig_handler(int signum, siginfo_t *siginfo, void *context){  //SIGALARM 
	//If no process is alive, handler does nothing 
	int w_status, status;
	if(is_alive(pid)){
		return;
	}
	while(1) //Check that there is a living process (j+1) to hit every process.
	{
		status = waitpid(pid[k], &w_status, WNOHANG);
		if(status == 0){ //The process is living
			// printf("-----> SIGSTOPPING INDEX %d WHICH IS: %d\n", k, pid[k]);
			kill(pid[k], SIGSTOP);
			k++;
			if(k == num_prog){
				k = 0;
			}
			break;
		}
		else{
			k++;
			if(k == num_prog){
				k = 0;
			}
		}
	}

	printf("\nScheduling next process for execution...\n");
	
	while(1){
		status = waitpid(pid[k], &w_status, WNOHANG);
		// printf("STATUS FOR CHILD %d IS %d\n", pid[k], status);
		if(status == 0){ //The process is living
			printf("----> Next process scheduled has PID: %d\n", pid[k]);
			kill(pid[k], SIGCONT);		
			break;
		}
		else{
			k++;
			if(k == num_prog){
				k = 0;
			}
		}
	}	
}

void sig_function(pid_t *pids, int signum, int flag){ 
	int i = 0, w_status, status;
	int num = num_prog;
	//Prints about status of child after signal should be here
	if(flag == 1){
		i = 1;
	}
	for(; i < num; i++){
		kill(pids[i], signum);
		status = waitpid(pids[i], &w_status, WNOHANG);
		if(signum == SIGUSR1){
			printf("Child %d received SIGUSR1\n", pids[i]);
		}
		else if(signum == SIGSTOP){ 
			if(status > -1){
				 printf("Child %d received SIGSTOP with w_status: %d (still alive).\n", pids[i], w_status);
			}
			else{
				printf("Child not alive after SIGSTOP with w_status: %d (killed)\n", w_status);
			}
		}
		else if(signum == SIGCONT){
			if(status >  -1){
				 printf("Child %d received SIGCONT with w_status: %d (still alive).\n", pids[i], w_status);
			}
			else{
				printf("Child terminated after SIGCONT with w_status: %d (killed)\n", w_status);
			}
		}
	}
}

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("No file provided\n");
		exit(EXIT_FAILURE);
	}

	num_prog = num_progs(argv[1]); //Defining number of programs. 

	//Signal Usage
	struct sigaction act;
	act.sa_sigaction = &sig_handler;
	act.sa_flags = SA_RESTART;

	int i, count;
	char *line = NULL;
	size_t len = 0;

	sigset_t set; //For use in sigwait()
	sigaddset(&set, 10);
	int sig;
	int *sig_p = &sig;
	int ret = 1, j = 0;

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
	sigaction(SIGALRM, &act, NULL); //Initializing signal

	printf("Children Starting to Wait\n");
	sleep(1);
	printf("All Children waiting...\n");
	sleep(1);
	printf("Sending Signal to Wake Children...\n");
	sleep(2);
	sig_function(pid, SIGUSR1, 0);
	sleep(2);

	printf("Sending SIGSTOP to all but one children...\n");
	sig_function(pid, SIGSTOP, 1); //Flag = 1 all but one stopped 
	sleep(2);

	// Beginning of alarm
	time_t now = time(NULL);
	time_t elapsed = time(NULL) + 3;
	while(1){
		if(is_alive(pid)){ //Will return 1 if all children are dead. 
			printf("ALL CHILDREN DEAD\n");
			break;
		}
		alarm(3);
		now = time(NULL);
		while(1){
			elapsed = time(NULL);
			proc(pid);
			sleep(2);
			system("clear");
			if(elapsed - now >= 3){
				break;
			}
		}
	}	

	printf("Exiting...\n");

	fclose(fp);
	free(line);

	return 1;
}