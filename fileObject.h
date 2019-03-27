#ifndef FILEOBJECT_H
#define FILEOBJECT_H
#endif


struct node {
	int frequency;
	char* atoken;
	struct node *lc;
	struct node *rc;
};

struct node *nodeArray;

	// Function Definitions
void correctCall(int, char* a[]);
int buildCB(char*, int);
char* concatDir(char*, char*);
void createTokenArray(char**, int);
void createNodeArray(char*, int);
//int isTXT(char*);
