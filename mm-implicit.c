// This file gives you a starting point to implement malloc using implicit list
// Each chunk has a header (of type header_t) and does *not* include a footer
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "mm-common.h"
#include "mm-implicit.h"
#include "memlib.h"

// turn "debug" on while you are debugging correctness. 
// Turn it off when you want to measure performance
static bool debug = false; 

size_t hdr_size = sizeof(header_t);

void 
init_chunk(header_t *p, size_t csz, bool allocated)
{
	p->size = csz;
	p->allocated = allocated;
}

// Helper function next_chunk returns a pointer to the header of 
// next chunk after the current chunk h.
// It returns NULL if h is the last chunk on the heap.
// If h is NULL, next_chunk returns the first chunk if heap is non-empty, and NULL otherwise.
header_t *
next_chunk(header_t *h)
{
	if (h == NULL){
		if (mem_heapsize()==0){
			return NULL;
		}else{
			return mem_heap_lo();
		}
	}else{
		header_t *ret_header = h;
		ret_header += h->size / hdr_size;
		if ((char *)ret_header >= (char *)mem_heap_hi()){
			return NULL;
		}else{
			return ret_header;
		}
	}
	
}


/* 
 * mm_init initializes the malloc package.
 */
int mm_init(void)
{
	//double check that hdr_size should be 16-byte aligned
	assert(hdr_size == align(hdr_size));
	// start with an empty heap. 
	// no additional initialization code is necessary for implicit list.
	return 0;
}


// helper function first_fit traverses the entire heap chunk by chunk from the begining. 
// It returns the first free chunk encountered whose size is bigger or equal to "csz".  
// It returns NULL if no large enough free chunk is found.
// Please use helper function next_chunk when traversing the heap
header_t *
first_fit(size_t csz)
{
	//Your code herheader_t 
//	header_t *header = NULL;
//	while (next_chunk(header) != NULL){
//		if (next_chunk(header)->allocated == false && next_chunk(header)->size >= csz){
//			return next_chunk(header);
//		} 
//		header = next_chunk(header);
//	}
//	return NULL;
	header_t *header = next_chunk(NULL);
	while (header != NULL){
		if (header->allocated == false && header->size >= csz){
			return header;
		}
		header = next_chunk(header);
	}
	return NULL;

}

// helper function split cuts the chunk into two chunks. The first chunk is of size "csz", 
// the second chunk contains the remaining bytes. 
// You must check that the size of the original chunk is big enough to enable such a cut.
void
split(header_t *original, size_t csz)
{
	//Your code here
	if (original->size > csz){
		header_t *second_chunk = original+csz/16;
		second_chunk->size = (original -> size) - csz;
		second_chunk->allocated = false;
		original->size = csz;
	}
}

// helper function ask_os_for_chunk invokes the mem_sbrk function to ask for a chunk of 
// memory (of size csz) from the "operating system". It initializes the new chunk 
// using helper function init_chunk and returns the initialized chunk.
header_t *
ask_os_for_chunk(size_t csz)
{
	header_t *brk = mem_sbrk(csz);
	brk->allocated = false;
	brk->size = csz;
	return brk;
}

/* 
 * mm_malloc allocates a memory block of size bytes
 */
void *
mm_malloc(size_t size)
{
	//make requested payload size aligned
	size = align(size);
	//chunk size is aligned because both payload and header sizes
	//are aligned
	size_t csz = hdr_size + align(size);
	header_t *p = NULL;

	//Your code here 
	//to obtain a free chunk p to satisfy this request.
	//
	//The code logic should be:
	//Try to find a free chunk using helper function first_fit
	//    If found, split the chunk (using helper function split).
	//    If not found, ask OS for new memory using helper ask_os_for_chunk
	//Set the chunk's status to be allocated
	if (first_fit(csz) != NULL){
		//printf("in a");
		p = first_fit(csz);
		if(p -> size > csz){
			split(p, csz);
		}	
		
	}else{
		//printf("in b");
		p = ask_os_for_chunk(csz);
	}

	//After finishing obtaining free chunk p, 
	//check heap correctness to catch bugs
	p -> allocated = true;
	p = (void *)((char *)p+hdr_size);
	if (debug) {
		mm_checkheap(true);
	}
	return (void *)p;
}

// Helper function payload_to_header returns a pointer to the 
// chunk header given a pointer to the payload of the chunk 
header_t *
payload2header(void *p)
{
	//Your code here
	return (header_t *)((char *)p-hdr_size);
}

// Helper function coalesce merges free chunk h with subsequent 
// consecutive free chunks to become one large free chunk.
// You should use next_chunk when implementing this function
void
coalesce(header_t *h)
{
	//Your code here
	//header_t *f = h
	if (h->allocated == false){
		size_t add_up_size = h -> size;
		while (next_chunk(h) != NULL && next_chunk(h) -> allocated == false)
		{
		add_up_size += next_chunk(h) -> size;
		//next_chunk(h) -> allocated = 0;
		//next_chunk(h) -> size = 0;
		h -> size = add_up_size;
		}
	}
	
}

/*
 * mm_free frees the previously allocated memory block
 */
void 
mm_free(void *p)
{
	// Your code here
	// 
	// The code logic should be:
	// Obtain pointer to current chunk using helper payload_to_header 
	// Set current chunk status to "free"
	// Call coalesce() to merge current chunk with subsequent free chunks
	//printf("in c");
	header_t *h = payload2header(p);
	h -> allocated = false;
	coalesce(h);
	// After freeing the chunk, check heap correctness to catch bugs
	if (debug) {
		mm_checkheap(true);
	}
}	

/*
 * mm_realloc changes the size of the memory block pointed to by ptr to size bytes.  
 * The contents will be unchanged in the range from the start of the region up to the minimum of   
 * the  old  and  new sizes.  If the new size is larger than the old size, the added memory will   
 * not be initialized.  If ptr is NULL, then the call is equivalent  to  malloc(size).
 * if size is equal to zero, and ptr is not NULL, then the call is equivalent to free(ptr).
 */
void *
mm_realloc(void *ptr, size_t size)
{
	// Your code here
	size = align(size);
	size_t csz = hdr_size + align(size);
	header_t *p = payload2header(ptr);
	//header_t *t = p;
	//printf("realloc");
	if (ptr == NULL){
		return mm_malloc(size);
	}else{
		if (size == 0){
			mm_free(ptr);
		}else{
			p -> allocated = false;
			coalesce(p);
			if (csz < p->size){
				//printf("in d");
				split(p,csz);
				p -> allocated = true;
				return 	(void *)((char *)p+hdr_size);
			}else{
				//printf("in f");
				header_t *dst = mm_malloc(csz-hdr_size);
				p = memcpy((void *)dst, ptr, csz-hdr_size);
				return (void *)p;
			}
		}
		
	}
	// Check heap correctness after realloc to catch bugs
	if (debug) {
		mm_checkheap(true);
	}
	return NULL;

}


/*
 * mm_checkheap checks the integrity of the heap and returns a struct containing 
 * basic statistics about the heap. Please use helper function next_chunk when 
 * traversing the heap
 */
heap_info_t 
mm_checkheap(bool verbose) 
{
	heap_info_t info;
	// Your code here
	// 
	// traverse the heap to fill in the fields of info
	info.num_allocated_chunks=0;
	info.num_free_chunks=0;
	info.allocated_size=0;
	info.free_size=0;
	header_t *header = NULL;
	while (next_chunk(header) != NULL){
		if (next_chunk(header)->allocated==true){
			info.num_allocated_chunks += 1;
			info.allocated_size+=next_chunk(header)->size;
		}else{
			info.num_free_chunks+=1;
			info.free_size+=next_chunk(header)->size;
		}
		header=next_chunk(header);
	}
	if (verbose){
		printf("\nNumber of allocated chunks: %ld\n", (long)info.num_allocated_chunks);
		printf("Number of free chunks: %ld\n", (long)info.num_free_chunks);
		printf("Allocated size: %ld\n", (long)info.allocated_size);
		printf("Free size: %ld\n", (long)info.free_size);
	}
	// correctness of implicit heap amounts to the following assertion.
	assert(mem_heapsize() == (info.allocated_size + info.free_size));
	return info;
}
