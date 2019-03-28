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

/* TO-DO:
 * Traverse huffman tree and write out to codeBook
*/



int tokenCounter = 0;
void correctCall(int argc, char* argv[]) {


	nodeArray = malloc(10*sizeof(*nodeArray));
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

	
	iSort(1, tokenCounter, 1);


	huffman(tokenCounter);

		// Prints nodeArray
	while(i < tokenCounter) {
//		printf("%s : %d\n", nodeArray[i].atoken, nodeArray[i].frequency);
		++i;
	}


/*
	struct node temp;
	temp = *(nodeArray)[0].rc;
	printf("%s : %d\n", temp.atoken, temp.frequency);

	struct node temp2;
	temp2 = *temp.rc;
	printf("%s : %d\n", temp2.atoken, temp2.frequency);

	struct node temp3;
	temp3 = *temp2.lc;
	printf("%s : %d\n", temp3.atoken, temp3.frequency);

	struct node temp4;
	temp4 = *temp3.rc;
	printf("%s : %d\n", temp4.atoken, temp4.frequency);
*/


	char a[50];
//	traverseH(nodeArray[0], a, 0);
	

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



void huffman(int length) {
	int size = length;
	int i = 0;
	while( (tokenCounter != 1) ) {
		struct node* temp = malloc(sizeof(*temp));
		temp->atoken = malloc(sizeof(11));
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



//		int g = 0;
//		while(g < tokenCounter) {
//			printf("%s : %d\n", nodeArray[g].atoken, nodeArray[g].frequency);
//			++g;
//		}
//		printf("\n");

//		--tokenCounter;
//		i += 2;
//		size -= 2;	
//		printf("%d\n", size);
	}


	nodeArray[0].atoken = malloc(9);
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


void traverseH(struct node c, char* a, int step) {

//	int fd = open("HuffmanCodeBook", O_CREAT | O_RDWR, 00600);

		
	if(c.lc != NULL) {
		a[step] = '0';
		traverseH(*c.lc, a, step+1);
	}	

	if(c.rc != NULL) {
		a[step] = '1';
		traverseH(*c.rc, a, step+1);
	}

	if(c.lc == NULL && c.rc == NULL) {
		printf("%s : %s\n", c.atoken, a);
	}



//	struct node temp;
//	temp = *c.lc;
//	printf("%s : %d\n", temp.atoken, temp.frequency);


//	close(fd);
}










