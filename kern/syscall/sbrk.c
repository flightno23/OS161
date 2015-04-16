#include <types.h>  /* file for different types of OS161 */
#include <kern/sbrk.h> /* header file of this file */
#include <current.h>  /* for curthread */
#include <addrspace.h> /* heap start, end , etc */
#include <kern/errno.h> /* for different error constants */

/* sys_sbrk function */
int sys_sbrk(int amount, int * retval) {
	
	// Get the end and start of the heap as well as the base of the stack
	vaddr_t heapEnd, heapStart, stackBase;

	heapEnd = curthread->t_addrspace->as_heapEnd;
	heapStart = curthread->t_addrspace->as_heapStart;
	stackBase = curthread->t_addrspace->as_stackvbase;
	
	// parameter checking to see if heap end + amount < heap start
	if ((heapEnd + amount) < heapStart) {
		return EINVAL;	
	} else if (heapEnd + amount > stackBase) { // parameter checking to see if heap has not overlapped with the stack
		return EINVAL;
	}

	// We are now clear to go ahead with the system call. But, before that return the old heap end through retval
	*retval = heapEnd;
	heapEnd += amount;
	curthread->t_addrspace->as_heapEnd = heapEnd;
 
	// 0 indicates success
	return 0;	
	

}



