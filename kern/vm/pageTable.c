#include <types.h>
#include <kern/pageTable.h> /* to get all the function prototypes in this file */
#include <current.h> /* for curthread */
#include <lib.h> /* for kfree and other standard library functions */
#include <kern/coremap.h> /* for constants such as DIRTY_PAGE, CLEAN_PAGE and functions defined in it */
#include <spl.h>

/* method to walk through the page table and find the matching entry if it exists, else return NULL */
struct page_table_entry * pgdir_walk(struct addrspace * as, vaddr_t va) {
	
	struct page_table_entry * tempNode = as->firstNode;
	
	// if page table has no entries, return NULL
	if (tempNode == NULL) return NULL;

	// else, search for matching nodes
	while (tempNode != NULL) {
		if ( (tempNode->va & PAGE_FRAME) == va) {
			return tempNode;
		}
		tempNode = tempNode->next;
	}

	// last case, full page table scanned and entry not found
	return NULL;
		
}

/* method that deletes the entire page table */
void deletePageTable(struct addrspace * as) {
	// get the first node of the current address space
	struct page_table_entry * firstPTE = as->firstNode;
	struct page_table_entry * temp;
	
	// loop from the first Node to the end
	while (firstPTE != NULL) {
		temp = firstPTE;
		page_free(as, temp->va);
		firstPTE = firstPTE->next;
		kfree(temp);
	}

	return;
}

/* method to delete a particular page table entry */
void deletePTE(vaddr_t va) {
	
	struct page_table_entry * current = curthread->t_addrspace->firstNode;
	struct page_table_entry * previous = NULL;

	// if firstNode is NULL, then there is nothing to delete
	if (current == NULL) {
		return;
	} else if ((current->va & PAGE_FRAME) == va) { // else if firstNode is the node to delete
		curthread->t_addrspace->firstNode = current->next;
		kfree(current);
		return;
	}

	// else loop until the virtual address is found
	while (current != NULL) {
		if ((current->va & PAGE_FRAME) == va) {
			previous->next = current->next;
			kfree(current);
			break;
		}
		previous = current;
		current = current->next;
	}	
	
}

/* method to add a page table entry into the page table (this inserts into head of linked list) */
struct page_table_entry * addPTE(struct addrspace * as, vaddr_t va, paddr_t pa) {
	
	struct page_table_entry * pteToAdd;
	pteToAdd = kmalloc(sizeof(struct page_table_entry));

	pteToAdd->va = va;
	pteToAdd->pa = pa;
	pteToAdd->state = DIRTY_PAGE;
	pteToAdd->permissions = as_get_permissions(as,va);
	pteToAdd->inDisk = false;
	pteToAdd->next = NULL;

	// if firstNode is not initialised, set this to the first node 
	if (curthread->t_addrspace->firstNode == NULL) {
		curthread->t_addrspace->firstNode = pteToAdd;
	}	

	// Else, insert into head of LL
	else{
		pteToAdd->next = curthread->t_addrspace->firstNode;
		curthread->t_addrspace->firstNode = pteToAdd;
	}

	return pteToAdd;		
}

/* method to copy a page table given the first node of the page table to be copied */
struct page_table_entry * copyPageTable(struct page_table_entry * firstNode, struct addrspace * as) {
	int spl;
	
	// if there is nothing to copy, return NULL
	if (firstNode == NULL) {
		return NULL;
	}

	// else start copying the page table
	struct page_table_entry * temp = firstNode;
	struct page_table_entry * newFirstNode = NULL;
	int pageIndex;
	
	// copy to head using a while loop as order doesn't matter
	while (temp != NULL) {	
		struct page_table_entry * newNode = kmalloc(sizeof(struct page_table_entry));
		newNode->va = temp->va;
		newNode->permissions = temp->permissions;
		newNode->state = temp->state;
		
		// Allocate a new page and move contents from the old page physical address to the new page
		newNode->pa = page_alloc(as, newNode->va, pageIndex); //Change this!!!
		
		spl = splhigh();

		memcpy((void *) PADDR_TO_KVADDR(newNode->pa), (const void *) PADDR_TO_KVADDR(temp->pa), PAGE_SIZE);
		
		splx(spl);
			
		newNode->next = newFirstNode;
		newFirstNode = newNode;
		temp = temp->next;
	}

	return newFirstNode;

	
}


