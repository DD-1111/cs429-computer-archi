#include "heap.h"
#include "debug.h"
#include "stdint.h"
#include "atomic.h"

/* This implement of heap is based on lectures and some ideas in the 
textbook of CS429, Computer Systems, A Programmer’s Perspective
chapter 9.4 */

/* Basic constants and macros*/

/* Word and header/footer size (bytes) */
#define WSIZE     4     
#define DSIZE     8   
/* rounds up to the next multiple of ALIGNMENT */
#define ALIGNMENT 4
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x3)





template<typename T>
inline T abs(T v) {
    return (v < 0) ? -v : v;
}

template<typename T>
inline T* ptr_add(int32_t* ptr, int32_t value) {
     return (T*) (((uintptr_t) ptr) + value);
}

inline int32_t* get_right_header(int32_t* header) {
     return ptr_add(header, abs(*header) + 8);
}

// header of the free block
struct Header {
	// size in bytes, > 0 if allocated, <0 if freed.
	int size; 
    
	Header* prevFreeNode;
	Header* nextFreeNode;

	// is this block free?
	inline bool isFree() {
		return size < 0;
	}

	// return a pointer to the header of next node
	inline Header* nextP() {
		return ptr_add<Header>(this, abs<int>(size));
	}

	inline Header* prevP() {
        Footer prevBlock = ptr_add<Footer>(this, -WSIZE);
		int prevSize = abs(prevBlock -> size);
		return ptr_add<Header>(this, -prevSize);
	}

	// get the Footer of this node
	inline Footer* getFooter() {
		return ptr_add<Footer>(nextP(), -WSIZE);
	}

    inline size_t absSize() {
        return abs(size);
    }

    inline void changeState() {
        size *= -1;
        getFooter() -> size *= -1;
    }
};

struct Footer {
	// nothing but size here
	int size;
};



/* Pointer to the head of the linked list */   
Header* headp = nullptr;
/* Pointer to the first byte of the first block in the heap*/ 
static void *heap_listp = nullptr; 
static size_t heap_size;



/*
* mem_sbrk - Simple model of the sbrk function from the book. Extends the heap
* by incr bytes and returns the start address of the new area. In
* this model, the heap cannot be shrunk.
*/


static void *coalesce(void *bp);


// to set a block using parameters 
Header setBlock(Header* hp, size_t bSize, Header next, Header prev) {
    hp -> size = bSize; 
    hp -> getFooter() ->size = hp -> size;
    hp -> prevFreeNode = prev;
    hp -> nextFreeNode = next;
}

Header setBlockSize(Header* hp, size_t bSize) {
    hp ->size = bSize;
    hp -> getFooter() ->size = hp -> size;
}

void heapInit(void* start, size_t bytes) {
    
    heap_size = bytes;
    //create first initial empty heap
    heap_listp = start;
    headp = (Header*) heap_listp;
    // initialize the whole empty heap as one large free block.
    setBlock(headp, -ALIGN(bytes), nullptr, nullptr); 
}


void add_to_head(Header *tgt) {
    if (headp != nullptr) {
       tgt ->nextFreeNode = headp;
       } else {
           tgt ->nextFreeNode = nullptr;
       }
    tgt ->prevFreeNode = nullptr;
    headp = tgt;
    tgt ->nextFreeNode ->prevFreeNode = tgt;
}

static void remove_node(Header *tgt) {   
    Header* next = tgt ->nextFreeNode;
    Header* prev = tgt ->prevFreeNode;
    /*At the beginning, no prev node
    if next exists, set next's prev to NULL*/
    if (prev == nullptr && next != nullptr) { 
        headp = tgt -> nextFreeNode;
        tgt -> nextFreeNode ->prevFreeNode = nullptr;
    // both exist
    } else if(prev != nullptr && next != nullptr) {
        tgt -> prevFreeNode -> nextFreeNode = tgt -> nextFreeNode;
		tgt -> nextFreeNode -> prevFreeNode = tgt -> prevFreeNode;
    } else if(prev == nullptr && next == nullptr) {
        headp = nullptr;
    } else {
        tgt -> prevFreeNode -> nextFreeNode = nullptr;
    }
    Debug::printf("target at %x removed\n",tgt);
}


// static void *coalesce(Header *bp)
// {
//     bool prev_alloc = !(bp ->prevP()->isFree());
//     bool next_alloc = !(bp ->nextP()->isFree());
//     size_t size = bp->absSize();
//     size_t next_size = bp->nextP()->absSize();
//     size_t prev_size =  bp->prevP()->absSize();
//     //case 1
//     if (prev_alloc && next_alloc) {                       

//     //case 2: remove middle node from 3 successing blocks
//     //insert the new merged block at the head of the link list                   
//     } else if (prev_alloc && !next_alloc) {
//         size += add_next_size;
//         remove_node(NEXT_BLKP(bp));
//         PUT(HDRP(bp), PACK(size, 0));
//         PUT(FTRP(bp), PACK(size, 0));

//     //case 3: adjacent to the head of list
//     } else if (!prev_alloc && next_alloc) {
//         size += add_prev_size;
//         remove_node(PREV_BLKP(bp));
//         PUT(FTRP(bp), PACK(size, 0));
//         PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//         bp = PREV_BLKP(bp);

//     //case 4: adjacent to two empty block
//     } else {
//         size += add_prev_size;
//         size += add_next_size;
//         remove_node(PREV_BLKP(bp));
//         remove_node(NEXT_BLKP(bp));
//         PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//         PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
//         bp = PREV_BLKP(bp);
//     }

//     add_to_head((char*)bp);
//     return bp;
// }



// first-fit 
void* malloc(size_t bytes) {
    size_t asize;
    char *bp;
    if(bytes == 0) 
        return heap_listp;
    if(bytes < 8) {
        asize = 8;
    }
    asize = ALIGN(bytes);
    Header* tempP = headp;
    while (tempP != nullptr)
    {  
        int tempSize = tempP ->absSize();
        // We got enough free space 
        if (asize <= tempSize - DSIZE) {
            // if the required size take most part of temp node, just give all of temp node 
            // since we don't have enough space to form new node
            if (asize >= tempSize - 3*DSIZE) {
                tempP -> changeState();
                remove_node(tempP);
                return ptr_add<void>(tempP, WSIZE);
            } else
            // split and allocate only a part of it
            {
                setBlockSize(tempP, -(tempSize - asize - DSIZE));
                setBlockSize(tempP->nextP(), asize + DSIZE)
                return ptr_add<void>(tempP->nextP(), WSIZE);
            }
        }
    // iterate the temp node
    tempP = tempP ->nextFreeNode;
    }
    Debug::printf("malloc failed\n");
    return nullptr;
}

void mergeRight(Header* ptr) {
	// 0. remember the Header of the node on the right
	Header* headerOfRightNode = ptr -> nextP();
	// 1. change the size of current node
	int signedSizeAfterMerge = (ptr -> size) + (headerOfRightNode -> size);
	ptr -> size = signedSizeAfterMerge;
	ptr -> getFooter() -> size = signedSizeAfterMerge;
	// 2. remove the right free node from the free node list
	remove_node(headerOfRightNode);
}

void free(void* p) {
    if(p != nullptr && (p < ((char*)heap_listp + heap_size))) {
    Header* freeHp = ptr_add<Header>(p, -WSIZE);
    size_t curSize = freeHp ->size;
    if (curSize < 0) {
        Debug::printf("try to free a block with size of negative, which means the sign of size is not correct\n");
    }
    freeHp ->changeState();
    add_to_head(freeHp);
  //  coalesce(p);
      // 3. merge with right node if there is a right node AND it is also free
    if ((char*) (freeHp -> nextP()) < (((char*) heap_listp) + heap_size) && freeHp -> nextP() -> isFree()) {
		// Debug::printf("merge right\n");
		mergeRight(freeHp);
    }  
    // 4. merge with left node if there is a left node AND it is also free
    if (((char*) freeHp) > ((char*) heap_listp) && freeHp -> prevP() -> isFree()) {
    	// Debug::printf("merge left!\n");
		mergeRight(freeHp -> hopLeft());
    }
    }
    return p;
}


/*****************/
/* C++ operators */
/*****************/

void* operator new(size_t size) {
    void* p =  malloc(size);
    if (p == 0) Debug::panic("out of memory!!");
    return p;
}

void operator delete(void* p) noexcept {
    return free(p);
}

void operator delete(void* p, size_t sz) {
    return free(p);
}

void* operator new[](size_t size) {
    void* p =  malloc(size);
    if (p == 0) Debug::panic("out of memory!!!");
    return p;
}

void operator delete[](void* p) noexcept {
    return free(p);
}

void operator delete[](void* p, size_t sz) {
    return free(p);
}
