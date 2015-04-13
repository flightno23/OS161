#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <thread.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
#include <kern/coremap.h> /* For access to the coremap interface */
#include <kern/pageTable.h> /* For access to the pageTable interface */

/* Our very own smart VM - work in progress */


/* Macros to round up or round down */
#define ROUND_DOWN(addr, size) ((addr) - ((addr) % (size)))
#define ROUND_UP(addr, size) ((addr) - ((addr) % (size)) + (size))
#define MAX_TIME 9223372036854775807



/*
 * Wrap rma_stealmem in a spinlock.
 */
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;


/* GLOBAL flag that records whether vm has initialized */
bool has_vm_boot = false;


/* function prototype for page_nalloc */
static paddr_t page_nalloc(int);

/* function prototype for make_page_avail  */
static int make_page_avail(void);


/* function that gets npages number of pages and returns the physical address of the start point */
static
paddr_t
getppages(unsigned long npages)
{
	paddr_t addr;

	spinlock_acquire(&stealmem_lock);

	addr = ram_stealmem(npages);
	
	spinlock_release(&stealmem_lock);
	return addr;
}


/* function to bootstrap the VM */
void
vm_bootstrap(void)
{
	
	/* Step 1 - Get firstpaddr, lastpaddr, start and end point of the coremap */

	paddr_t firstpaddr, lastpaddr, freeaddr; /* hold values for first and last physical address returned by ram_getsize */
	ram_getsize(&firstpaddr, &lastpaddr); /* get the first and last physical address */
	
	total_page_num = (int) (ROUND_DOWN(lastpaddr, PAGE_SIZE)/ PAGE_SIZE); /* calculate the number of pages from 0 to lastpaddr by calling macro */

	coremap = (struct coremap_entry *)PADDR_TO_KVADDR(firstpaddr); /* coremap array starts from firstpaddr location */
	freeaddr = firstpaddr + (total_page_num * sizeof(struct coremap_entry)); /* freeaddr starts from after the coremap ends to lastpaddr */
	
	/* Step 2 - Initialize the coremap entries with pages between 0 and freeaddr being marked as FIXED and 
	pages between (freeaddr, lastaddr) marked as free */

	/* finding number of pages between [0, freeaddr) and initialize them to FIXED */
	int noOfFixed = ROUND_UP(freeaddr, PAGE_SIZE) / PAGE_SIZE;
	int i=0; /* this will act as the loop counter */

	while (i < noOfFixed) {
		
		coremap[i].state = FIXED_PAGE;
		coremap[i].npages = 1; 	
		i++;
	}
	
	/* next initialize the rest of the pages till last paddr. These will be marked FREE */
	while (i < total_page_num) {
		
		coremap[i].state = FREE_PAGE;
		coremap[i].npages = 1;
		i++;

	}

	/* Step 3 - mark the VM as bootstrapped so that alloc_kpages can function normally instead of calling ram_stealmem everytime */
	has_vm_boot = true;	
	coremapLock = lock_create("coremap lock"); // initializing the lock for the coremap array

}

/* function to allocate kernel pages - if (n>1), the pages have to be continuous */
vaddr_t alloc_kpages(int npages) {

	paddr_t pa;

	if (has_vm_boot == false) {
		pa = getppages(npages);
		if (pa == 0) {
			return 0;
		}
	} else {	// VM has bootstrapped , use page_nalloc
		lock_acquire(coremapLock);
	
		pa = page_nalloc(npages);
		if (pa == 0) {
			return 0;
		}

		lock_release(coremapLock);
	}
	return PADDR_TO_KVADDR(pa);
}

/* function to free the kernel pages */
void free_kpages(vaddr_t addr) {
	
		
	paddr_t pAddress = KVADDR_TO_PADDR(addr);	
	/* find the coremap index of the address  */
	int coremapIndex = ROUND_DOWN(pAddress, PAGE_SIZE) / PAGE_SIZE;
	

	/* make the pages back to free and npages to 1 */
	lock_acquire(coremapLock);

	int npages = coremap[coremapIndex].npages;
	
	for (int i=0; i < npages; i++) {
		coremap[coremapIndex+i].state = FREE_PAGE;
		coremap[coremapIndex+i].npages = 1;
	}

	lock_release(coremapLock);
	
}




/* function to allocate n pages - should be continuous if they are coming from alloc_kpages */
static paddr_t page_nalloc(int npages) {
	
	int j=1;
	bool chunk_okay = false;
	int coremapIndexFound = -1;
	paddr_t addrToReturn; 
	/* scan the coremap to find continous chunk of free pages */
	for (int i=0; i < total_page_num; i++) {

		if (coremap[i].state == FREE_PAGE) {
			chunk_okay = true;
			while (j < npages) {
				if (coremap[i+j].state == FREE_PAGE) {
					chunk_okay = true;
				} else {
					chunk_okay = false;
					break;
				}
			}
			if (chunk_okay == false) {
				continue; // find some other chunk of pages
			} else {
				coremapIndexFound = i;
				break;	
			}
		}
	}
	
	/* if the coremap is full, return a 0 signalling an error */
	if (coremapIndexFound == -1) return 0;
	
	/* now that coremap index to allocate has been found , allocate it and return the base physical address */
	coremap[coremapIndexFound].npages = npages;
	for (int i = 0; i < npages; i++) {
		coremap[coremapIndexFound + i].state = FIXED_PAGE;
	}
	
	/* return the physical address of the start point of the chunk of pages */
	addrToReturn = coremapIndexFound * PAGE_SIZE;
	
	return addrToReturn;	

}


/* STUB functions for compilation - modify later */
void
vm_tlbshootdown_all(void)
{
	panic("smart vm tried to do tlb shootdown?!\n");
}

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("smartvm tried to do tlb shootdown?!\n");
}

// function that handles tlb faults
int
vm_fault(int faulttype, vaddr_t faultaddress) {
	
	struct addrspace * as;
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop, heapStart, heapEnd;
	int permissions;
	
	// cleaning up the address (only high 20 bits required)
	faultaddress &= PAGE_FRAME;
	
	// sanity check to see if the address space is uninitialized
	as = curthread->t_addrspace;
	if (as == NULL) {
		return EFAULT;
	}

	/* Assert that the address space has been set up properly. */
	KASSERT(as->as_vbase1 != 0);
	KASSERT(as->as_npages1 != 0);
	KASSERT(as->as_vbase2 != 0);
	KASSERT(as->as_npages2 != 0);
	KASSERT(as->as_stackvbase != 0);
	
	// collecting information about the address space
	vbase1 = (as->as_vbase1 & PAGE_FRAME);
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = (as->as_vbase2 & PAGE_FRAME);
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = as->as_stackvbase;
	stacktop = USERSTACK;
	heapStart = as->as_heapStart;
	heapEnd = as->as_heapEnd;
	 
	// Now, check if the faultaddress is a valid one
	if (faultaddress >= vbase1 && faultaddress < vtop1) {
		// Do nothing , in valid region
	}
	else if (faultaddress >= vbase2 && faultaddress < vtop2) {
		// Do nothing , in valid region
	}
	else if (faultaddress >= stackbase && faultaddress < stacktop) {
		// Do nothing , in valid region
	}
	else if (faultaddress >= heapStart && faultaddress < heapEnd) {
		// Do nothing , in valid region
	}
	else {
		return EFAULT;	// oops, you have reached an invalid region of the address space
	}
	

	// Next, deal with VM_FAULT_READ and VM_FAULT_WRITE
	if (faulttype == VM_FAULT_READ || faulttype == VM_FAULT_WRITE) {
		struct page_table_entry * tempPTE;
		tempPTE = pgdir_walk(faultaddress);	// walk through the page table and find the PTE

		if (tempPTE == NULL) {	// no entry found, create a new entry and store in the page table
			paddr_t tempPa;
			tempPa = page_alloc(as, faultaddress);	
			tempPTE = addPTE(as, faultaddress, tempPa);
		}

		// Call tlb_random after the packing the bits
		permissions = as_get_permissions(as,faultaddress);
		uint32_t addrHi, addrLo;
		addrHi = faultaddress;
		addrLo = (((permissions & 2)|1) << 9) | (tempPTE->pa);	// setting the dirty bit ( & with 010 - 2) and valid bit ( | with 1)
		tlb_random(addrHi, addrLo);
					
	} else if (faulttype == VM_FAULT_READONLY) {  // Dealing with VM_FAULT_READONLY, when write was requested on address having read-only permission
		/* checking if the page is actually writable */
		permissions = as_get_permissions(as, faultaddress);
		if (((permissions & 2) >> 1) == 1) {
			paddr_t paRead;
			vaddr_t vaRead;
			
			int result = tlb_probe(faultaddress, 0);
			tlb_read(&vaRead, &paRead, result);
			paRead |= (1 << 10); // setting the dirty bit to 1
			tlb_write(vaRead, paRead, result);
 
		} else {
			/* panic or kill the process */
			panic("user tried writing into a region without write permissions. WTF!\n");
		}	
	}
	
	return 0; // success
		
}


/* Method to look for a free physical page or finds a victim according to timestamp to throw out */
static int make_page_avail (){

        time_t min_time;
        min_time = MAX_TIME;
        int index_to_ret;


        /*Find appropriate index of coremap to allocate a free page or the oldest page*/
        for (int i=0; i < total_page_num; i++){

                /* Checks to find a free page */
                if (coremap[i].state == FREE_PAGE){
                        return i;
                }

                /* Checks to find the oldest page which is not fixed */
                else if (coremap[i].state != FIXED_PAGE){
                        if (coremap[i].timeStamp < min_time){
                                min_time = coremap[i].timeStamp;
                                index_to_ret = i;
                        }
                }

        }

        return index_to_ret;

}

/* function to zero a particular region */
static
void
as_zero_region(paddr_t paddr, unsigned npages)
{
	bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}


/* Method to allocate one physical page to the user */
paddr_t page_alloc(struct addrspace *as, vaddr_t va){

	int index;
	
	lock_acquire (coremapLock);
	
	index = make_page_avail();	// Find an index in coremap with FREE page or victim page that is not FIXED
	
	as_zero_region(index * PAGE_SIZE, 1); // Zero the said PAGE
	
	coremap[index].as = as;		// coremap entry now point to this PAGE
	coremap[index].va = va;
	coremap[index].state = DIRTY_PAGE;
	coremap[index].npages = 1;
	
	lock_release(coremapLock);

	return index*PAGE_SIZE;	
}

/* Method to free a page with a start virtual address va */
void page_free(vaddr_t va){

	/* Define a spinlock to protect the TLB */

	/* Find if the page is in the TLB and shoot it down */

	int spl;
	spl = splhigh();

	uint32_t ehi, elo;

	for (int i=0; i < NUM_TLB; i++){
		
		tlb_read(&ehi, &elo, i);
		if(ehi == (va & PAGE_FRAME)){
			
			tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
			break;
		}
	}

	splx(spl);

	/* Free the corresponding coremap entry */
	lock_acquire(coremapLock);  //optimize lock
	for (int i = 0; i < total_page_num; i++){
		
		if(coremap[i].va == va){
			
			coremap[i].as = NULL;
			coremap[i].va = 0;
			coremap[i].npages = 0;
			coremap[i].state = FREE_PAGE;
			break;

		}
	}
	lock_release(coremapLock);

}
	
