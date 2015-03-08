#include <kern/execv.h>
#include <kern/copyinout.h>

int sys_execv(const_userptr_t progname, userptr_t args){
	
	int i, result;
	int numArgs = 0;
	void *dest;
	char progNameFromUser[BUF_SIZE];
 	
	/*Check the validity of arguments*/

	/* Copying the program name */
	size_t actual;
	if ((result = copyinstr(progname, progNameFromUser, BUF_SIZE, &actual)) != 0){
		return result;
	}

	/* Copying the user arguments and count the number of arguments*/
	do{
		dest = kmalloc (sizeof(char));
		copyin (args, dest, sizeof(char));
		if (*(char *)dest == '\0')
			numArgs++;
		dest++;
	}while(*(char *)dest != NULL);

	
}
