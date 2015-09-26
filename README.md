# OS161
<b>OS-161 </b> is an instructional OS created by Harvard University as a standalone kernel and a simple userland, all written in C. 

The source code contains implementations to the following things (added by me): <br>
1. <b>Implementation of Locks, Condition Variables and Reader-Writer Locks</b> <br>
2. <b>Implementation of file system calls - open, close, read, write, getpid, dup2</b> <br>
3. <b>Implementation of Process System calls - fork, execv, sbrk</b> <br>
4. <b>Implementation of a fully functional virtual memory subsystem - uses paging and swapping to manage memory</b> <br>
<br>
To understand the code better, I have put together a guide of where the important files are present in the source and their contents.<br> This will enable others to read my code and understand the internal workings of the OS if they want to.
<br>
a. <b>kern/syscall/execv.c</b> - Execv() system call implementation
<br>
b. <b>kern/syscall/fileOperations.c</b> - implementation of open(), close(), read(), write(), dup2(), chdir(), getcwd(), lseek() <br>
c. <b>kern/syscall/fork.c </b>- fork() system call implementation <br>
d. <b>kern/syscall/waitpidexit.c </b> - waitpid() and exit() system calls implementation<br>
e. <b>kern/vm/pageTable.c </b> - implementation of a linked list based page table structure for the new virtual memory subsystem <br>
f. <b>kern/vm/swapOperations.c </b>- implementations of swapin, swapout such that pages are moved between the disk and memory <br>
g. <b>kern/arch/mips/vm/vm.c </b>- VM implemntation ; algorithm to allocate and free pages (as well kernel pages ) and the coremap functionality has been added. <br>
