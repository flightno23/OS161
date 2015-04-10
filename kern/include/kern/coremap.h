/*Header file for coremap*/

#define FIXED_PAGE 0
#define FREE_PAGE 1
#define CLEAN_PAGE 2
#define DIRTY_PAGE 3

/* COREMAP structure */
struct coremap_entry{

	int npages; /* holds the number of continuous allocations */
	int state; /* state of page - fixed, free, clean, dirty */
	vaddr_t va;
	struct addrspace * as;
	/* add more info here later */
};

/* total no of pages in the coremap */
int total_page_num;

/* start address of the coremap array */
struct coremap_entry *coremap;

/* lock for the coremap */
struct lock * coremapLock;

