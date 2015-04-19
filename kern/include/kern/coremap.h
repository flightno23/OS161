/*Header file for coremap*/

#define FIXED_PAGE 0
#define FREE_PAGE 1
#define CLEAN_PAGE 2
#define DIRTY_PAGE 3

/* COREMAP structure */
struct coremap_entry{

	uint32_t npages:7; /* holds the number of continuous allocations */
	uint32_t state:2; /* state of page - fixed, free, clean, dirty */
	vaddr_t va;
	struct addrspace * as;
	/* add more info here later */
	time_t secs;
	uint32_t nanosecs;

};

/* total no of pages in the coremap */
int total_page_num;

/* start address of the coremap array */
struct coremap_entry *coremap;

/* lock for the coremap */
struct lock * coremapLock;


