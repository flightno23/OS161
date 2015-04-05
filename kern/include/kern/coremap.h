/*Header file for coremap*/

struct coremap_entry{

	struct page_struct *page;
	unsigned int cpu_index:4; /* the cpu's TLB on which this page exists */
	int tlb_index:7; /* specific index of the TLB (0-63) */
	bool is_kern_page:1; /* (is this a kernel page ? ) */
	bool is_last_page:1; /* (is this the last page ? ) */
	bool is_allocated:1; /* (has this page been already allocated? ) */
	bool is_pinned:1; /* is this page pinned (notion of pinned -  when a page is being used, for ex. during swapping its pinned*/
}

struct page_struct{
	
	paddr_t ps_paddr; /* physical address of the page (start point) */
	off_t ps_swapaddr; /* swap address in case the page is in the disk */
	struct spinlock ps_spinlock; /* lock to protect access to this page */

}

int total_page_num;

struct coremap_entry *coremap;
