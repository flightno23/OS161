#include <types.h>
#include <lib.h>

#include <kern/execv.h>
#include <copyinout.h>

#define BUFFER_SIZE 255

int sys_execv(const_userptr_t progname, userptr_t args){
	
	
	/* Step 1: get the program name from the user */
	
	char progNameFromUser[BUFFER_SIZE];
	int result = 0;
	size_t actual;
	if ((result = copyinstr(progname, progNameFromUser, BUFFER_SIZE, &actual)) != 0){
		return result;
	}

	/* Step 2: get the number of arguments from userspace */
	int numArgs = 0;
	int random;
	
	
	int i = 0;
	while (*(char **)(args+i) != NULL) {
		copyin(args+i, &random, sizeof(int));
		numArgs++;
		i += 4;
	}
	
	numArgs--;	
	char * commands[numArgs];
	int pointersToGet[numArgs];
	int j = 4;
	for (int i=0; i < numArgs; i++) {
		copyin(args + (j*i), &pointersToGet[i], sizeof(int));
		copyinstr((userptr_t)pointersToGet[i] , commands[i], 100, &actual); 
	}

	
	
	 










	
	return 0;	
}
