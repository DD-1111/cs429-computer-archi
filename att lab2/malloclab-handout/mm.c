
/* Implement explict list instead of implicit list like the example
given in the book. Use a lot of code from the book's example and solutions
to the practice problem. Only several changes are made to make it become 
a explicit list allocator with a much higher efficiency.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


/* Implement explict list */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the  nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define DSIZEALIGN(size) (DSIZE)*((size+(DSIZE)+(DSIZE-1)) / (DSIZE))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* use example variables as the book does*/
static char *heap_listp = NULL;  /* Pointer to first block */  


static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t size);
static void place(void *bp,size_t asize);
void add_to_head(char *tgt);
static void remove_node(char *tgt);



static char *headp = NULL;  /* Pointer to the head of the linked list */  


/* Basic constants and macros in the textbook*/
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                  
#define GET_ALLOC(p) (GET(p) & 0x1)                    

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                    
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) 
#define GET_PREVRP(bp) ((char *)(bp))
#define GET_NEXTRP(bp) ((char *)(bp) + WSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


int mm_init(void) {
    //different from what we got in the book
    //add linknode prev/next to the book version
    if((heap_listp = mem_sbrk(6 * WSIZE)) == (void *)-1) 
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), 0);// prev free node
    PUT(heap_listp + (2*WSIZE), 0);// next free node 
    PUT(heap_listp + (3*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (4*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (5*WSIZE), PACK(0, 1));
    heap_listp += (4*WSIZE);
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL) 
        return -1;
    return 0;
}

static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    // Use even times of Dsize to make sure we have 4 words
    size = (words % 2) ? (words + 1) * DSIZE : words * DSIZE;
    if((long)(bp = mem_sbrk(size)) == -1) 
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    // Free two linknodes
    PUT(GET_NEXTRP(bp), 0);
    PUT(GET_PREVRP(bp), 0);
    // New epilogue header
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 * Always allocate a block whose size is a multiple of the alignment.
 * OG code from the book
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    if(size == 0) 
        return NULL;
    if(size <= DSIZE) {
        asize = 2*(DSIZE);
    } else {
        asize = DSIZEALIGN(size);
    }
    if((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL){
        return NULL;
    }
    place(bp, asize);
    return bp;
}


//simple first fit 
static void *find_fit(size_t asize) 
{
    char *temp = GET(headp);
    while (temp != NULL) {
        if (GET_SIZE(HDRP(temp)) >= asize) 
            return temp;
        temp = GET(GET_NEXTRP(temp));
    }
    return NULL;
}

// modified code from the book
static void place(void *bp, size_t asize){
    size_t csize = GET_SIZE(HDRP(bp));
    remove_node(bp);
    if((csize - asize) < 2*DSIZE) {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    } else {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        // also erase next and prev
        PUT(GET_NEXTRP(bp), 0);
        PUT(GET_PREVRP(bp), 0);
        coalesce(bp);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    if(!bp) 
        return;
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(GET_NEXTRP(bp),0);
    PUT(GET_PREVRP(bp),0);
    coalesce(bp);
}

/* OG code from book */
void *mm_realloc(void *ptr, size_t size) {
    size_t oldsize;
    void *newptr;
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }
    
    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

// modified coalesece code from the book
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    size_t add_next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
    size_t add_prev_size = GET_SIZE(HDRP(PREV_BLKP(bp)));
    //case 1
    if (prev_alloc && next_alloc) {                       

    //case 2: remove middle node from 3 successing blocks
    //insert the new merged block at the head of the link list                   
    } else if (prev_alloc && !next_alloc) {
        size += add_next_size;
        remove_node(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

    //case 3: adjacent to the head of list
    } else if (!prev_alloc && next_alloc) {
        size += add_prev_size;
        remove_node(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

    //case 4: adjacent to two empty block
    } else {
        size += add_prev_size;
        size += add_next_size;
        remove_node(PREV_BLKP(bp));
        remove_node(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    add_to_head(bp);
    return bp;
}

// helper method to remove nodes considerately from virtual linked list
static void remove_node(char *tgt) {   
    char* next = GET(GET_NEXTRP(tgt));
    char* prev = GET(GET_PREVRP(tgt));
    /*At the beginning, no prev node
    if next exists, set next's prev to NULL*/
    if (prev == NULL && next != NULL) { 
        PUT(GET_PREVRP(next), 0);
        PUT(headp, next);
    // both exist
    } else if(prev != NULL && next != NULL) {
        PUT(GET_PREVRP(next),prev);
        PUT(GET_NEXTRP(prev), next);
    } else if(prev == NULL && next == NULL) {
        PUT(headp, next);
    } else {
      PUT(GET_NEXTRP(prev), next);
    }
    // remove from emptylist
    PUT(GET_PREVRP(tgt), 0);
    PUT(GET_NEXTRP(tgt), 0);
}

void add_to_head(char *tgt) {
    // LIFO 
    char *nextp = GET(headp);
    PUT(headp, tgt);
    if (nextp != NULL)
       PUT(GET_PREVRP(nextp), tgt);
    PUT(GET_NEXTRP(tgt), nextp);
}



