/*Header file for coremap*/

struct coremap_entry{

	struct addrspace* as;
	vaddr_t va;
	
	int state; //Add type for this later on 
}

int total_page_num;

struct coremap_entry *coremap;
