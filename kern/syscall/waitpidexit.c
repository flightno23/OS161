
#include <types.h>
#include <kern/waitpidexit.h>
#include <synch.h>
#include <current.h>
#include <kern/errno.h>
#include <copyinout.h>
#include <kern/wait.h> /*for MK_WAIT function*/
#include <thread.h> /* for exiting the thread */

/* sys_exit function*/
void sys_exit(int exitcode) {
		
	/*set exited is true and fill in the exit code */
	lock_acquire(waitpidlock);
	p_table[curthread->t_pid]->exited = true;
	p_table[curthread->t_pid]->exitcode = _MKWAIT_EXIT(exitcode);	
	
	/* Broadcast on the wait channel that I have exited*/
	cv_broadcast(waitpidcv, waitpidlock);
	lock_release(waitpidlock);
	
	thread_exit();

}

/* waitpid function */
int sys_waitpid(pid_t childpid, userptr_t status, int options, int * retval) {

	/* Step 1: check if status pointer is aligned and if its valid */

	/* step 2: check if the options are valid */
	if (options != 0) {
		*retval = -1;
		return EINVAL;
	}

	/* Step 3: check if childpid is valid/exist */
	if (childpid < 1 || childpid > MAX_RUNNING_PROCS) {
		*retval = -1;
		return ESRCH;
	}
	if (p_table[childpid] == NULL) {
		*retval = -1;
		return ESRCH;
	}

	/* Step 4: check if childpid is actually our child*/
	if (p_table[childpid]->ppid != curthread->t_pid) {
		*retval = -1;
		return ECHILD;
	}

	/* check the status pointer for kernel, NULL or invalid pointer */
	if (status == NULL || status == (void *)0x40000000 || status == (void *)0x80000000) {
		*retval = -1;
		return EFAULT;
	}	

	/* Step 5: Acquire the lock and then check on the child, if not exited continue waiting on cv*/
	
	lock_acquire(waitpidlock);
	if (p_table[childpid]->exited == true) {
		copyout((const void *)&p_table[childpid]->exitcode, status, sizeof(int));
		*retval = childpid;
		process_destroy(childpid);	// create this function and make sure process table entry is also set to NULL
	} 
	else {
		while (p_table[childpid]->exited != true) {
			cv_wait(waitpidcv, waitpidlock);
		}
		
		//lock_acquire(waitpidlock);
		copyout((const void *)&p_table[childpid]->exitcode, status, sizeof(int));
		*retval = childpid;
		process_destroy(childpid);	// create this function and make sure process table entry is also set to NULL;
	}
	lock_release(waitpidlock);	// finally release the lock
	
	return 0;
	
}

/* sys_getpid() funtion */
void sys_getpid(int * retval) {

	*retval = curthread->t_pid;

}
