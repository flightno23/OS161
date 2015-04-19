/* Custom page table interface - we are following a linked list structure for the page tables */
#include <addrspace.h>

struct page_table_entry {

	vaddr_t va;	// base virtual address of the page
	paddr_t pa;	// base physical address of the page if in memory (no concept of swapping added yet)
	

	// states, permissions information , etc .. add later as needed 

	uint32_t state:2;
	uint32_t permissions:3;
	struct page_table_entry * next;
	bool inDisk:1;
};


/* method to add a node to the page table */
struct page_table_entry * addPTE(struct addrspace * as, vaddr_t va, paddr_t pa);

/* method to walk through the page table and find and entry that matches the virtual address */
struct page_table_entry * pgdir_walk(struct addrspace * as, vaddr_t va);

/* method to delete an entry from the page from the page table (Not used currently) */
void deletePTE(vaddr_t va);

/* method to delete the entire page table structure - freeing memory carefully */
void deletePageTable(struct addrspace * as);

/* method to copy the linked list page table structure for as_copy */
struct page_table_entry * copyPageTable(struct page_table_entry * firstNode, struct addrspace * as);

