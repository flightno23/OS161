#include <types.h> /* for the pid_t variable */

#define MAX_RUNNING_PROCS 250

/* Structure of a process */
struct process {

	pid_t ppid;	// parent's process id
	struct semaphore* exitsem;	// for synchronization
	bool exited;	// flag for exit
	int exitcode;	// exitcode sent by user
	struct thread * self;	// pointer to the thread

}

/* Global array of processes */
struct process * p_table[MAX_RUNNING_PROCS];

/* Process create function prototype */
pid_t process_create(struct thread * selfThread);

/* Process destroy function prototype */
void process_destroy(struct process *);



