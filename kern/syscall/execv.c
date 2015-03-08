#include <types.h>
#include <lib.h>

#include <kern/execv.h>
#include <copyinout.h>

#define BUFFER_SIZE 255

int sys_execv(const_userptr_t progname, userptr_t args){
	
	int result;
	int numArgs = 0;
	char *dest;
	char progNameFromUser[BUFFER_SIZE];
 	
	/*Check the validity of arguments*/

	/* Copying the program name */
	size_t actual;
	if ((result = copyinstr(progname, progNameFromUser, BUFFER_SIZE, &actual)) != 0){
		return result;
	}

	/* Copying the user arguments and count the number of arguments*/
	int padding = 0;
	
	dest = kmalloc (sizeof(char));
	copyin (args, dest, sizeof(char));
	while (dest != NULL) {	
		
		if (*dest == '\0') {
			numArgs++;
			padding = 4 - ((int)dest % 4);
			
			if (padding != 4) {
				for (int j=0; j < padding; j++) {
					dest++;
					dest = kmalloc(sizeof(char));
					*dest = '\0';
				}
			}
		}

		dest++;
		dest = kmalloc(sizeof(char));
		copyin (args, dest, sizeof(char));
	}
	
	return 0;	
}
