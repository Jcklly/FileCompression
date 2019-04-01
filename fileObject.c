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

	// Keeps track of number of tokens when building the HuffmanCodeBook.
int tokenCounter = 0;


	// Splits up arguments and sends to appropriate functions with correct arguments. 
void correctCall(int argc, char** argv) {


		// Goes over arguments, exits program if anything is invalid.
	arguments(argv, argc);

	int i, code;
	i = code = 0;

		// Struct that holds everything for building the codebook.
	nodeArray = calloc(10, sizeof(*nodeArray));


	if( (argv[1][1]  == 'b') || (argv[2][1] == 'b') ) {	

			// Checks if there is already a codebook. If there is, remove it.
		int acc = access("HuffmanCodeBook", F_OK);
		if(acc == 0) {
				// If a HuffmanCodeBook already exists, delete it.
			int remov = remove("HuffmanCodeBook");
			if(remov != 0) {
				printf("Error removing HuffmanCodeBook\n");
				exit(1);
			}
		} 
		if(argc == 3) {
				// 0 means there is no -R flag
			buildCB(argv[2], 0, 0, argv[2]);
			code = 1;
		} else if(argc == 4) {
				// 1 means there is an -R flag
			buildCB(argv[3], 1, 0, argv[3]);
			code = 1;
		} else {
			printf("Invalid number of inputs.\n");
			exit(1);
		}


	} else if( (argv[1][1] == 'c') || (argv[2][1] == 'c') ) {
		printf("Compressing...Please wait.\n");
		if(argc == 4) {
			deleteHCZ(argv[2], 0);
			buildCB(argv[2], 0, 1, argv[3]);
			code = 2;

		} else if(argc == 5) {
			deleteHCZ(argv[3], 1);
			buildCB(argv[3], 1, 1, argv[4]);
			code = 2;
		} else {
			printf("Invalid number of inputs.\n");
			exit(1);
		}
	} else if( (argv[1][1] == 'd') || (argv[2][1] == 'd') ) {
		printf("Decompressing...Please wait.\n");
		if(argc == 4) {
			buildCB(argv[2], 0, 2, argv[3]);
			code = 3;
		} else if(argc == 5) {
			buildCB(argv[3], 1, 2, argv[4]);
			code = 3;
		} else {
			printf("Invalid number of inputs.\n");
			exit(1);
		}

	} else {
			fprintf(stderr, "Invalid number of inputs.\n");
			exit(1);
	}

	
		// if -b was called, sorts the array of structs, creates a huffman tree, stores in HuffmanCodeBook, then adds \n at the end.
	if(code == 1) {
		iSort(1, tokenCounter, 1);
	 	huffman(tokenCounter);

		char a[50];
		traverseH(&nodeArray[0], a, 0);
	
		int fd = open("HuffmanCodeBook", O_CREAT | O_APPEND | O_RDWR, 0644);
				
		if (fd == -1) {
			printf("Error on open: %d\n", errno);
			return;
		}
		
		write(fd, "\n", 1);
		close(fd);
	}

}


	// flag: 0 = filename, 1 = directory
	// type: 0 = buildCB, 1 = compress, 2 = decompress
	// 
	// This function will give all the other functions the file given in the arguments, or recursively throughout a directory.
int buildCB(char* s_dir, int flag, int type, char* codeBook) {

	
	DIR* d = opendir(s_dir);
	

	char* newDir;	
	struct dirent* status = NULL;

		// Checks if -R flag is in input.
	if( flag == 0 ) {
		createTokenArray(&s_dir, type, codeBook);
	} else {
		if( d != NULL ) {

			status = readdir(d);

			do {
			
				char* fullDir = concatDir(s_dir, status->d_name);	

				if( status->d_type == DT_REG ) {
					createTokenArray(&fullDir, type, codeBook);
				} else {
					;
				}

					// Recurssive method for going through subdirectories	
				if( status->d_type == DT_DIR ) {
					if( (strcmp(status->d_name, ".") == 0) || (strcmp(status->d_name, "..") == 0) ) {
						;
					} else {
						newDir = concatDir(s_dir, status->d_name);
						buildCB(newDir, flag, type, codeBook);
						free(newDir);
					}
				}
				free(fullDir);				
				status = readdir(d);
			} while ( status != NULL );
		closedir(d);
		} else if(ENOENT == errno) {
			printf("Invalid directory Received. Please provide a valid directory.\n Error: ENOENT\n");
			exit(1);
		} 	
	}
	return 0;
}


	// Given a file or directory, will create an array of all tokens contained within the file.
void createTokenArray(char** file, int flag, char* codeBook) {

	size_t fileSize;
	int i, j, count;
	i = j = count = 0;
	char buffer;	
	
	int fd;

	if( flag == 0  ) {
		fd = open(*file, O_RDWR, 0644);
	} else if (flag == 1)  {
		fd = open(*file, O_RDWR, 0644);
	} else {	
		int contain = 0;
		contain = contains(*file);

		if(contain != 1) {	
			return;
		} 	

		fd = open(*file, O_RDWR, 0644);
	
	}

	if (fd == -1) {
		printf("Invalid Open of file: %s\n", *file);
		exit(1);
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

	tokenArray[j] = '\0';

	if( flag == 0 ) {
		createNodeArray(tokenArray, fileSize, 0);
	} else if( flag == 1 ) {
		compress(tokenArray, fileSize, *file, codeBook);
	} else {
		decompress(tokenArray, fileSize, *file, codeBook);
	}
	
}

	// Given an arry of tokens, will create the node array with all unique tokens and their respective frequency.
	// flag = 0 -> createNode array
	// flag = 1 -> simply return token
void createNodeArray(char* tokenArray, int length, int flag) {

	int i, j, k;
	i = j = k = 0;
	
		// Used for allocating memory to array, keeping track and expanding memory if needed.
	int N = 10;

		// Token that will hold substring of main string.
	int tokenLength = 0;

		// Main tokenizer	
	while (i < length) {
		if ( (tokenArray[i] > 32) && (tokenArray[i] < 127) && (tokenArray[i] != '\t') && (tokenArray[i] != ' ') && (tokenArray[i] != '\n') ) {
			j = i;
			while ((j < length) && ( (tokenArray[j] > 32) && (tokenArray[j] < 127) && (tokenArray[j] != '\t') && (tokenArray[j] != ' ') && (tokenArray[j] != '\n') ) ) {	
				++j;	
			}

				// Size of individual tokens.
			tokenLength = (j-i);
			char token[tokenLength + 1];
		
			if( flag == 0 ) {

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
		char delim[5];

		check = tokenArray[i];
		switch(check) {
		case ' ':
			strcpy(delim, "  ");	
			break;
		case '\t':
			strcpy(delim, "\\t ");
			break;
		case '\n':
			strcpy(delim, "\\n ");
			break;
		case '\0':
			strcpy(delim, "\\0 ");
			break;
		case '\r':
			strcpy(delim, "\\r ");
			break;
		case '\a':
			strcpy(delim, "\\a ");
			break;
		case '\b':
			strcpy(delim, "\\b ");
			break;
		case '\f':
			strcpy(delim, "\\f ");
			break;
		case '\v':
			strcpy(delim, "\\v ");
			break;
		}

			if( flag == 1 ) {
				printf("%s\n", delim);
			} else {

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
				nodeArray[tokenCounter].atoken = (char*)malloc(5);
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



	// Creates the huffman tree out of the array of structs.
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

		// Free what was once there, then change it.
	free(nodeArray[0].atoken);
	nodeArray[0].atoken = malloc(9*sizeof(char));
	strcpy(nodeArray[0].atoken, "_ROOTNODE_");

}



	// Insertion Sort algorithm for huffman tree
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

	// Traverses all nodes in huffman tree. If left and right childs are NULL (its a leaf) write its value and bit sequence to HuffmanCodeBook file, then free the node.
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
		int writeSize = 2 + step + strlen(c[0].atoken);

		char t[writeSize];
		strcpy(t, a);
		strcat(t, "\t");
		strcat(t, c[0].atoken);		
		strcat(t, "\n");

			// Create and write to HuffmanCodeBook
		int fd = open("HuffmanCodeBook", O_CREAT | O_APPEND | O_RDWR, 0644);
				
		if (fd == -1) {
			printf("Error on open: %d\n", errno);
		}
		
		write(fd, t, writeSize);
		close(fd);

	}

}


	// Compress function. Works in O(n^2) runtime.
void compress(char* tokenArray, int length, char* file, char* codeBook) {

	size_t fileSize;
	int o, p, first, mid, last, numLines, counter;
	o = p = first = mid = last = numLines = counter = 0;
	char buffer;	

	int fd = open(codeBook, O_RDWR, 0644);

	if (fd == -1) {
		close(fd);
		return;
	} 

	off_t cp = lseek(fd, (size_t)0, SEEK_CUR);	
	fileSize = lseek(fd, (size_t)0, SEEK_END); 
	lseek(fd, cp, SEEK_SET);
	char huffmanArray[fileSize];

	o = fileSize;
	do {

		read(fd, &buffer, 1);
		if(buffer == '\n') {
			++numLines;
		}
		huffmanArray[p] = buffer;
		++p;
		--o;

	} while (o > 0);
	close(fd);


	struct codeBook book[numLines];

	fd = open(codeBook, O_RDWR, 0644);

	if (fd == -1) {
		close(fd);
		return;
	} 
		

	o = fileSize;
	p = 0;
	do {

		read(fd, &buffer, 1);

		if( buffer == '\t' ) {
			mid = p;
		}
		if( buffer == '\n' ) {
			last = p;
		}


		if(last-mid > 0) {
			if(p <= last) {
				char token2[last-mid];
				int tbSize = mid-first+1;
				char tokenBit[tbSize];
				memcpy(tokenBit, huffmanArray+first, mid-first);
				tokenBit[mid-first] = '\0';

				if(p+1 < fileSize-1) {
					first = p+1;
				}
			
				memcpy(token2, huffmanArray+mid+1, last-mid-1);
				token2[last-mid-1] = '\0';

				book[counter].atoken = malloc((last-mid)*sizeof(char));
				strcpy(book[counter].atoken, token2);

				book[counter].bitSeq = malloc((tbSize)*sizeof(char));
				strcpy(book[counter].bitSeq, tokenBit);
				++counter;

				
			}		
		}


		--o;
		++p;

	} while (o > 1);
	close(fd);


	int i, j, k, count;
	i = j = k = count = 0;
	
		// Used for allocating memory to array, keeping track and expanding memory if needed.
	int N = 10;

		// Token that will hold substring of main string.
	int tokenLength = 0;

		// Main tokenizer	
	while (i < length) {
		if ( (tokenArray[i] > 32) && (tokenArray[i] < 127) && (tokenArray[i] != '\t') && (tokenArray[i] != ' ') && (tokenArray[i] != '\n') ) {
			j = i;
			while ((j < length) && ( (tokenArray[j] > 32) && (tokenArray[j] < 127) && (tokenArray[j] != '\t') && (tokenArray[j] != ' ') && (tokenArray[j] != '\n') ) ) {	
				++j;	
			}

				// Size of individual tokens.
			tokenLength = (j-i);
			char token[tokenLength + 1];
		
				// If non-alphabetic character, allocate memory and copy token out. Stores pointer to tokens in array.
			if( ((tokenArray[j] >= 0)  && (tokenArray[j] <= 32)) || (tokenArray[j] == 127) || (tokenArray[j] == '\t') || (tokenArray[j] == ' ') || (tokenArray[j] == '\n') ) {		
				
				memcpy(token, tokenArray+i, tokenLength);
				token[tokenLength] = '\0';
				

	
				count = 0;
				while(count < numLines-1) {
							
					if(book[count].atoken == NULL || book[count].bitSeq == NULL) {
						break;
					}
				
					if(strcmp(token, book[count].atoken) == 0) {

						char newFile[strlen(file) + 5];
						strcpy(newFile, file);
						strcat(newFile, ".hcz");

						int fdr = open(newFile, O_CREAT | O_APPEND | O_RDWR, 0644);
								
						if (fdr == -1) {
							printf("Invalid Open: %s\n", newFile);
							return;
						}
					
						write(fdr, book[count].bitSeq, strlen(book[count].bitSeq));
						close(fdr);

						break;

					}
						
					if(count == numLines-2) {

						char newFile[strlen(file) + 5];
						strcpy(newFile, file);
						strcat(newFile, ".hcz");

						int remov = remove(newFile);
						return;

					}
					++count;
				}
			}
			i = j;
		} 
	
		char check;
		char delim[5];

		check = tokenArray[i];
		switch(check) {
		case ' ':
			strcpy(delim, "  ");	
			break;
		case '\t':
			strcpy(delim, "\\t ");
			break;
		case '\n':
			strcpy(delim, "\\n ");
			break;
		case '\0':
			strcpy(delim, "\\0 ");
			break;
		case '\r':
			strcpy(delim, "\\r ");
			break;
		case '\a':
			strcpy(delim, "\\a ");
			break;
		case '\b':
			strcpy(delim, "\\b ");
			break;
		case '\f':
			strcpy(delim, "\\f ");
			break;
		case '\v':
			strcpy(delim, "\\v ");
			break;
		}


	
		count = 0;
		while(count < numLines-1) {
			
			if(book[count].atoken == NULL || book[count].bitSeq == NULL) {
				break;
			}
		
			if(strcmp(delim, book[count].atoken) == 0) {

				char newFile[strlen(file) + 5];
				strcpy(newFile, file);
				strcat(newFile, ".hcz");

				int fdr = open(newFile, O_CREAT | O_APPEND | O_RDWR, 0644);
						
				if (fdr == -1) {
					return;
				}
			
				write(fdr, book[count].bitSeq, strlen(book[count].bitSeq));
				close(fdr);

				break;

			}
				++count;
		}
			
		++i;
	}

	int freeB = 0;
	while(freeB < numLines-1) {
		if(book[freeB].atoken == NULL || book[freeB].bitSeq == NULL) {
			break;
		} else {
		free(book[freeB].atoken);
		free(book[freeB].bitSeq);
		++freeB;
		}
	}

}	

	// Decompress function. Very similar to compress function, O(n^2) run time.
void decompress(char* tokenArray, int length, char* file, char* codeBook) {

	char fileTemp[strlen(file)+5];
	strcpy(fileTemp, file);
	strcat(fileTemp, "TEMP");

	size_t fileSize;
	int i, o, p, first, mid, last, numLines, counter, bitPlace;
	o = p = first = mid = last = numLines = counter = bitPlace = 0;
	char buffer;	

	int fd = open(codeBook, O_RDWR, 0644);

	if (fd == -1) {	
		printf("ERROR\n");
		close(fd);
		return;
	} 

	off_t cp = lseek(fd, (size_t)0, SEEK_CUR);	
	fileSize = lseek(fd, (size_t)0, SEEK_END); 
	lseek(fd, cp, SEEK_SET);
	char huffmanArray[fileSize];

	o = fileSize;
	do {

		read(fd, &buffer, 1);
		if(buffer == '\n') {
			++numLines;
		}
		huffmanArray[p] = buffer;
		++p;
		--o;

	} while (o > 0);
	close(fd);

		// Creates codeBook struct with all tokens and bits to check against.
	struct codeBook book[numLines];

	fd = open(codeBook, O_RDWR, 0644);

	if (fd == -1) {
		printf("ERROR\n");
		close(fd);
		return;
	} 
		

	o = fileSize;
	p = 0;
	do {

		read(fd, &buffer, 1);

		if( buffer == '\t' ) {
			mid = p;
		}
		if( buffer == '\n' ) {
			last = p;
		}


		if(last-mid > 0) {
			if(p <= last) {
				char token2[last-mid];
				int tbSize = mid-first+1;
				char tokenBit[tbSize];
				memcpy(tokenBit, huffmanArray+first, mid-first);
				tokenBit[mid-first] = '\0';

				if(p+1 < fileSize-1) {
					first = p+1;
				}
			
				memcpy(token2, huffmanArray+mid+1, last-mid-1);
				token2[last-mid-1] = '\0';



				book[counter].atoken = malloc((last-mid)*sizeof(char));
				strcpy(book[counter].atoken, token2);

				book[counter].bitSeq = malloc((tbSize)*sizeof(char));
				strcpy(book[counter].bitSeq, tokenBit);
				++counter;		
			}		
		}


		--o;
		++p;

	} while (o > 1);
	close(fd);


	first = mid = last = o = i = p = 0;	

	int booli = 0;
	int test = 0;
	int count = 0;

		// Main looping function
	while(i < length) {

		char bits[length];
		bits[test] = tokenArray[i];	
		bits[test+1] = '\0';				

		count = 0;
		while(count < numLines-1) {
			if(book[count].atoken == NULL || book[count].bitSeq == NULL) {
				break;
			}

			if( strcmp(bits, book[count].bitSeq) == 0) {
				int s = 0;
				char delim;
				
				char token2[strlen(book[count].atoken)];
				strcpy(token2, book[count].atoken);

				if(token2[strlen(token2)-1] == ' ') {
					
					char ch = token2[strlen(token2)-2];

					switch(ch) {
					case ' ':
						delim = ' ';	
						break;
					case 't':
						delim = '\t';
						break;
					case 'n':
						delim = '\n';
						break;
					case '0':
						delim = '\0';
						break;
					case 'r':
						delim = '\r';
						break;
					case 'a':
						delim = '\a';
						break;
					case 'b':
						delim = '\b';
						break;
					case 'f':
						delim = '\f';
						break;
					case 'v':
						delim = '\v';
						break;
					}

					s = 1;

				}

				int fdr = open(fileTemp, O_CREAT | O_APPEND | O_RDWR, 0644);
						
				if (fdr == -1) {
					printf("ERROR\n");
					return;
				}
			
				if(s == 1) {
					write(fdr, &delim, 1);
				} else {
					write(fdr, token2, strlen(token2));
				}
				close(fdr);

				bitPlace += test;
				int clear = 0;
				while(clear < length) {
					bits[clear] = '\0';
					++clear;
				}
				booli = 1;
				break;	
			
			}
			++count;
		}

		++i;
		if(booli == 1) {
			test = 0;
			booli = 0;
		} else {
			++test;
		}
	}


	char removeTemp[strlen(file)-3];
	strncpy(removeTemp, file, strlen(file)-4);
	removeTemp[strlen(file)-4] = '\0';
		// Created temp file to decompress. This deletes the original and replaces it with the new decompressed file.
	
	int remov = remove(removeTemp);

	int renam = rename(fileTemp, removeTemp);
	if(renam != 0) {
		printf("Error decompressing file: %s\n", file);
		exit(1);
	}

	int freeB = 0;
	while(freeB < numLines-1) {
		if(book[freeB].atoken == NULL || book[freeB].bitSeq == NULL) {
			break;
		}
		free(book[freeB].atoken);
		free(book[freeB].bitSeq);
		++freeB;
	}

}


	// Checks if given file is already .hcz file. Returns 1 if yes, 0 if no.
int contains(char* file) {
	int check = 0;

	if(strlen(file) >= 5) {
	
		char* temp = file+strlen(file)-3;		
		
		if(strcmp(temp, "hcz") == 0) {
			check = 1;
		}
		
	}
	
	return check;

}

	// Deletes a given hcz file.
void deleteHCZ(char* s_dir, int flag) {


	DIR* d = opendir(s_dir);
	char* newDir;	
	struct dirent* status = NULL;

		// Checks if -R flag is in input.
	if( flag == 0 ) {
		int containsTest = 0;
		containsTest = contains(s_dir);
		
		int acc = access(s_dir, F_OK);
		if( (containsTest == 1) && (acc == 0) ) {
			int remov = remove(s_dir);
			if(remov != 0) {
				printf("Error removing pre-existing file in compress: %s\n", s_dir);
				exit(1);
			}
		}
	} else {
		if( d != NULL ) {

			status = readdir(d);

			do {
			
				char* fullDir = concatDir(s_dir, status->d_name);	

				int containsTest = 0;	
				containsTest = contains(fullDir);

				if( (containsTest == 1) ) {
					int remov = remove(fullDir);
					if(remov != 0) {
						fprintf(stderr, "Error removing pre-existing file in compress: %s\n", fullDir);
						exit(1);
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
						deleteHCZ(newDir, flag);
						free(newDir);
					}
				}
				free(fullDir);				
				status = readdir(d);
			} while ( status != NULL );
		}	
		closedir(d);
	}
}


	// Used to ensure arguemnts are valid before passing into functions.
void arguments(char** arg, int c) {

	if(arg[1][1] == 'b' || arg[2][1] == 'b') {
	
		if( c > 4 ) {
			printf("Invalid number of arguments for building codebook.\nExtected 2 or 3 arguments.\nReceived: %d\n", c);
			exit(0);
		} else {
		
			if(c == 4) {
				if(arg[1][1] == 'b' && arg[2][1] == 'R') {
					return;
				} else if(arg[1][1] == 'R' && arg[2][1] == 'b') {
					return;
				} else {
					printf("Invalid number of arguments for building codebook.\nExtected flags -R -b or -b -R.\n");
					exit(1);
				}
			}
			
		} 
		
	} else if( arg[1][1] == 'c' || arg[2][1] == 'c' ) {
		
		if(c > 5) {
			printf("Invalid number of arguments for compressing.\nExtected 2 or 3 arguments.\nReceived: %d\n", c);
			exit(1);
		} else {
		
			if(c == 5) {
				if(arg[1][1] == 'c' && arg[2][1] == 'R') {
					return;
				} else if(arg[1][1] == 'R' && arg[2][1] == 'c') {
					return;
				} else {
					printf("Invalid number of arguments for compressing.\nExtected flags -R -c or -c -R.\n");
					exit(1);
				}
			}
			if(c <= 3) {
				printf("Invalid number of arguments for compressing.\n");
				exit(1);
			}
		

		}


	} else if( arg[1][1] == 'd' || arg[2][1] == 'd' ) {

		if(c > 5) {
			printf("Invalid number of arguments for decompressing.\nExtected 2 or 5 arguments.\nReceived: %d\n", c);
			exit(1);
		} else {
		
			if(c == 5) {
				if(arg[1][1] == 'd' && arg[2][1] == 'R') {
					return;
				} else if(arg[1][1] == 'R' && arg[2][1] == 'd') {
					return;
				} else {
					printf("Invalid number of arguments for decompressing.\nExtected flags -R -d or -d -R.\n");
					exit(1);
				}
			} else if(c <= 3) {
				
				printf("Invalid number of arguments for decompressing.\n");
				exit(1);
			}
		

		}
	} else {
		printf("Invalid argument. Arguments must be either 'b' 'c' 'd' 'R'.\n");
		exit(1);
	}


}


