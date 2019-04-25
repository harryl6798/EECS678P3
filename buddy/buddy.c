/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	/* TODO: DECLARE NECESSARY MEMBER VARIABLES */
	int index;
	int page_order;
	char* mem_loc;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/


/**
 * Return the minimal block order which can contain the  
 * memory requested.
 */
int get_order(int size){
	int ord = 1;
	size--;
	while(size>>=1){
		ord++;
	}
	if(ord < MIN_ORDER)
		return MIN_ORDER;
	else
		return ord;
}

/**
 * Given an order of block to obtain, either 
 * split the larger block, or return a free one
 */
void *get_free_block(int order){
	if(order > MAX_ORDER || order < MIN_ORDER){
		return NULL;
	}
	else if(list_empty(&free_area[order])){
		//There's no free block of that order, split next level,
		//add right hand block to the free_area, return the left_hand block
		void* super_block = get_free_block(order + 1);
		PDEBUG("Splitting %dK -> %dK + %dK\n", 1 << order+1-10, 1<< order-10, 1<<order-10);
		//Left block addr  = super_block
		void* right_block = BUDDY_ADDR(super_block, order);
		g_pages[ADDR_TO_PAGE(right_block)].page_order = order; //update the order of the pages just split
		g_pages[ADDR_TO_PAGE(super_block)].page_order = order;

		//add right_block to the free_area
		list_add(&g_pages[ADDR_TO_PAGE(right_block)].list, &free_area[order]);
		return super_block;
	}
	else{
		//There's a free block of the correct order
		page_t *iter = NULL;
		int lowest_i = INT_MAX;
		list_for_each_entry(iter, &free_area[order], list){
			if(iter->index < lowest_i){
				lowest_i = iter->index;
			}
		}
		list_del_init(&g_pages[lowest_i].list); //Delete the free block from the free_area list
		return PAGE_TO_ADDR(lowest_i);
	}
}


/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		g_pages[i].index = i;
		g_pages[i].page_order = MAX_ORDER; 
		g_pages[i].mem_loc = PAGE_TO_ADDR(i);
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	int order = get_order(size);
	PDEBUG("%d",order);
	fflush(stdout);

	return get_free_block(order);
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */

}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}

