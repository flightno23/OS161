
#include <types.h> /* for various types to be accessed */
#include <thread.h> /* for the thread_fork function */
#include <current.h> /* for the value of curthread */
#include <kern/fork.h> /* to include the fork header file */
#include <addrspace.h> /* to get the as_copy, as_activate functions */
#include <lib.h> /* for the memcpy function */
// #include <kern/processManage.h>

/* fork() funtion handler*/
/* return value of 0 on success or else error code */
int sys_fork(struct trapframe * tf, int * retval) {
	
	int err;
	struct thread * newThread;
	/*call thread_fork using the child_forkentry function and the trapframe and address spaces as the arguments*/
	struct addrspace * addressChild;
	as_copy(curthread->t_addrspace, &addressChild);
	
	struct trapframe * tfChild = (struct trapframe *)kmalloc(sizeof(struct trapframe));
	*tfChild = *tf;	
	err = thread_fork("new thread", child_forkentry,tfChild,(unsigned long)addressChild, &newThread);
	if (err) {
		kfree(tfChild);
		kfree(addressChild);	
		return err;	// in case the fork failed
	}
	
	*retval = newThread->t_pid; // parent returns with child's pid
	return 0;

}

/* entry point for the child process */
/* data1 - parent's trapframe
data2 - address space of the parent */
void child_forkentry(void * data1, unsigned long data2) {

	/* Step 1: Modify the trapframe for the child to make it look like a success for the child */
	/*struct trapframe * data1Copy = (struct trapframe *)data1;
	data1Copy->tf_a3 = 0;
	data1Copy->tf_v0 = 0;
	data1Copy->tf_epc += 4;*/
	
	/* Step 2: Load and activate the address space of the child */

	curthread->t_addrspace = (struct addrspace *) data2;
	as_activate(curthread->t_addrspace);
	
	/* Step 3: Declare a struct trapframe and copy the contents of the trapframe passed in onto the new trapframe */
	struct trapframe tf;
	tf = *(struct trapframe *)data1;
	tf.tf_a3 = 0;
	tf.tf_v0 = 0;
	tf.tf_epc += 4;
	/* Step 4: Call mips_usermode and return to user mode */
	mips_usermode(&tf);
}
