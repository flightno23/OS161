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
#include <spl.h>
#include <spinlock.h>
#include <thread.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>

/* Our very own smart VM - work in progress */


/* Macros to round up or round down */
#define ROUND_DOWN(addr, size) ((addr) - ((addr) % (size)))
#define ROUND_UP(addr, size) ((addr) - ((addr) % (size)) + (size))

/*
 * Wrap rma_stealmem in a spinlock.
 */
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

void
vm_bootstrap(void)
{
	
	/* Step 1 - Get firstpaddr, lastpaddr, start and end point of the coremap */

	paddr_t firstpaddr, lastpaddr; /* hold values for first and last physical address returned by ram_getsize */
	ram_getsize(&firstpaddr, &lastpaddr); /* get the first and last physical address */
	
	total_page_num = ROUND_DOWN(lastpaddr, PAGE_SIZE)/PAGE_SIZE; /* calculate the number of pages from 0 to lastpaddr by calling this macro */

	coremap = (struct coremap_entry *)PADDR_TO_KVADDR(firstpaddr); /* coremap array starts from firstpaddr location */
	freeaddr = firstpaddr + total_page_num*sizeof(struct coremap_entry); /* freeaddr starts from after the coremap ends to lastpaddr */
	
	/* Step 2 - Initialize the coremap entries with pages between 0 and freeaddr being marked as FIXED and 
	pages between (freeaddr, lastaddr) marked as free */

	/* finding number of pages between [0, freeaddr) and initialize them to FIXED */
	int noOfFixed = ROUND_UP(freeaddr, PAGE_SIZE) / PAGE_SIZE;
	int i=0; /* this will act as the loop counter */

	while (i < noOfFixed) {
		
		coremap[i].
		i++;
	}
	
	/* next initialize the rest of the pages till last paddr. These will be marked FREE */
	while (i < total_page_num) {

		coremap[i].
		i++;
	}

	/* Step 3 - mark the VM as bootstrapped so that alloc_kpages can function normally instead of calling ram_stealmem everytime */
	
	

}

