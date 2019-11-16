#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

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

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("No file provided\n");
		exit(EXIT_FAILURE);
	}

	int num_prog = num_progs(argv[1]);
	pid_t pid[num_prog]; //There will be num_prog process ID's

	int i, count, j = 0;
	char *line = NULL;
	size_t len = 0;
	int w_status;

	
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
		// printf("%d\n", pid[j]);

		if(pid[j] < 0){
			perror("Forking failed...");
			exit(EXIT_FAILURE);
		}
		if(pid[j] == 0){
			//This is the child process.
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

	//Anything happening here is the parent. 
	for(i = 0; i < num_prog; i++){
		waitpid(pid[i], &w_status, 0);
	}

	fclose(fp);
	free(line);

	return 1;
}