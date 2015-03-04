#include <kern/fileOperations.h> /*this adds the function prototypes for this file */
#include <kern/errno.h>	/*for EINVAL, other errors */
#include <types.h>  /* for types like off_t, size_t, etc*/
#include <vnode.h>  /*for all the VOP methods */
#include <lib.h>  /* for kmalloc, fuctions etc */
#include <kern/stat.h> /* for getting the file size through VOP_STAT */
#include <copyinout.h> /* for using the copyinstr function */
#include <kern/fcntl.h> /* for the O_RDONLY and the other flags */
#include <kern/limits.h>  /* for the OPEN_MAX constant */
#include <current.h> /* for the curthread variable */
#include <vfs.h> /* for vfs_open, close , etc */
#include <uio.h> /* for uio and iovec , this is for moving data from kernel to userspace and vice versa */
#include <kern/seek.h> /* for seek operation lseek() */

#define BUF_SIZE 255 /* maximum valid length of filename */

int sys_open(const_userptr_t fileName, int flags, int * retval) {
	
	char fileNameFromUser[BUF_SIZE]; // file name from copyinstr
	bool append_flag = false;	// if O_APPEND is one of the flags, this variable is set to TRUE
	int err;
	
	// Step 1: first check if the filename pointer is valid
	if (fileName == NULL) {
		err = EFAULT;
		return err;
	}		

	// Step 2: then check if the flags are valid (use switch statement to check for valid flags)
	switch(flags) {
		case O_RDONLY: break;
		case O_WRONLY: break;
		case O_RDWR: break;
		case O_RDONLY|O_CREAT: break;
		case O_WRONLY|O_CREAT: break;
		case O_WRONLY|O_APPEND: append_flag = true;
					 break;
		case O_RDWR|O_CREAT: break;
		case O_RDWR|O_APPEND: append_flag = true;
					break;
		default: err = EINVAL;
			return err;
	}
	

	// Step 3: if steps 1 and 2 are passing, then find an empty slot in the tfd_table and allocate it to this file that is going to be opened
	int slot = -1;
	for (int i=3; i<OPEN_MAX; i++) {
		if (curthread->t_fdtable[i] == NULL) {
			slot = i;
			break;	
		}
	}

		/* in case the file table is full and there are no empty slots */
	if (slot == -1) {
		err = EMFILE;
		return err;
	}

	// Step 4: copy the valid filename into the kernel buffer, using the copyinstr function
	size_t actual;
	if ((err = copyinstr(fileName, fileNameFromUser, BUF_SIZE, &actual)) != 0) {
		return err;
	}

	// Step 5: call vfs_open, with correct arguments
	struct vnode * node;
	err = vfs_open(fileNameFromUser, flags, 0, &node);
	// add code here to check if vnode returned successfully
	if (err) {	// if vnode not successful, then return appropriate error to user
		return err;
	}	
	
	// Step 6: initialize the struct fhandle with correct values, taking the flags into consideration		
	if (append_flag) {   /* case where the APPEND flag is true, get the file size and set pass it as the offset */
		struct stat buffer;
		VOP_STAT(node, &buffer);
		int bytes = buffer.st_size;
		*retval = slot;
		curthread->t_fdtable[slot] = fhandle_create(fileNameFromUser, flags, bytes, node);
	} else { /* case where append flag is not TRUE */
	 	*retval = slot;
		curthread->t_fdtable[slot] = fhandle_create(fileNameFromUser, flags, 0, node);
	}

	/* on success, return 0 */
	return 0;

}

/* close() system call handler */
int sys_close(int fd) {
	
	/* check if fd is a valid file handle first */
	if (fd >= OPEN_MAX) {	// to avoid fd's that have values > than OPEN_MAX
		return EBADF;
		if (curthread->t_fdtable[fd] == NULL) {	// to avoid fd's whose entries are already NULL
			return EBADF;
		}
	}

	/* check if file handle at that position has an ref_count of 1*/
	struct fhandle *fdesc = curthread->t_fdtable[fd];
	fdesc->ref_count--;
	
	if (fdesc->ref_count == 0) {  /*if the ref_count is 0, that is only one link to this file handle */
		curthread->t_fdtable[fd] = NULL;
		fhandle_destroy(fdesc);
	} else {	/*else, just set the file table entry to NULL */
		curthread->t_fdtable[fd] = NULL;
	}
	
	/*on success , return 0*/
	return 0;
}


/* write() call handler */
int sys_write(int fd, void *buf, size_t nbytes, int * retval){
	
	int err;	
	/* Step 1: Check if the fd value is valid and that the file handle has been opened */
	
	if (fd >= OPEN_MAX || fd <= -1) {	// out of bounds of the file table array
		return EBADF;
	} else { // if within the range of the file table
		if (curthread->t_fdtable[fd] == NULL) {		// if entry corresponding to the fd number is NULL, then return bad fd error
			return EBADF;
		}
	}
		/* Now, checking for "is file handle allowed to be written into? " */
	
	struct fhandle * fdesc;
	fdesc = curthread->t_fdtable[fd];
	
	if (fdesc->flags == O_RDONLY) {
		return EBADF;
	} 
	
	/* Step 2: Check if the buffer location is valid in userspace */
	if (buf == NULL) {
		return EFAULT;
	}
	
	
	/* Step 4: Initialize iovec and uio with uio_kinit and correct arguments */
	struct iovec iov;
	struct uio user;
	
	/* get the file handle from the file table and initialize uio with the uio_kinit method using fhandle's offset value also*/
	uio_kinit(&iov, &user, buf, nbytes, fdesc->offset, UIO_WRITE);	
	user.uio_segflg = UIO_USERSPACE;
	user.uio_space = curthread->t_addrspace;
	
	/* use VOP_WRITE to write into the file */
	err = VOP_WRITE(fdesc->vn, &user);
	if (err) {
		return err;
	}
	
	/* Calculate the amount of bytes written and return success to user (indicated by 0)*/
	*retval = nbytes - user.uio_resid;
	fdesc->offset += *retval; // move the offset by the amount of bytes written	

	return 0;	
}

/* read() call handler */

int sys_read(int fd, void *buf, size_t nbytes, int * retval){
	
	int err;	
	/* Step 1: Check if the fd value is valid and that the file handle has been opened */
	
	if (fd >= OPEN_MAX || fd <= -1) {	// out of bounds of the file table array
		return EBADF;
	} else { // if within the range of the file table
		if (curthread->t_fdtable[fd] == NULL) {		// if entry corresponding to the fd number is NULL, then return bad fd error
			return EBADF;
		}
	}
		/* Now, checking for "is file handle allowed to be written into? " */
	
	struct fhandle * fdesc;
	fdesc = curthread->t_fdtable[fd];
	
	if (fdesc->flags == O_WRONLY) {
		return EBADF;
	} 
	
	/* Step 2: Check if the buffer location is valid in userspace */
	if (buf == NULL) {
		return EFAULT;
	}
	
	
	/* Step 4: Initialize iovec and uio with uio_kinit and correct arguments */
	struct iovec iov;
	struct uio user;
	
	/* get the file handle from the file table and initialize uio with the uio_kinit method using fhandle's offset value also*/
	uio_kinit(&iov, &user, buf, nbytes, fdesc->offset, UIO_READ);	
	user.uio_segflg = UIO_USERSPACE;
	user.uio_space = curthread->t_addrspace;
	
	/* use VOP_READ to read from the file */
	err = VOP_READ(fdesc->vn, &user);
	if (err) {
		return err;
	}
	
	/* Calculate the amount of bytes written and return success to user (indicated by 0)*/
	*retval = nbytes - user.uio_resid;
	fdesc->offset += *retval; 	// advance offset by amount of bytes read	

	return 0;	
}

/* dup2() call handler*/
int sys_dup2(int oldfd, int newfd, int * retval) {


	/* Check the validity of arguments*/
	if(oldfd >= OPEN_MAX || oldfd < 0 || newfd >= OPEN_MAX || newfd < 0 ){
		return EBADF;
	}

	struct fhandle *fdesc;
	fdesc = curthread->t_fdtable[oldfd];
	/* Check if newfd points to null*/
	if (curthread->t_fdtable[newfd] == NULL){
		curthread->t_fdtable[newfd] = fdesc;
		fdesc->ref_count++;
	}else {
	/* Close existing fd */
		sys_close(newfd);
		curthread->t_fdtable[newfd] = fdesc;
		fdesc->ref_count++;
	}

	*retval = newfd;
	return 0;
}

/* chdir() function handler */
int sys_chdir(const_userptr_t pathName){
	
	char pathNameFromUser[BUF_SIZE];
	size_t actual;
	int err;

	if (pathName == NULL){
		return EFAULT;
	}

	if ((err =  copyinstr(pathName, pathNameFromUser, BUF_SIZE, &actual) != 0)){
		return err;
	}

	err = vfs_chdir(pathNameFromUser);	
	return err;
	
}

/* __getcwd() function handler */
int sys_getcwd(userptr_t buf, int * retval){

	struct uio user;
	struct iovec iov;
	uio_kinit(&iov, &user, buf, BUF_SIZE, 0, UIO_READ);
	user.uio_segflg = UIO_USERSPACE;
	user.uio_space = curthread->t_addrspace;

	
	int err;
	if ((err = vfs_getcwd(&user) != 0)){
		return err;
	}

	*retval = BUF_SIZE - user.uio_resid;
	return 0;
	
}

/* lseek() call handler */
int sys_lseek(int fd, off_t pos, int whence, off_t * retVal64){
	
	int err;
	off_t currentPosition;
	off_t endPosition;
	off_t posSeek;
	struct stat buffer;
	
	/* Step 1: Check if the file descriptor passed in is valid */
	if (fd >= OPEN_MAX || fd < 0) {	// fd is out of bounds of the file table
		return EBADF;
	} 

	struct fhandle * fdesc = curthread->t_fdtable[fd];
	if (fdesc == NULL) {
		return EBADF;
	}

	
	switch(whence) {	// logic for different cases
		case SEEK_SET: //VOP_TRYSEEK(vnode, position)
				if (pos < 0) return EINVAL;	// seek position is negative
				posSeek = pos;
				if ((err = VOP_TRYSEEK(fdesc->vn, posSeek)) != 0) {
					return ESPIPE;	// in case the SEEK fails
				}
				fdesc->offset = posSeek;
				*retVal64 = posSeek;
				break;
			
		case SEEK_CUR:  currentPosition = fdesc->offset;
				posSeek = currentPosition + pos;
				
				if (posSeek < 0) return EINVAL;
				
				if ((err = VOP_TRYSEEK(fdesc->vn, posSeek)) != 0) {
					return ESPIPE;
				}
				fdesc->offset = posSeek;
				*retVal64 = posSeek;
				break;

		case SEEK_END:  VOP_STAT(fdesc->vn, &buffer);
				endPosition = buffer.st_size;
				posSeek = endPosition + pos;
				
				if (posSeek < 0) return EINVAL;

				if ((err = VOP_TRYSEEK(fdesc->vn, posSeek)) != 0) {
					return ESPIPE;
				}
				fdesc->offset = posSeek;
				*retVal64 = posSeek;
				break;
		default: 
			return EINVAL; 
	}
	
	return 0;

}

/* This function creates the file handle */
struct fhandle * fhandle_create(char *name, int flags, off_t offset, struct vnode *node) {
	struct fhandle * fdesc;
	
	/*allocating memory for the file handle with malloc*/
	fdesc = kmalloc(sizeof(struct fhandle));
	if (fdesc == NULL)
		return NULL;
	
	/* assigning the name for the file handle */	
	fdesc->name = kstrdup(name);
	if (fdesc->name == NULL) {
		kfree(fdesc);
		return NULL;
	}

	/*initializing the flags, offset and ref_count*/
	fdesc->flags = flags;
	fdesc->offset = offset;
	fdesc->ref_count = 1;

	/* creating the lock*/
	fdesc->lock = lock_create(name);

	/* assigning the vnode for the fhandle structure*/
	fdesc->vn = node;
	
	return fdesc;
}


/* This function destroys the file handle */
void fhandle_destroy(struct fhandle *fdesc) {

	KASSERT(fdesc != NULL);
	/* destroy lock and close vnode*/
	lock_destroy(fdesc->lock);
	vfs_close(fdesc->vn);
	
	/* free the name and structure*/
	kfree(fdesc->name);
	kfree(fdesc);

}
