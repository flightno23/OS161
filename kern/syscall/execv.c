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
		commands[i] = kmalloc(100*sizeof(char));
		copyin(args + (j*i), &pointersToGet[i], sizeof(int));
		copyinstr((userptr_t)pointersToGet[i] , commands[i], 100, &actual); 
	}

	void * startPoint;
	int numBytes = 0;
	int len;
	int padding = 0;
	/* Creating (numArgs+1) contigous block of pointers of 4 bytes each */
	char nullPadding[3] = "\0\0\0";
	startPoint = kmalloc(sizeof(int *) * (numArgs+1));
	numBytes += (sizeof(int *) * (numArgs+1));
	startPoint += numBytes;
	int intSize = 4;

	for (i=0;i<numArgs; i++){
		*(int *)((startPoint-numBytes) + (i*intSize))  = numBytes;
		len = strlen(commands[i]) +1;
		memcpy(startPoint,commands[i],len);
		numBytes += len;
		startPoint += len;
		padding = 4 - ((int)startPoint % 4);
		if (padding != 4) {
			memcpy(startPoint, nullPadding, padding);
			startPoint += padding;
			numBytes += padding;
		}  		
	}
	
	return 0; 
}
