/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <kern/pageTable.h> /* For access to the page table structure and interface */
#include <spl.h> /* For access to the the interrupts level setter functions splhigh and spl */
#include <mips/tlb.h> /* for access to the TLB interface */

/* Number of stack pages in system - right now its hard coded */
#define SMARTVM_STACKPAGES 12


/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

/* Function that initialises the new address space for you and returns it back  */
struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}

	/*
	 * Initialize as needed.
	 */	
	as->as_vbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_npages2 = 0;
	as->as_stackvbase = 0;	// base of the stack . Stack goes from as_stackvbase -> stacktop
	as->as_stacknPages = 0;	// number of pages held by the stack  
	as->as_heapStart = 0;	// start point of the heap
	as->as_heapEnd = 0;	// end point of the heap
	as->as_heapnPages = 0;	// number of pages that the heap is currently holding

	return as;
}


/* Copies the address space of the old process and creates an identical address space which is returned to user */
int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */

	// copy the page table of the old address space
	newas->firstNode = copyPageTable(old->firstNode);
	
	// copy the region contents of the address space
		
	newas->as_vbase1 = old->as_vbase1;
	newas->as_npages1 = old->as_npages1;
	newas->as_vbase2 = old->as_vbase2;
	newas->as_npages2 = old->as_npages2;
	newas->as_stackvbase = old->as_stackvbase;	// base of the stack . Stack goes from as_stackvbase -> stacktop
	newas->as_stacknPages = old->as_stacknPages;	// number of pages held by the stack  
	newas->as_heapStart = old->as_heapStart;	// start point of the heap
	newas->as_heapEnd = old->as_heapEnd;	// end point of the heap
	newas->as_heapnPages = old->as_heapnPages;	// number of pages that the heap is currently holding

	*ret = newas;
	return 0;
}


/* Destroys the address space structure as well as the page table data structure associated with it*/
void
as_destroy(struct addrspace *as)
{
	
	deletePageTable();	
		
	kfree(as);
}


/* this function will activate the address space by shooting down the TLB entries */
void
as_activate(struct addrspace *as)
{
	/*
	 * Write this.
	 */	
	int i, spl;

	(void)as;

	/* Disable interrupts on this CPU while frobbing the TLB. */
	spl = splhigh();

	for (i=0; i<NUM_TLB; i++) {
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */


	size_t npages; 

	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = sz / PAGE_SIZE;

	/* Permissions are marked as follows - (read bit, write bit, executable bit) with executable bit being the LSB*/
	vaddr = (vaddr | (readable|writeable|executable));	

	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
		as->as_heapStart = as->as_vbase1 + (as->as_npages1 * PAGE_SIZE) + 1;
		as->as_heapEnd = as->as_heapStart; 	
		return 0;
	}

	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
		as->as_heapStart = as->as_vbase2 + (as->as_npages2 * PAGE_SIZE) + 1;
		as->as_heapEnd = as->as_heapStart; 	
		return 0;
	}

	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;

}

/* Method that sets the (code, readonly and data regions) as read-write permissions to load binary */
int
as_prepare_load(struct addrspace *as)
{
	
	/* use the bits (3,4,5) to record the old permissions - make this change to vbase1 and vbase2 */
	int permissions, newPermissions;
	permissions = (as->as_vbase1 & 7) << 3;
	/* new permissions recorded in bits (0,1,2) */
	newPermissions = (permissions | 6 );
	
	/* clearing the last 12 bits and setting new permissions */
	as->as_vbase1 &= PAGE_FRAME;
	as->as_vbase1 |= newPermissions;

	/* repeating the same steps for vbase2 */
	
	permissions = (as->as_vbase2 & 7) << 3;
	/* new permissions recorded in bits (0,1,2) */
	newPermissions = (permissions | 6 );
	
	/* clearing the last 12 bits and setting new permissions */
	as->as_vbase2 &= PAGE_FRAME;
	as->as_vbase2 |= newPermissions;
	

	return 0;
}

/* Method that resets back the regions to the original permissions */
int
as_complete_load(struct addrspace *as)
{
	int newPermissions;
	/* get the permissions from the as for the vbase1 region and revert to older permissions */
	newPermissions = (as->as_vbase1 & 0x3F) >> 3; // 0x3F stands for (111 111) in binary
	as->as_vbase1 &= PAGE_FRAME;
	as->as_vbase1 |= newPermissions;

	
	/* get the permissions from the as for the vbase2 region and revert to older permissions */
	newPermissions = (as->as_vbase2 & 0x3F) >> 3;
	as->as_vbase2 &= PAGE_FRAME;
	as->as_vbase2 |= newPermissions;
	
	
	
	return 0;
}

/* Function that defines the start of the STACK */
int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;
	(void) as;
	
	return 0;
}

/* Function that gets the regions permissions that a fault address belongs to */
int as_get_permissions(struct addrspace * as, vaddr_t faultaddress){
	
	// Variable declarations	
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop, heapStart, heapEnd;
	
	// collecting information about the address space
        vbase1 = as->as_vbase1;
        vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
        vbase2 = as->as_vbase2;
        vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
        stackbase = USERSTACK - SMARTVM_STACKPAGES * PAGE_SIZE;
        stacktop = USERSTACK;
        heapStart = as->as_heapStart;
        heapEnd = as->as_heapEnd;

	int permissions;

	// The permissions will be set in the last 3 bits
	// 7 is 111 (binary)
	if (faultaddress >= vbase1 && faultaddress < vtop1) {
		permissions = (7 & vbase1); 
        }
        else if (faultaddress >= vbase2 && faultaddress < vtop2) {
		permissions = (7 & vbase2);
        }
        else if (faultaddress >= stackbase && faultaddress < stacktop) {
		permissions = (7 & stackbase);
        }
        else if (faultaddress >= heapStart && faultaddress < heapEnd) {
		permissions = (7 & heapStart);
        }
	
	return permissions;                

}
