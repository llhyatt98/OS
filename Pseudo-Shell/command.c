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

void showCurrentDir(){
	char *buff = (char *)malloc(BUFSIZ * sizeof(char));
	char *ptr = getcwd(buff, BUFSIZ);
	char new[3] = "\n";
	strcat(ptr, new); //Adding new line
	write(1, ptr, strlen(ptr));
	free(buff);
} 

void listDir(){
	char *buff = (char *)malloc(BUFSIZ * sizeof(char));
	char *ptr = getcwd(buff, BUFSIZ);
	char new[3] = " ";
	char old[3] = "\n";

	struct dirent *dp;
	DIR *dir = opendir(ptr); //Opening current directory for traversal

	if(dir != NULL){ //Check that directory was properly opened
		while(1){
			dp = readdir(dir);
			if(dp != NULL){
				strcat(dp->d_name, new); //Adding new line
				write(1, dp->d_name, strlen(dp->d_name));
			}
			else{ break; } //Reached the end of contents of directory.
		}
		closedir(dir);
		write(1, old, strlen(old));
	}	
	else{ printf("Cannot open directory."); } //Printing if there is an error
	free(buff);
} 

void makeDir(char *dirName){
	//Will return 0 upon success, implements mkdir system call
	if(mkdir(dirName, 0777) != 0){
		printf("Failed to make directory\n"); //Printing if there is an error
	}
} 

void changeDir(char *dirName){
	//Will return 0 upon success, implements chdir system call
	if(chdir(dirName) != 0){
		printf("Failed to change directory\n"); //Printing if there is an error
	}
} 

void moveFile(char *sourcePath, char *destinationPath){
	struct stat dest_buf; //Used to check file or directory 
	memset((void *)&dest_buf, 0, sizeof(struct stat));
	struct stat src_buf; //Used to check file or directory
	memset((void *)&src_buf, 0, sizeof(struct stat));
	char *tok = (char *)malloc(40 * sizeof(char));
	char *save = (char *)malloc(40 * sizeof(char));
	strcpy(tok, sourcePath);
	char *add = "/";
	char *src = NULL;
	int test = 0;

	//Used to check file or directory.
	stat(destinationPath, &dest_buf);
	stat(sourcePath, &src_buf);

	if(S_ISDIR(dest_buf.st_mode) && S_ISDIR(src_buf.st_mode)){ //Both are directories
		while(tok != NULL){ // Getting the last file portion of source and appending to destination. 
			tok = strtok_r(NULL, "/", &save);
		}
		strcat(destinationPath, add);
		strcat(destinationPath, tok);
	}
	else if(S_ISDIR(dest_buf.st_mode) && !S_ISDIR(src_buf.st_mode)){ //Destination is directory but not source
		
		if(strchr(sourcePath, '/')){ //It was a path
			src = strtok_r(tok, "/", &save);
			while(1){ // Getting the last file portion of source and appending to destination. 
				tok = strtok_r(NULL, "/", &save);
				if(tok == NULL){
					break;
				}
				else{
					test = 1;
					src = (char *)malloc(40 * sizeof(char));
					strcpy(src, tok);
				}
			}

			strcat(destinationPath, add);
			strcat(destinationPath, src);
			if(test){
				free(src);
			}
		}
		else{ //It was a single file.
			strcat(destinationPath, add);
			strcat(destinationPath, sourcePath);	
		}
	}
	int mv = rename(sourcePath, destinationPath);
	if(mv != 0){
		printf("Failed to rename file.\n");
	}
	free(tok);
	free(save);
}

void copyFile(char *sourcePath, char *destinationPath){
	struct stat dest_buf; //Used to check file or directory 
	memset((void *)&dest_buf, 0, sizeof(struct stat));
	struct stat src_buf; //Used to check file or directory
	memset((void *)&src_buf, 0, sizeof(struct stat));
	char *tok2 = (char *)malloc(40 * sizeof(char));
	char *save = (char *)malloc(40 * sizeof(char));
	strcpy(tok2, sourcePath);
	char *add = "/";
	char *src = NULL;
	int test = 0;

	//Used to check file or directory.
	stat(destinationPath, &dest_buf);
	stat(sourcePath, &src_buf);

	if(S_ISDIR(dest_buf.st_mode) && S_ISDIR(src_buf.st_mode)){ //Both are directories
		while(tok2 != NULL){ // Getting the last file portion of source and appending to destination. 
			tok2 = strtok_r(NULL, "/", &save);
		}
		strcat(destinationPath, add);
		strcat(destinationPath, tok2);
	}
	else if(S_ISDIR(dest_buf.st_mode) && !S_ISDIR(src_buf.st_mode)){ //Destination is directory but not source
		if(strchr(sourcePath, '/')){ //It was a path
			src = strtok_r(tok2, "/", &save);
			while(1){ // Getting the last file portion of source and appending to destination. 
				tok2 = strtok_r(NULL, "/", &save);
				if(tok2 == NULL){
					break;
				}
				else{
					test = 1;
					src = (char *)malloc(40 * sizeof(char));
					strcpy(src, tok2);
				}
			}
			strcat(destinationPath, add);
			strcat(destinationPath, src);
			if(test){
				free(src);
			}
		}
		else{ //It was a single file.
			strcat(destinationPath, add);
			strcat(destinationPath, sourcePath);		
		}
	}

	// Now using previous logic from mv, implement open(), read(), write() to keep file
	char *buff = (char *)malloc(BUFSIZ * sizeof(char));
	memset((void *)buff, 0, BUFSIZ);
	int src_path, dest_path, rd, wr;
	src_path = open(sourcePath, O_RDONLY);
	dest_path = open(destinationPath, O_WRONLY | O_CREAT);
	rd = read(src_path, buff, BUFSIZ);
	wr = write(dest_path, buff, BUFSIZ);

	close(src_path);
	close(dest_path);

	free(buff);
	free(tok2);
	free(save);
}

void deleteFile(char *filename){
	//Removes file "filename" from current directory. 
	if(filename){
		remove(filename);
	}
}

void displayFile(char *filename){
	//printf("%s\n", filename);
	size_t size = BUFSIZ; //Just in case the file happens to be larger
	char buff[size];
	char *pwd = getcwd(buff, size);
	strcat(buff, "/");
	strcat(buff, filename);
	// printf("%s\n", buff);
	char *line = (char *)malloc(size * sizeof(char));
	int src = open(filename, O_RDONLY);
	if(src < 0){
		printf("Error not opened...\n");
	}
	else{
		read(src, line, size);
		//printf("THIS IS THE LINE:\n%s\n", line);
		write(1, line, strlen(line));
	}
	close(src);
	free(line);
} 