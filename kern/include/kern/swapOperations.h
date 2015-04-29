#include <types.h>
#include <addrspace.h>
#include <kern/pageTable.h>
#include <kern/stat.h>

#define MAX_SWAPPED_PAGES 4096

/* global variable to indicate first swap out */
bool firstSwapOccur;

/* Structure of each page swapped in disk 
	- will help locate location of page in the swap file */
struct swapPageEntry {
	
	struct addrspace * as;
	vaddr_t va;

};

/* Global array of swapped pages (maximum number of swapped pages are 256) */
struct swapPageEntry * swapMap[MAX_SWAPPED_PAGES];

/* Global swapfile which holds the swapped pages - will be opened when the first swapout operation occurs */
struct vnode * swapFile;

/* stat of the swap file*/
struct stat st;


/* Swapping operations interface - 
	swapout - write dirty pages to disk
	swapin  - bring in pages from disc
	write_page - write to swap file
	read_page - read from swap file
	locate_free_entry - find free entry in the swapMap array
	locate_swap_page - find the location of page in swap file
*/

/* Swaps out the page present at a particular index of the coremap 
	as indicated by indexToSwap */
void swapout(int indexToSwap);

/* Swaps in page whose details are denoted by PTE to the index 
	as indicated by indexToSwap */
void swapin(struct page_table_entry * tempPTE, int indexToSwap);

/* IO operation functions on the swap file 
	write_page - write page contents to a specified region of the swap file
	read_page - read page contents from the swap file to a particular region of the coremap
*/
void write_page(int indexToSwapOut); 

void read_page(int indexToSwapIn, int indexOnMap);

/* Function to locate a free entry in the swapMap array */
int locate_free_entry(void);

/* Function to locate the position of the page in the swap file by matching (as, va) as the key */
int locate_swap_page(struct addrspace * as, vaddr_t va);

