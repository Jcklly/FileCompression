#include <stdlib.h>
#include <stdio.h>
#include "fileObject.h"


int main(int argc, char** argv) {
	
	if(argc > 5 || argc <= 2) {
		fprintf(stderr, "Invalid Input. Incorrect number of arguments.\nRecieved: %d\nExpected: 2-5 arguments.\n", argc-1);
		exit(1);
	}

		// Send to another function that will determine correct functions to call to build the codebook.
	correctCall(argc, argv);



return 0;	
		
}
