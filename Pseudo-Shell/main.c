/*

Author: Lucas Hyatt

Description: 
	- Implementation of a Pseudo-Shell:
		- A single-threaded UNIX system command-line interface.
	- Functionality is centered around file system navigation, manipulation and I/O. 
	- Offers two modes: 
		- File Mode (indicated using -f flag): Takes in a file containing commands for CLI
		- Interactive Mode (indicated using -command flag): Allows users to interact with CLI directly.
	- CLI recognizes 8 different commands: ls, pwd, mkdir, cd, cp, mv, rm, and cat...

Date: 10/20/2019

Notes: 
	- BUFSIZ is used throughout to indicate size of buffer. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include "command.h"

int main(int argc, char *argv[]){
	
	if(argc == 2){
		//Check for interactive mode
		if(strcmp(argv[1], "-command") == 0){
			//Main function variables
			size_t len = BUFSIZ;
			char *line = NULL;
			size_t chars = 0;
			char *token = NULL;

			char sourcePath [BUFSIZ];
			char destinationPath [BUFSIZ];
			char *check = NULL;

			int test = 1;
			while(test){
				//Print >>> then get the input string
				printf(">>> ");
				chars = getline(&line, &len, stdin);
				if(chars == 1){ //Initial check for enter
					continue;
				}
				else{
					//Tokenize input string
					char buffer [BUFSIZ];
					strcpy (buffer, line);
					token = strtok(buffer, " ");

					//This would indicate end of line
					while(1){
						if(token == NULL){
							break;
						}
						//If user enters exit, exits loop
						if(strcmp(token, "exit\n") == 0 || strcmp(token, "exit") == 0){
							test = 0;
							break;
						}
						//Call ls to command
						else if(strcmp(token, "ls\n") == 0 || strcmp(token, "ls") == 0){
							listDir();
							token = strtok(NULL, " ");
						}
						//Call pwd to command
						else if(strcmp(token, "pwd\n") == 0 || strcmp(token, "pwd") == 0){
							showCurrentDir();
							token = strtok(NULL, " ");
						}
						//Call to mkdir command
						else if(strcmp(token, "mkdir\n") == 0 || strcmp(token, "mkdir") == 0){
							//Get next token
							token = strtok(NULL, " ");
							check = &token[strlen(token)-1];
							if(strcmp(check, "\n") == 0){
								token[strlen(token)-1] = 0;
								makeDir(token);
							}
							else{
								makeDir(token);
							}
							token = strtok(NULL, " ");
						}
						//Call to cd command
						else if(strcmp(token, "cd\n") == 0 || strcmp(token, "cd") == 0){
							//Get next token
							token = strtok(NULL, " ");
							check = &token[strlen(token)-1];
							if(strcmp(check, "\n") == 0){
								token[strlen(token)-1] = 0;
								changeDir(token);
							}
							else{
								changeDir(token);
							}
							token = strtok(NULL, " ");
						}
						else if(strcmp(token, "mv\n") == 0 || strcmp(token, "mv") == 0){
							//Call to mv command
							token = strtok(NULL, " ");
							strcpy(sourcePath, token);
							token = strtok(NULL, " ");
							check = &token[strlen(token)-1];
							if(strcmp(check, "\n") == 0){
								token[strlen(token)-1] = 0;
								strcpy(destinationPath, token);
								moveFile(sourcePath, destinationPath);
							}
							else{
								strcpy(destinationPath, token);
								moveFile(sourcePath, destinationPath);
							}
							token = strtok(NULL, " ");
						}
						else if(strcmp(token, "cp\n") == 0 || strcmp(token, "cp") == 0){
							//Call to cp command
							token = strtok(NULL, " ");
							strcpy(sourcePath, token);
							token = strtok(NULL, " ");
							check = &token[strlen(token)-1];
							if(strcmp(check, "\n") == 0){
								token[strlen(token)-1] = 0;
								check = &token[strlen(token)-1];
								strcpy(destinationPath, token);
								copyFile(sourcePath, destinationPath);
							}
							else{
								strcpy(destinationPath, token);
								copyFile(sourcePath, destinationPath);
							}
							token = strtok(NULL, " ");
						}
						else if(strcmp(token, "rm\n") == 0 || strcmp(token, "rm") == 0){
							//Get next token
							token = strtok(NULL, " ");
							check = &token[strlen(token)-1];
							if(strcmp(check, "\n") == 0){
								token[strlen(token)-1] = 0;
								deleteFile(token);
							}
							else{
								deleteFile(token);
							}
							token = strtok(NULL, " ");
						}
						else if(strcmp(token, "cat\n") == 0 || strcmp(token, "cat") == 0){
							//Get next token
							token = strtok(NULL, " ");
							// check = &token[strlen(token)-1];
							int length = strlen (token);
							if (token [length -1] == '\n' || token [length -1] == '\r') {
								token [length -1] = '\0';
								if (length-1 > 0) {
									if (token [length -2] == '\n' || token [length -2] == '\r') {
										token [length -2] = '\0';
									}
								}
							}
							displayFile (token);
							token = strtok(NULL, " ");
						}
						else if(strcmp(token, ";\n") == 0 || strcmp(token, ";") == 0){
							//Basically move to next line
							token = strtok(NULL, " ");
						}
						else{
							printf("Unrecognized command.\n");
							break;
						}
					}
				}
			}

			//Free the allocated memory
			free(line);
		}
		else if(strcmp(argv[1], "-f") == 0){ printf("File required for file mode...\n"); }
		else{ printf("Unrecognized command...\n"); }
	}
	//Check for filemode
	else if(argc == 3){
		if(strcmp(argv[1], "-f") == 0){
			//Main function variables
			FILE *fptr = fopen(argv[2], "r");
			FILE *fout = freopen("output.txt", "w+", stdout);

			size_t len = BUFSIZ;
			char *line = NULL;
			size_t chars = 0;
			char *token = NULL;
			char sourcePath [BUFSIZ];
			char destinationPath [BUFSIZ];
			char buffer [BUFSIZ];
			char *check = NULL;

			while(!feof(fptr)){
				//Tokenize input string
				chars = getline(&line, &len, fptr);
				strcpy (buffer, line);
				token = strtok(buffer, " ");

				while(1){
					if(token == NULL){
						break;
					}
					//Call ls to command
					else if(strcmp(token, "ls\n") == 0 || strcmp(token, "ls") == 0){
						listDir();
						token = strtok(NULL, " ");
					}
					//Call pwd to command
					else if(strcmp(token, "pwd\n") == 0 || strcmp(token, "pwd") == 0){
						showCurrentDir();
						token = strtok(NULL, " ");
					}
					//Call to mkdir command
					else if(strcmp(token, "mkdir\n") == 0 || strcmp(token, "mkdir") == 0){
						//Get next token
						token = strtok(NULL, " ");
						check = &token[strlen(token)-1];
						if(strcmp(check, "\n") == 0){
							token[strlen(token)-1] = 0;
							makeDir(token);
						}
						else{
							makeDir(token);
						}
						token = strtok(NULL, " ");
					}
					//Call to cd command
					else if(strcmp(token, "cd\n") == 0 || strcmp(token, "cd") == 0){
						//Get next token
						token = strtok(NULL, " ");
						check = &token[strlen(token)-1];
						if(strcmp(check, "\n") == 0){
							token[strlen(token)-1] = 0;
							changeDir(token);
						}
						else{
							changeDir(token);
						}
						token = strtok(NULL, " ");
					}
					else if(strcmp(token, "mv\n") == 0 || strcmp(token, "mv") == 0){
						//Call to mv command
						token = strtok(NULL, " ");
						strcpy(sourcePath, token);
						token = strtok(NULL, " ");
						check = &token[strlen(token)-1];
						if(strcmp(check, "\n") == 0){
							token[strlen(token)-1] = 0;
							check = &token[strlen(token)-1];
							if(strcmp(check, "\r") == 0){
								token[strlen(token)-1] = 0;
								strcpy(destinationPath, token);
								moveFile(sourcePath, destinationPath);
							}
							else{
								strcpy(destinationPath, token);
								moveFile(sourcePath, destinationPath);
							}
						}
						else{
							strcpy(destinationPath, token);
							moveFile(sourcePath, destinationPath);
						}
						token = strtok(NULL, " ");
					}
					else if(strcmp(token, "cp\n") == 0 || strcmp(token, "cp") == 0){
						//Call to cp command
						token = strtok(NULL, " ");
						strcpy(sourcePath, token);
						token = strtok(NULL, " ");
						check = &token[strlen(token)-1];
						if(strcmp(check, "\n") == 0){
							token[strlen(token)-1] = 0;
							check = &token[strlen(token)-1];
							if(strcmp(check, "\r") == 0){
								token[strlen(token)-1] = 0;
								strcpy(destinationPath, token);
								copyFile(sourcePath, destinationPath);
							}
							else{
								strcpy(destinationPath, token);
								copyFile(sourcePath, destinationPath);
							}
						}
						else{
							strcpy(destinationPath, token);
							copyFile(sourcePath, destinationPath);
						}
						token = strtok(NULL, " ");
					}
					else if(strcmp(token, "rm\n") == 0 || strcmp(token, "rm") == 0){
						//Get next token
						token = strtok(NULL, " ");
						check = &token[strlen(token)-1];
						if(strcmp(check, "\n") == 0){
							token[strlen(token)-1] = 0;
							deleteFile(token);
						}
						else{
							deleteFile(token);
						}
						token = strtok(NULL, " ");
					}
					else if(strcmp(token, "cat\n") == 0 || strcmp(token, "cat") == 0){
						//Get next token
						token = strtok(NULL, " ");
						check = &token[strlen(token)-1];
						if(strcmp(check, "\n") == 0){
							token[strlen(token)-1] = 0;
							displayFile(token);
						}
						else{
							displayFile(token);
						}
						token = strtok(NULL, " ");
					}
					else if(strcmp(token, ";\n") == 0 || strcmp(token, ";") == 0){
						//Basically move to next line
						token = strtok(NULL, " ");
					}
					else{
						printf("Unrecognized command.\n");
						break;
					}
				}
			}
			//Free the allocated memory
			free(line);
			fclose(fptr);
		}
		// Tried to run in unspecified mode.
		else{
			printf("Unrecognized command...\n");
		}
	}
	else if(argc < 2){
		printf("Must either specify -command (interactive) or -f (file)...\n");
	}
	else{
		printf("Too many arguments...\n");
	}
	return 1; 
}