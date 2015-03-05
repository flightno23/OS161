
/* Process Create Function */
int process_create(pid_t ppid, pid_t cpid, struct thread * selfThread) {
	
	/* initialize the process structure */	
	struct process * proc;
	proc = kmalloc(sizeof(*proc));
	proc->exitcv = waitpidcv;
	exited = false;
	proc->self = selfThread;
	
	/* initialize the process table with child's pid*/	
	p_table[cpid] = proc;

	return 0;
}


/* pid allocator function */
int pid_alloc(pid_t * pidValue) {
	
	for (int i=1; i < MAX_RUNNING_PROCS; i++) {
		if (p_table[MAX_RUNNING_PROCS] == NULL) {
			 *pidValue = i;
			 return 0;	// return 0 upon success
		}	
	}

	return 1;	// case of error, process table is full
}

/* Process Destroy Function*/
void process_destroy(struct process * proc) {
	
	/* Destroy the process structure and set the p_table value of the corresponding process to NULL*/
	p_table[MAX_RUNNING_PROCS] = NULL;
	kfree(proc);

}
