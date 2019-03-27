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

int tokenCounter = 0;

void correctCall(int argc, char* argv[]) {


	nodeArray = malloc(10*sizeof(*nodeArray));
//	nodeArray[9].frequency = -1;
	int i = 0;

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


	while(i < tokenCounter) {
		printf("%s : %d\n", nodeArray[i].atoken, nodeArray[i].frequency);
		++i;
	}
//	printf("%d\n", tokenCounter);
//	printf("%c\n", *argv[2]);
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


	// Given a file or directory, will create an array of all tokens contained within the file.
void createTokenArray(char** file, int flag) {
//	printf("%s\n", *file);	

	size_t fileSize;
	int i, j, count;
	i = j = count = 0;
	char buffer;	

	int fd = open(*file, O_RDWR | O_CREAT, 00600);
	off_t cp = lseek(fd, (size_t)0, SEEK_CUR);	
	fileSize = lseek(fd, (size_t)0, SEEK_END); 
	lseek(fd, cp, SEEK_SET);
	char tokenArray[fileSize];

	if (fd == -1) {
		printf("Error on open: %d\n", errno);
	}
		

	i = fileSize;
	do {

		read(fd, &buffer, 1);
		tokenArray[j] = buffer;
		++j;
		--i;

	} while (i > 0);
	close(fd);

	createNodeArray(tokenArray, fileSize);

}

	// Given an arry of tokens, will create the node array with all unique tokens and their respective frequency.
void createNodeArray(char* tokenArray, int length) {

	int i, j, k;
	i = j = k = 0;
	
		// Used for allocating memory to array, keeping track and expanding memory if needed.
//	static int tokenCounter = 0;
	int N = 10;


		// Token that will hold substring of main string.
	int tokenLength = 0;

		// Main tokenizer. Based on pseudo code from assignment documentation	
	while (i < length) {
		if ( (tokenArray[i] > 32) && (tokenArray[i] < 127) && (tokenArray[i] != '\t') && (tokenArray[i] != ' ') && (tokenArray[i] != '\n') ) {
			j = i;
			while ((j < length) && ( (tokenArray[j] > 32) && (tokenArray[j] < 127) && (tokenArray[j] != '\t') && (tokenArray[j] != ' ') && (tokenArray[j] != '\n') ) ) {	
				++j;	
			}

				// Size of individual tokens.
			tokenLength = (j-i);
			char token[tokenLength];
						
				// Checks if theres enough allocated space for tokens. If reaches max, expands tokens array.
			if( tokenCounter >= N-1) {
				N += 10;
				struct node* temp;
				temp = realloc(nodeArray, N*sizeof(*nodeArray));
				if (temp == NULL) {
					printf("Reallocation Error: Failed to reallocate more memory. Exiting...\n");
					exit(1);
				} else {
					nodeArray = temp;
				}
			}
			
				// If non-alphabetic character, allocate memory and copy token out. Stores pointer to tokens in array.
			if( ((tokenArray[j] >= 0)  && (tokenArray[j] <= 32)) || (tokenArray[j] == 127) || (tokenArray[j] == '\t') || (tokenArray[j] == ' ') || (tokenArray[j] == '\n') ) {		
				
				memcpy(token, tokenArray+i, tokenLength);
				(token[tokenLength]) = '\0';
	
				k = 0;	
				while(k < tokenCounter) {
		
					if(nodeArray[0].atoken != NULL && nodeArray[k].atoken != NULL) {
						if( (strcmp(nodeArray[k].atoken, token ) == 0)) {
							break;
							
						}
					}
					++k;
				}	

				if(k == tokenCounter) { 
					nodeArray[tokenCounter].atoken = malloc(tokenLength);
					if(nodeArray[tokenCounter].atoken == NULL) {
						printf("Memory Error: Failed to allocate memory. Exiting...\n");
						exit(1);
					}

					strcpy(nodeArray[tokenCounter].atoken, token);
					nodeArray[tokenCounter].frequency = 1;
					++tokenCounter;		

				} else {
					nodeArray[k].frequency += 1;
				}
			}
			i = j;
		} 
	
		char check;
		char delim[4];

		check = tokenArray[i];
		switch(check) {
		case ' ':
			strcpy(delim, "\\s");	
			break;
		case '\t':
			strcpy(delim, "\\t");
			break;
		case '\n':
			strcpy(delim, "\\n");
			break;
		case '\0':
			strcpy(delim, "\\0");
			break;
		case '\r':
			strcpy(delim, "\\r");
			break;
		case '\a':
			strcpy(delim, "\\a");
			break;
		case '\b':
			strcpy(delim, "\\b");
			break;
		case '\f':
			strcpy(delim, "\\f");
			break;
		case '\v':
			strcpy(delim, "\\v");
			break;
		}
			
			int p = 0;
			while(p < tokenCounter) {
				if(nodeArray[0].atoken != NULL && nodeArray[p].atoken != NULL) {
					if( (strcmp(nodeArray[p].atoken, delim ) == 0)) {
						break;
					}
				}
				++p;
			}

			if(p == tokenCounter) {
		
				nodeArray[tokenCounter].atoken = malloc(4);
				if(nodeArray[tokenCounter].atoken == NULL) {
					printf("Memory Error: Failed to allocate memory. Exiting...\n");
					exit(1);	
				}
				strcpy(nodeArray[tokenCounter].atoken, delim);
				nodeArray[tokenCounter].frequency = 1;
				++tokenCounter;		
			} else {
				nodeArray[p].frequency += 1;
			}

		++i;
	}	

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


