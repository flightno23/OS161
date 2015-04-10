/* Header file for page table related operations */

struct page_table_entry{

	vaddr_t va;
	paddr_t pa;

	// add other components - states, permissions.

	struct page_table_entry * next;
}

struct page_table_entry * pgdir_walk(vaddr_t va){

	


}
