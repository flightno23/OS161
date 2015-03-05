
/* sys_exit function*/
void sys_exit(int exitcode) {

	struct process * proc;
	proc = p_table[curthread->t_pid]
	proc->exited = true;
	proc->exitcode = _MK_WAIT(exitcode);

}

/* waitpid function */
pid_t sys_waitpid(pid_t childpid, userptr_t status, int options) {

	/* Step 1: check if status pointer is aligned and if its valid */

	/* step 2: check if the options are valid */
	if (options != 0) {
		return EINVAL;
	}

	/* Step 3: check if childpid is valid/exist */
	if (childpid < 1 || childpid > MAX_RUNNING_PROCS) {
		return ESRCH;
	}
	if (p_table[childpid] == NULL) {
		return ESRCH;
	}

	/* Step 4: check if childpid is actually our child*/
	if (p_table[childpid]->ppid != curthread->t_pid) {
		return ECHILD;
	}

	/* Step 5: Acquire the lock and then check on the child, if not exited continue waiting on cv*/
	lock_acquire(waitpidlock);
	if (p_table[childpid]->exited == true) {
		*status = p_table[childpid]->exitcode;
		process_destroy(p_table[childpid]);	// create this function and make sure process table entry is also set to NULL
	} else {
		while (true) {
			cv_wait(waitpidcv, waitpidlock);
			if (p_table[childpid]->exited == true) {
				*(int *)status = p_table[childpid]->exitcode;
				process_destroy(p_table[childpid]);	// create this function and make sure process table entry is also set to NULL;
				break;
			}
		}
	}
	lock_release(waitpidlock);	// finally release the lock
	return 0;
	
}


