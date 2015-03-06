
/* sys_exit function*/
void sys_exit(int exitcode) {
		
	/*set exited is true and fill in the exit code */
	p_table[curthread->t_pid]->exited = true;
	p_table[curthread->t_pid]->exitcode = _MK_WAIT(exitcode);
	
	/* acquire the lock and broadcast. Then, release the lock.*/
	lock_acquire(waitpidlock);
	cv_broadcast(waitpidcv, waitpidlock);
	lock_release(waitpidlock);

}

/* waitpid function */
int sys_waitpid(pid_t childpid, userptr_t status, int options, int * retval) {

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
		copyout((const void *)&p_table[childpid]->exitcode, status, sizeof(int));
		*retval = childpid;
		process_destroy(p_table[childpid]);	// create this function and make sure process table entry is also set to NULL
	} else {
		while (true) {
			cv_wait(waitpidcv, waitpidlock);
			if (p_table[childpid]->exited == true) {
				copyout((const void *)&p_table[childpid]->exitcode, status, sizeof(int));
				*retval = childpid;
				process_destroy(p_table[childpid]);	// create this function and make sure process table entry is also set to NULL;
				break;
			}
		}
	}
	lock_release(waitpidlock);	// finally release the lock
	return 0;
	
}


