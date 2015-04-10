
/* method to walk through the page table and find the matching entry if it exists, else return NULL */
struct page_table_entry * pgdir_walk(vaddr_t va) {
	
	struct page_table_entry * tempNode = curthread->t_addrspace->firstNode;
	
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
void deletePageTable() {
	// get the first node of the current address space
	struct page_table_entry * firstPTE = curthread->t_addrspace->firstNode;
	struct page_table_entry * temp;
	
	// loop from the first Node to the end
	while (firstPTE != null) {
		temp = firstPTE;
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
struct page_table_entry *  addPTE(struct addrspace * as, vaddr_t va,paddr_t pa) {
	
	struct page_table_entry * pteToAdd;
	pteToAdd = kmalloc(sizeof(struct page_table_entry));

	pteToAdd->va = va;
	pteToAdd->pa = pa;
	pteToAdd->state = DIRTY_PAGE;
	pteToAdd->permissions = as_get_permissions(as,va);
	struct page_table_entry * firstNode = curthread->t_addrspace->firstNode;

	// if firstNode is not initialised, set this to the first node 
	if (firstNode == NULL) {
		firstNode = pteToAdd;
	}	

	// Else, insert into head of LL
	else{
		pteToAdd->next = firstNode;
		firstNode = pteToAdd;
	}

	return;		
}

