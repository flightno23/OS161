#include <types.h>
#include <lib.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <thread.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>

#include <kern/execv.h>
#include <copyinout.h>
#include <syscall.h>

#define BUFFER_SIZE 255

int sys_execv(const_userptr_t progname, userptr_t args){
	
	
	/* Step 1: get the program name from the user */
	
	char progNameFromUser[BUFFER_SIZE];
	int result = 0;
	size_t actual;
	/* check progname pointer for NULL */
	if (progname == NULL) {
		return EFAULT;
	}

	if ((result = copyinstr(progname, progNameFromUser, BUFFER_SIZE, &actual)) != 0){
		return result;
	}

	/* check if program name is empty */
	if (strcmp(progNameFromUser, "") == 0) {
		return EINVAL;
	}

	if (args == NULL || args == (void *)0x80000000 || args == (void *) 0x40000000) {
		return EFAULT;
	}
	

	/* Step 2: get the number of arguments from userspace */
	int numArgs = 0;
	int random;
	
	
	int i = 0;
	//char junk[255];

	// This while loop gets the number of arguments from userspace
	while (*(char **)(args+i) != NULL) {
		result = copyin(args+i, &random, sizeof(int));
		if (result) return EFAULT;
		//result = copyinstr((userptr_t)random, junk, 255, &actual);
		//if (result) return EFAULT; 
		numArgs++;
		i += 4;
	}	

	/* get the arguments from userspace and store it into a array of strings called commands 
		commmands[0] will store the program name, commands[1] onwards will store any extra arguments, if present*/	
	char * commands[numArgs];
	int pointersToGet[numArgs];
	int j = 4;
	for (int i=0; i < numArgs; i++) {
		commands[i] = kmalloc(100*sizeof(char));
		
		result = copyin(args + (j*i), &pointersToGet[i], sizeof(int));
		if (result) return EFAULT;
	
		result = copyinstr((userptr_t)pointersToGet[i] , commands[i], 100, &actual);
		if (result) return EFAULT; 
	}

	void * startPoint;
	int numBytes = 0;
	int len;
	int padding = 0;
	/* Creating (numArgs+1) contigous block of pointers of 4 bytes each */
	char nullPadding[4] = "\0\0\0\0";
	startPoint = kmalloc(sizeof(int *) * (numArgs+1));
	numBytes += (sizeof(int *) * (numArgs+1));
	startPoint += numBytes;
	int intSize = 4;

	for (i=0;i<numArgs; i++){
		*(int *)((startPoint-numBytes) + (i*intSize))  = numBytes;
		len = strlen(commands[i])+1;
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

	startPoint -= numBytes;


	/* Destroy the current address-space */
	as_destroy(curthread->t_addrspace);
	curthread->t_addrspace = NULL;

	/* Rest of the functionality is similar to runprogam */
	struct vnode *p;
        vaddr_t entrypoint, stackptr;

        /* Open the file. */
        result = vfs_open(progNameFromUser, O_RDONLY, 0, &p);
        if (result) {
                return result;
        }

        /* We should be a new thread. */
        KASSERT(curthread->t_addrspace == NULL);

        /* Create a new address space. */
        curthread->t_addrspace = as_create();
        if (curthread->t_addrspace==NULL) {
                vfs_close(p);
                return ENOMEM;
        }

        /* Activate it. */
        as_activate(curthread->t_addrspace);

        /* Load the executable. */
	        result = load_elf(p, &entrypoint);
        if (result) {
                /* thread_exit destroys curthread->t_addrspace */
                vfs_close(p);
                return result;
        }

        /* Done with the file now. */
        vfs_close(p);

        /* Define the user stack in the address space */
        result = as_define_stack(curthread->t_addrspace, &stackptr);
        if (result) {
                /* thread_exit destroys curthread->t_addrspace */
                return result;
        }
	
	uint32_t stackStart;
	stackStart = stackptr - numBytes;
	
	/* Update the addresses that need to be copied in the user stack*/
	for (i = 0; i < numArgs; i ++){

		*(int*)startPoint = stackStart + *(int *)startPoint;
		startPoint += 4;
	}	                             

	*(int *)startPoint = 0x0;
	startPoint -= (numArgs*4);

	copyout(startPoint, (userptr_t)stackStart, numBytes);
	stackptr -= numBytes;		
	KASSERT(stackStart == stackptr);
	enter_new_process(numArgs, (userptr_t)stackptr, stackptr, entrypoint);
	
	return 0; 
}
