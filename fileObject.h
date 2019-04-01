#ifndef FILEOBJECT_H
#define FILEOBJECT_H
#endif


struct node {
	int frequency;
	char* atoken;
	struct node* lc;
	struct node* rc;
};

struct node *nodeArray;

struct codeBook {
	char* atoken;
	char* bitSeq;
};





	// Function Definitions
void correctCall(int, char** a);
int buildCB(char*, int, int, char*);
char* concatDir(char*, char*);
void createTokenArray(char**, int, char*);
void createNodeArray(char*, int, int);
void iSort(int, int, int);
void huffman(int);
void traverseH(struct node*, char a[], int);
void compress(char*, int, char*, char*);
void decompress(char*, int, char*, char*);
int contains(char*);
void deleteHCZ(char*, int);
void arguments(char**, int);
