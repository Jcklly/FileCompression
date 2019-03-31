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

	// Function Definitions
void correctCall(int, char* a[]);
int buildCB(char*, int, int);
char* concatDir(char*, char*);
void createTokenArray(char**, int);
void createNodeArray(char*, int, int);
int sortHeapify(int, int);
int sort(int);
void iSort(int, int, int);
void huffman(int);
void traverseH(struct node*, char a[], int);
void compress(char*, int, char*);
void decompress(char*, int, char*);



