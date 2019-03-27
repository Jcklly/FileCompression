#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include "fileObject.h"
#include <sys/stat.h>
#include <fcntl.h>


void correctCall(int argc, char* argv[]) {

	nodeArray = malloc(10*sizeof(*nodeArray));


	if( (argv[1][1]  == 'b') || (argv[2][1] == 'b') ) {	
		if(argc == 3) {
				// 0 means there is no -R flag
			buildCB(argv[2], 0);
		} else if(argc == 4) {
				// 1 means there is an -R flag
			buildCB(argv[3], 1);
		} else {
			fprintf(stderr, "Invalid number of inputs.\n");
			exit(0);
		}
	} else {
		printf("Different\n");
	}

	//printf("%c\n", *argv[2]);
}




int buildCB(char* s_dir, int flag) {

	//char* t = ".//dir1/dir2";

	DIR* d = opendir(s_dir);
	char* newDir;	


		// Checks if -R flag is in input.
	if( flag == 0 ) {
		createTokenArray(&s_dir, flag);
	} else {
		if( d != NULL ) {
			
			struct dirent* status = NULL;
			status = readdir(d);

			do {
			
				char* fullDir = concatDir(s_dir, status->d_name);	
//				printf("%s\n", fullDir);

				if( status->d_type == DT_REG ) {
					createTokenArray(&fullDir, flag);
				} else {
					;
				}

					// Recurssive method for going through subdirectories	
				if( status->d_type == DT_DIR ) {
					if( (strcmp(status->d_name, ".") == 0) || (strcmp(status->d_name, "..") == 0) ) {
						;
					} else {
						newDir = concatDir(s_dir, status->d_name);
						buildCB(newDir, flag);
						free(newDir);
					}
				}
				free(fullDir);				
				status = readdir(d);
			} while ( status != NULL );
		}	
		closedir(d);
	}
	return 0;
}


	// Given a file or directory, will create an unsorted array of nodes containing tokens and frequency's.
void createTokenArray(char** file, int flag) {
//	printf("%s\n", *file);	

	static int num = 0;

//	char* test = "fileObject.h";

	int fd = open(*file, O_RDWR | O_CREAT, 00600);


	size_t fileSize;
	int count = 0;
	char buffer;
	char* token;

	off_t cp = lseek(fd, (size_t)0, SEEK_CUR);	
	fileSize = lseek(fd, (size_t)0, SEEK_END); 
	lseek(fd, cp, SEEK_SET);
	

	if (fd == -1) {
		printf("Error on open: %d\n", errno);
	}
		
	do {

		read(fd, &buffer, 1);
				
		if( buffer == '\n' ) {
			printf("NNNNN");
		} else {
			printf("%c", buffer);
		}


		--fileSize;
	} while (fileSize > 0);
	close(fd);

}









	// Used for recurssion. Concats directory strings.
char* concatDir(char* original, char* toAdd) {

	int bufferSize = strlen(original) + strlen(toAdd) + 2;
	
	char* concat = malloc(bufferSize);
	strcpy(concat, original);
	strcat(concat, "/");
	strcat(concat, toAdd);

	return concat;
}








/*	// Checks if given directory is at DT_ file. Returns 1 if yes, 0 if no.
int isTXT(char* s) {

	int final = 0;
	char* last = &s[strlen(s)-3];

	if( strcmp(last, "txt") == 0 ) {
		final = 1;
	}

	return final;
	
}
*/


