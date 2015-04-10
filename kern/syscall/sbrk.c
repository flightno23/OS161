
/* sys_sbrk function */
int sys_sbrk(int amount, int * retval) {
	
	// Get the end and start of the heap as well as the base of the stack
	vaddr_t heapEnd, heapStart, stackBase;

	heapEnd = curthread->t_addrspace->as_heapEnd;
	heapStart = curthread->t_addrspace->as_heapStart;
	stackBase = curthread->t_addrspace->as_stackvBase;
	
	// parameter checking to see if heap end + amount < heap start
	if ((heapEnd + amount) < heapStart) {
		return EINVAL;	
	} else if (heapEnd + amount > stackvBase) { // parameter checking to see if heap has not overlapped with the stack
		return EINVAL;
	}

	// We are now clear to go ahead with the system call. But, before that return the old heap end through retval
	*retval = heapEnd;
	heapEnd += amount;
	
	// 0 indicates success
	return 0;	
	

}



