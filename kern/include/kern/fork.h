#include <machine/trapframe.h> /* for access to the trapframe of the parent process */


/* sys_fork funtion prototype */
int sys_fork(struct trapframe tf, int * retval);

/* child_forkentry function prototype */
void child_forkentry(void * data1, unsigned long data2);


