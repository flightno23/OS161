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

	(void)old;
	
	*ret = newas;
	return 0;
}


/* Destroys the address space structure as well as the page table data structure associated with it*/
void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */
	
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
		
		return 0;
	}

	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
		return 0;
	}

	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;

}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	(void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;
	
	return 0;
}

int as_get_permissions(struct addrspace * as, vaddr_t faultaddress){
	
	// collecting information about the address space
        vbase1 = as->as_vbase1;
        vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
        vbase2 = as->as_vbase2;
        vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
        stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
        stacktop = USERSTACK;
        heapstart = as->as_heapStart;
        heapend = as->as_heapEnd;

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
