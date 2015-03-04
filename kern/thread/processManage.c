
/* Process Create Function */
pid_t process_create(struct thread * selfThread) {
	
	/* initialize the process structure */	
	pid_t retval;	
	struct process * proc;
	proc = kmalloc(sizeof(*proc));
	proc->exitsem = sem_create("procsem", 0); 	
	exited = false;
	
	/* find an empty slot in the process table and return the pid*/
	for (int i=1; i< MAX_RUNNING_PROCS; i++) {
		if (p_table[MAX_RUNNING_PROCS] == NULL) {
			 p_table[MAX_RUNNING_PROCS] = proc;
			 retval = i;
			 break;
		}
	}
	
	proc->self = selfThread;

	return retval;
}


/* Process Destroy Function*/
void process_destroy(struct process * proc) {
	
	/* Destroy the process structure and set the p_table value of the corresponding process to NULL*/
	sem_destroy(proc->exitsem);
	p_table[MAX_RUNNING_PROCS] = NULL;
	kfree(proc);

}
