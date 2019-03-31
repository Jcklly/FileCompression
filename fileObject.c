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
	} else if( (argv[1][1] == 'd') || (argv[2][1] == 'd') ) {
		if(argc == 3) {
			buildCB(argv[2], 0, 2);
			code = 3;
		} else if(argc == 4) {
			buildCB(argv[3], 1, 2);
			code = 3;
		} else {
			fprintf(stderr, "Invalid number of inputs.\n");
			exit(1);
		}

	} else {

			fprintf(stderr, "Invalid number of inputs.\n");
			exit(1);
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
		createTokenArray(&s_dir, type);
	} else {
		if( d != NULL ) {

			status = readdir(d);

			do {
			
				char* fullDir = concatDir(s_dir, status->d_name);	
//				printf("%s\n", fullDir);

				if( status->d_type == DT_REG ) {
					createTokenArray(&fullDir, type);
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
	
	int fd;

	if(flag == 0 || flag == 1) {
		fd = open(*file, O_RDWR, 00600);
	} else {
		char temp[strlen(*file) + 5];
		strcpy(temp, *file);
		strcat(temp, ".hcz");
		fd = open(temp, O_RDWR, 00600);
	
	}

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

	tokenArray[j] = '\0';

	if( flag == 0 ) {
		createNodeArray(tokenArray, fileSize, 0);
	} else if( flag == 1 ) {
		compress(tokenArray, fileSize, *file);
	} else {
		decompress(tokenArray, fileSize, *file);
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
		strcpy(t, a);
		strcat(t, "\t");
		strcat(t, c[0].atoken);		
		strcat(t, "\n");

			// Create and write to HuffmanCodeBook
		int fd = open("HuffmanCodeBook", O_CREAT | O_APPEND | O_RDWR, 00600);
				
		if (fd == -1) {
			printf("Error on open: %d\n", errno);
		}
		
		write(fd, t, writeSize);
		close(fd);


			// Free the token and the node.
//		free(c[0].atoken);
//		free(c);
	}

}


void compress(char* tokenArray, int length, char* file) {

	size_t fileSize;
	int o, p;
	o = p = 0;
	char buffer;	

	int fd = open("HuffmanCodeBook", O_RDWR, 00600);

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
		
/*		if( buffer == '\t' ) {
			mid = p;
//			printf("%d\n", mid);
		}
		if( buffer == '\n' ) {
			last = p;
//			printf("%d\n", last);
		}

		if(last-mid > 0) {
			if(p <= last) { printf("%d", p); }
			printf(" \t%d - %d = %d\n", last, mid, last-mid);
		}
*/
		huffmanArray[p] = buffer;
		++p;
		--o;

	} while (o > 0);
	close(fd);



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
		
				// If non-alphabetic character, allocate memory and copy token out. Stores pointer to tokens in array.
			if( ((tokenArray[j] >= 0)  && (tokenArray[j] <= 32)) || (tokenArray[j] == 127) || (tokenArray[j] == '\t') || (tokenArray[j] == ' ') || (tokenArray[j] == '\n') ) {		
				
				memcpy(token, tokenArray+i, tokenLength);
				token[tokenLength] = '\0';
				




			
				int first, mid, last;
				first = mid = last = o = p = 0;
				char buffer;	

				int fd = open("HuffmanCodeBook", O_RDWR | O_APPEND, 00600);

				if (fd == -1) {
					close(fd);
					return;
				} 
					
				o = fileSize;
		
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
//							printf("%d  %d  %d\n", first, mid, last);
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
							
//							printf("%s  %s\n", tokenBit, token2);
							
							if(strcmp(token, token2) == 0) {
//								printf("SAME: %s = %s | %s\n", token, token2, tokenBit);								

								char newFile[strlen(file) + 5];
								strcpy(newFile, file);
								strcat(newFile, ".hcz");

								int fdr = open(newFile, O_CREAT | O_APPEND | O_RDWR, 00600);
										
								if (fdr == -1) {
									return;
								}
							
//								printf("%s : %d\n", tokenBit, tbSize);	
								write(fdr, tokenBit, tbSize-1);
								close(fdr);
	
								break;
							}

						}
					}

					++p;
					--o;

				} while (o > 0);
				close(fd);

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

				
				int first, mid, last;
				first = mid = last = o = p = 0;
				char buffer;	

				int fd = open("HuffmanCodeBook", O_RDWR | O_APPEND, 00600);

				if (fd == -1) {
					close(fd);
					return;
				} 
					
				o = fileSize;
		
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
//							printf("%d  %d  %d\n", first, mid, last);
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
							
//							printf("%s  %s\n", delim, token2);
							if(strcmp(delim, token2) == 0) {
//								printf("SAME: %s = %s | %s\n", delim, token2, tokenBit);								

								char newFile[strlen(file) + 5];
								strcpy(newFile, file);
								strcat(newFile, ".hcz");

								int fdr = open(newFile, O_CREAT | O_APPEND | O_RDWR, 00600);
										
								if (fdr == -1) {
									return;
								}
							
//								printf("%s : %d\n", tokenBit, tbSize);	
								write(fdr, tokenBit, tbSize-1);
								close(fdr);
	
								break;							
							}

						}
					}

					++p;
					--o;

				} while (o > 0);
				close(fd);



			
		++i;
	}
}	


void decompress(char* tokenArray, int length, char* file) {

	char newFile[strlen(file) + 5];
	strcpy(newFile, file);
	strcat(newFile, ".hcz");



	char fileTemp[strlen(file)+5];
	strcpy(fileTemp, file);
	strcat(fileTemp, "TEMP");


	int first, mid, last, o, p, i, bitPlace, fileSize;
	o = p = i = fileSize = bitPlace =0;
	char buffer;	

	int fd = open("HuffmanCodeBook", O_RDWR | O_APPEND, 00600);

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
		huffmanArray[p] = buffer;
		++p;
		--o;

	} while (o > 0);
	close(fd);


	



	first = mid = last = o = p = 0;	

	int booli = 0;
	int test = 0;
	while(i < length) {

//		printf("%d\n", i);

		fd = open("HuffmanCodeBook", O_RDWR | O_APPEND, 00600);

		if (fd == -1) {
			close(fd);
			return;
		} 

		o = fileSize;
		mid = 0;
		first = 0;
		last = 0;
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
//					printf("%d  %d  %d\n", first, mid, last);
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

//					printf("%s : %s\n", token2, tokenBit);
				
							
					char bits[length];
					bits[test] = tokenArray[i];	
					bits[test+1] = '\0';				
					
//					printf("%d\n", test);
//					printf("%s ==== %s\n", bits, tokenBit);

					if( strcmp(bits, tokenBit) == 0) {
//						printf("%s = %s = %s\n", bits, tokenBit, token2);
						int s = 0;
						char delim;
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

						int fdr = open(fileTemp, O_CREAT | O_APPEND | O_RDWR, 00600);
								
						if (fdr == -1) {
							printf("ERROR\n");
							return;
						}
					
						if(s == 1) {
							write(fdr, &delim, 1);
						} else {
							write(fdr, token2, last-mid-1);
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
				}
			}

			++p;
			--o;

		} while (o > 0);
		close(fd);
		++i;
		if(booli == 1) {
			test = 0;
			booli = 0;
		} else {
			++test;
		}
	}


		// Created temp file to decompress. This deletes the original and replaces it with the new decompressed file.
	int remov = remove(file);
	if(remov != 0) {
		fprintf(stderr, "Error decompressing file: %s\n", file);
		exit(1);
	}

	int renam = rename(fileTemp, file);
	if(renam != 0) {
		fprintf(stderr, "Error decompressing file: %s\n", file);
		exit(1);
	}


}


