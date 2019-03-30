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

	
	nodeArray = calloc(10, sizeof(*nodeArray));
	int i, code;
	i = code = 0;

	if( (argv[1][1]  == 'b') || (argv[2][1] == 'b') ) {	
		if(argc == 3) {
				// 0 means there is no -R flag
			buildCB(argv[2], 0, 0);
			code = 1;
		} else if(argc == 4) {
				// 1 means there is an -R flag
			buildCB(argv[3], 1, 0);
			code = 1;
		} else {
			fprintf(stderr, "Invalid number of inputs.\n");
			exit(1);
		}
	} else if( (argv[1][1] == 'c') || (argv[2][1] == 'c') ) {
		if(argc == 3) {
			buildCB(argv[2], 0, 1);
			code = 2;
		} else if(argc == 4) {
			buildCB(argv[3], 1, 1);
			code = 2;
		} else {
			fprintf(stderr, "Invalid number of inputs.\n");
			exit(1);
		}
	} else {
		printf("Different\n");
	}

	

	if(code == 1) {
		iSort(1, tokenCounter, 1);
	 	huffman(tokenCounter);

		char a[50];
		traverseH(&nodeArray[0], a, 0);
	

		int fd = open("HuffmanCodeBook", O_CREAT | O_APPEND | O_RDWR, 00600);
				
		if (fd == -1) {
			printf("Error on open: %d\n", errno);
		}
		
		write(fd, "\n", 1);
		close(fd);
	}


}



	// flag: 0 = filename, 1 = directory
	// type: 0 = buildCB, 1 = compress, 2 = decompress
int buildCB(char* s_dir, int flag, int type) {


	DIR* d = opendir(s_dir);
	char* newDir;	
	struct dirent* status = NULL;

		// Checks if -R flag is in input.
	if( flag == 0 ) {
		if(type == 0) {
			createTokenArray(&s_dir, flag);
		} else if(type == 1) {

			compress(&s_dir, s_dir); // send file to compress
		} else {
			; // send to file decompress
		}
	} else {
		if( d != NULL ) {

			status = readdir(d);

			do {
			
				char* fullDir = concatDir(s_dir, status->d_name);	
//				printf("%s\n", fullDir);

				if( status->d_type == DT_REG ) {
					if(type == 0) {
						createTokenArray(&fullDir, flag);
					} else if(type == 1) {
						compress(&fullDir, status->d_name); // send to file compress
					} else {
						; // send to file decompress
					}
				} else {
					;
				}

					// Recurssive method for going through subdirectories	
				if( status->d_type == DT_DIR ) {
					if( (strcmp(status->d_name, ".") == 0) || (strcmp(status->d_name, "..") == 0) ) {
						;
					} else {
						newDir = concatDir(s_dir, status->d_name);
						buildCB(newDir, flag, type);
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

	if (fd == -1) {
		close(fd);
		return;
	} 
		
	off_t cp = lseek(fd, (size_t)0, SEEK_CUR);	
	fileSize = lseek(fd, (size_t)0, SEEK_END); 
	lseek(fd, cp, SEEK_SET);
	char tokenArray[fileSize];

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
			char token[tokenLength + 1];
		
				// Checks if theres enough allocated space for tokens. If reaches max, expands tokens array.
			if( tokenCounter >= N-1) {
				N = tokenCounter + 10;
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
				token[tokenLength] = '\0';
					
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
					nodeArray[tokenCounter].atoken = malloc(tokenLength + 1);
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
			strcpy(delim, " ");	
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

				// Checks if theres enough allocated space for tokens. If reaches max, expands tokens array.
			if( tokenCounter >= N-1) {
				N = tokenCounter + 10;
				struct node* temp;
				temp = realloc(nodeArray, N*sizeof(*nodeArray));
				if (temp == NULL) {
					printf("Reallocation Error: Failed to reallocate more memory. Exiting...\n");
					exit(1);
				} else {
					nodeArray = temp;
				}
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
				nodeArray[tokenCounter].atoken = (char*)malloc(4);
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




void huffman(int length) {
	int size = length;
	int i = 0;
	while( (tokenCounter != 1) ) {
		struct node* temp = malloc(sizeof(*temp));
		temp->atoken = malloc(11*sizeof(char));
		strcpy(temp->atoken, "_NULLNODE_");
		if(temp == NULL) {
			printf("oh no\n");
			exit(0);
		}
		temp->lc = malloc(sizeof(*temp));
		if(temp->lc == NULL) {
			printf("oh no\n");
			exit(0);
		}
		*temp->lc = nodeArray[i];
		if(i+1 <= tokenCounter) {
			temp->rc = malloc(sizeof(*temp));
			if(temp->rc == NULL) {
				printf("oh no\n");
				exit(0);
			}
			*temp->rc = nodeArray[i+1];
			temp->frequency = nodeArray[i].frequency + nodeArray[i+1].frequency;
		} else {
			temp->frequency = nodeArray[i].frequency;
		}
	
//		printf("%d\n", temp->frequency);

		int j = tokenCounter;
		int f = 2;
		while( f < j ) {

			nodeArray[f-2] = nodeArray[f];			
			++f;		
		}


		--tokenCounter;
		nodeArray[tokenCounter-1] = *temp;
		iSort(1, tokenCounter, 2);
	}

	free(nodeArray[0].atoken);
	nodeArray[0].atoken = malloc(9*sizeof(char));
	strcpy(nodeArray[0].atoken, "_ROOTNODE_");

}



	// Insertion Sort
void iSort(int start, int length, int flag) {

	int i, j;
	struct node k;

	i = start;
	while(i < length) {
		k = nodeArray[i];
		j = i-1;

		while( (j >= 0) && (nodeArray[j].frequency > k.frequency)) {
			nodeArray[j+1] = nodeArray[j];
			--j;	
		}
		nodeArray[j + 1] = k;
		++i;
	}

}


void traverseH(struct node* c, char a[], int step) {



	if(c[0].lc != NULL) {
		a[step] = '0';
		traverseH(c[0].lc, a, step+1);
	}	

	if(c[0].rc != NULL) {
		a[step] = '1';
		traverseH(c[0].rc, a, step+1);
	}

	if(c[0].lc == NULL && c[0].rc == NULL) {
		a[step] = '\0';
//		printf("%s : %s: ", c[0].atoken, a);

//		printf("%d\n", 2 + step + strlen(c[0].atoken));
		int writeSize = 2 + step + strlen(c[0].atoken);

		char t[writeSize];
		strcpy(t, c[0].atoken);
		strcat(t, "\t");
		strcat(t, a);		
		strcat(t, "\n");

			// Create and write to HuffmanCodeBook
		int fd = open("HuffmanCodeBook", O_CREAT | O_APPEND | O_RDWR, 00600);
				
		if (fd == -1) {
			printf("Error on open: %d\n", errno);
		}
		
		write(fd, t, writeSize);
		close(fd);


			// Free the token and the node.
		free(c[0].atoken);
		free(c);
	}

}


void compress(char** file, char* name) {
	

	char cFile[strlen(*file) + 5];
	strcpy(cFile, *file);
	strcat(cFile, ".hcz");

	
	

	size_t fileSize;
	int i, j, count;
	i = j = count = 0;
	char buffer;	

	int fd = open(*file, O_RDWR | O_APPEND, 00600);

	if (fd == -1) {
		close(fd);
		return;
	} 
		
	off_t cp = lseek(fd, (size_t)0, SEEK_CUR);	
	fileSize = lseek(fd, (size_t)0, SEEK_END); 
	lseek(fd, cp, SEEK_SET);
	char tokenArray[fileSize];

	i = fileSize;
	do {

		read(fd, &buffer, 1);
		tokenArray[j] = buffer;
		++j;
		--i;

	} while (i > 0);
	
	close(fd);
	
	tokenArray[fileSize] = '\0';
	printf("%s\n", tokenArray);


//	int fd = open(cFile, O_RDWR | O_CREAT | O_APPEND, 00600);
//	close(fd);
}





























	// Heapify function for heapsort algorithm
int sortHeapify(int size, int mid) {
	
		// Position for root, left and right child.
	int root = mid;
	int rightChild = 2*mid+2;
	int leftChild = 2*mid+1;
	
		// Check whether left or right child is larger than root	
	if( (size > leftChild) && (nodeArray[root].frequency > nodeArray[leftChild].frequency) ) {
		root = leftChild; 
	}
	if( (size > rightChild) && (nodeArray[root].frequency <  nodeArray[rightChild].frequency) ) {
		root = rightChild;
	}
	
		// if root changed, swap root with current value
	if(root != mid) {
		struct node temp3 = nodeArray[root];
		nodeArray[root] = nodeArray[mid];
		nodeArray[mid] = temp3;
		sortHeapify(size, root);
	}
	
	return 0;
}

	// Sorting Function. Given an address to a char** array and array length, applied heapsort algorithm to array!
int sort(int arrayLength) {
	
		// Check to see if there is only 1 element in the array.
	if( arrayLength == 1 ) {
		return 0;
	}
		// Declare variables
	
	int size = arrayLength;
	
		// Build the heap
	int mid = ((arrayLength/2) - 1);
	while(mid >= 0) {
		sortHeapify(size, mid);
		--mid;
	}

		// Heapify
	mid = arrayLength - 1;
	while(mid >= 0) {
		struct node temp2= nodeArray[0];
		nodeArray[0] = nodeArray[mid];
		nodeArray[mid] = temp2;
		sortHeapify(mid, 0);
		--mid;
	}

	return 0;
}