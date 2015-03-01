
#include <types.h>
#include <vnode.h>
#include <synch.h>
#define MAX_FILENAME_LEN 256

// the structure of the file handle
struct fhandle {
	char *name;
	int flags;
	off_t offset;
	int ref_count;
	struct lock * lock;
	struct vnode * vn;
};

/* fhandle create and destroy function prototype */
struct fhandle *fhandle_create(char name[], int flags, off_t offset, struct vnode *);
void fhandle_destroy(struct fhandle *); 

/* sys-open function prototype */
int sys_open(const_userptr_t fileName, int flags, int * retval);

/* sys_close function prototype */
int sys_close(int fd);

/* sys_write function prototype */
int sys_write(int fd, void * buf, size_t nbytes, int * retval);

/* sys_read function prototype */
int sys_read(int fd, void * buf, size_t nbytes, int * retval);

/* sys_dup2 function prototype */
int sys_dup2(int oldfd, int newfd, int * retval);

/* sys_chdir function prototype */
int sys_chdir(const_userptr_t pathname);

/* sys_getcwd function prototype */
int sys_getcwd(userptr_t buf, int * retval);

/* sys_lseek function prototype */
int sys_lseek(struct * tf, int * retval);
