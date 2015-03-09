#include <types.h> /* for the pid_t variable */
#include <synch.h> /* to create the common condition variable */

#define MAX_RUNNING_PROCS 250

/*Common cv and lock to synchronize between processes */
struct lock * waitpidlock;// = lock_create("waitpid lock");
struct cv * waitpidcv;// = cv_create("waitpid cv");

/* Structure of a process */
struct process {

	pid_t ppid;	// parent's process id
	struct cv* exitcv;	// for synchronization
	bool exited;	// flag for exit
	int exitcode;	// exitcode sent by user
	struct thread * self;	// pointer to the thread

};

/* Global array of processes */
struct process * p_table[MAX_RUNNING_PROCS];

/* Process create function prototype */
int process_create(pid_t ppid, pid_t cpid, struct thread * selfThread);

/* pid_alloc funtion prototype */
int pid_alloc(pid_t * pidValue);

/* Process destroy function prototype */
void process_destroy(pid_t pidValue);

/* Initialize synch primitives for waitpid */
void waitpid_init(void);


