#include <stdlib.h>
#include <stdio.h>
#include "fileObject.h"


int main(int argc, char* argv[]) {
	
	if(argc > 5) {
		fprintf(stderr, "Invalid Input. Exceeded Maximum Number of Arguments.\n");
	}


		// Send to another function that will determine correct functions to call for given input
	correctCall(argc, argv);

return 0;	
		
}
