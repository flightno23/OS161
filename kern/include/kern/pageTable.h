/* Custom page table interface - we are following a linked list structure for the page tables */

struct page_table_entry {

	vaddr_t va;	// base virtual address of the page
	paddr_t pa;	// base physical address of the page if in memory (no concept of swapping added yet)
	

	// states, permissions information , etc .. add later as needed 

	int state;
	int permissions:3;
	struct page_table_entry * next;
}


/* method to add a node to the page table */
struct page_table_entry * addPTE(struct page_table_entry * pteToAdd);

/* method to walk through the page table and find and entry that matches the virtual address */
struct page_table_entry * pgdir_walk(vaddr_t va);

/* method to delete an entry from the page from the page table */
void deletePTE(vaddr_t va);

/* method to delete the entire page table structure - freeing memory carefully */
void deletePageTable();

