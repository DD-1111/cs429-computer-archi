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

// some function rom piazza
template<typename T>
inline T abs(T v) {
    return (v < 0) ? -v : v;
}

template<typename T>
inline T* ptr_add(void* ptr, int value) {
     return (T*) (((char*) ptr) + value);
}



// Actually a just a int stored, to formalize the structure of explicit free node list.
// nothing but size here
struct Footer {

    int size;
};

// header of the free block
struct Header {
	// size in bytes, > 0 if allocated, < 0 if freed.
	int size; 
    
	Header* prevFreeNode;
	Header* nextFreeNode;

	// is this block free?
	inline bool isFree() {
		return size < 0;
	}

    inline size_t absSize() {
        return abs(size);
    }

	// return a pointer to the header of next node
	inline Header* nextP() {
		return ptr_add<Header>(this, abs<int>(size));
	}

	inline Header* prevP() {
        Footer* prevBlock = ptr_add<Footer>(this, -WSIZE);
		int prevSize = abs(prevBlock -> size);
		return ptr_add<Header>(this, -prevSize);
	}

	// get the Footer of this node
	inline Footer* getFooter() {
		return ptr_add<Footer>(nextP(), -WSIZE);
	}

    inline void changeState() {
        size *= -1;
        getFooter() -> size *= -1;
    }
    
};


// to set a block using parameters 
void setBlock(Header* hp, size_t bSize, Header* next, Header* prev) {
    hp -> size = bSize; 
    hp -> getFooter() ->size = hp -> size;
    hp -> prevFreeNode = prev;
    hp -> nextFreeNode = next;
}

void setBlockSize(Header* hp, size_t bSize) {
    hp ->size = bSize;
    hp -> getFooter() ->size = hp -> size;
}

/* Pointer to the head of the linked list */   
Header* headp = nullptr;
/* Pointer to the first byte of the first block in the heap*/ 
static void *heap_listp = nullptr; 
static size_t heap_size;
SpinLock aLock;

void heapInit(void* start, size_t bytes) {
    
    heap_size = bytes;
    //create first initial empty heap
    heap_listp = start;
    headp = (Header*) heap_listp;
    // initialize the whole empty heap as one large free block.
    setBlock(headp, -ALIGN(bytes), nullptr, nullptr); 
}


bool inRange (void* p) {

    if (((char*)p > (char*)heap_listp - 30) 
    && ((char*)p < ((char*)heap_listp + heap_size + 30))) {
    return true;
    } 
    return false;
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
    // Debug::printf("target at %d removed\n",tgt);
}

// void remove_node (Header* ptr) {
// 	if (ptr -> prevFreeNode == nullptr && ptr -> nextFreeNode == nullptr) {
// 		// Debug::printf("remove first (and only) node in FNL!\n");
// 		headp = nullptr;
// 	} else if (ptr -> prevFreeNode == nullptr && ptr -> nextFreeNode != nullptr) {
// 		// Debug::printf("remove first node in FNL!\n");
// 		headp = ptr -> nextFreeNode;
// 		ptr -> nextFreeNode -> prevFreeNode = nullptr;
// 	} else if (ptr -> prevFreeNode != nullptr && ptr -> nextFreeNode == nullptr) {
// 		// Debug::printf("remove last node in FNL!\n");
// 		ptr -> prevFreeNode -> nextFreeNode = nullptr;
// 	} else {
// 		// Debug::printf("remove a middle node in FNL!\n");
// 		ptr -> prevFreeNode -> nextFreeNode = ptr -> nextFreeNode;
// 		ptr -> nextFreeNode -> prevFreeNode = ptr -> prevFreeNode;
// 	}
// }


void* malloc(size_t bytes) {
    aLock.lock();
    size_t asize;
    if(bytes == 0) 
        return heap_listp;
    if(bytes < 8) {
        asize = 8;
    }
    asize = ALIGN(bytes);
    Header* tempP = headp;
    while (tempP != nullptr)
    {  
        //if (inRange((void*)tempP)) {
        size_t tempSize = tempP ->absSize();
        //Debug::printf("temp size = %d\n", tempSize);
        // We got enough free space 
        if (asize <= tempSize - DSIZE) {
           // Debug::printf(" enough place found ");
            // if the required size take most part of temp node, just give all of temp node 
            // since we don't have enough space to form new node
            if (asize >= tempSize - 3*DSIZE) {
                // Debug::printf("give whole block");
                tempP -> changeState();
                remove_node(tempP);
                aLock.unlock();
                return ptr_add<void>(tempP, WSIZE);
            } else
            // split and allocate only a part of it
            {
                setBlockSize(tempP, -(tempSize - asize - DSIZE));
                // Debug::printf("tempP size = %d\n", tempP ->size);
                // Debug::printf("tempP isfree = %d\n", tempP ->isFree());
                setBlockSize(tempP->nextP(), asize + DSIZE);
                // Debug::printf("tempP right size = %d\n", tempP->nextP() ->size);
                // Debug::printf("tempP right isfree = %d\n", tempP->nextP() ->isFree());
                aLock.unlock();
                return ptr_add<void>(tempP -> nextP(), WSIZE);
            }
        }
        //}
    // iterate the temp node
        tempP = tempP ->nextFreeNode;
    }
    aLock.unlock();
    //Debug::printf("malloc failed\n");
    return nullptr;
}


// void* malloc(size_t bytes) {

// 	// don't even bother to check anything if requested memory is > heap size.
// 	if ((int) bytes > (int)heap_size - 8 || (int) bytes < 0) {
// 		// Debug::printf("I don' have that much memory left. sorry!\n");
// 		return nullptr;
// 	}
// 	// return one past the last byte in heap if malloc(0)
// 	if (bytes == 0) {
// 		return (void*) (((char*) heap_listp) + heap_size);
// 	}


// 	aLock.lock();
// 	int requestedMemInWords = ALIGN(bytes);

// 	Header* freeNodePtr = headp;

// 	while (freeNodePtr != nullptr) {
// 		// good, we find a free node to host the request
// 		if (requestedMemInWords <= abs(freeNodePtr -> size) - DSIZE) {
// 			// however, if requested memory is large enough so that the rest will be too small
// 			// to be a free node (i.e. only 3 words or less is left), then we just allocate
// 			// the whole node to the user
// 			if (requestedMemInWords >= abs(freeNodePtr -> size) - 3*DSIZE) {
// 				// 1. negate the size sign in both Header and Footer
// 				freeNodePtr -> size *= -1;
// 				freeNodePtr -> getFooter() -> size *= -1;
// 				// 2. remove this node from free node list
// 				remove_node(freeNodePtr);
// 				// 3. return a pointer to the start of block in this node

// 				// if (!isValid(heap_listp, heap_size)) {
// 			 //    	Debug::printf("something wrong after malloc size = %d node! (no split case)\n", byte2WordRoundUp(bytes,4) + 2);
// 				// 	traverseAllNodes();
// 				// 	traverseFNL();
// 			 //    	Debug::shutdown();
// 			 //    } else {
// 			 //    	Debug::printf("heap good after malloc size = %d node!\n", byte2WordRoundUp(bytes,4) + 2);
// 			 //    	traverseAllNodes();
// 			 //    	traverseFNL();
// 			 //    }

// 				aLock.unlock();
// 				return ptr_add<void>(freeNodePtr, 4);
// 			} else {
// 				// otherwise, split this free node
// 				// 1. change the size in the remaining free node (on the left) and
// 				// the newly-allocated taken node (on the right)
// 				freeNodePtr -> size = -(abs(freeNodePtr -> size) - requestedMemInWords - DSIZE);
// 				freeNodePtr -> getFooter() -> size = freeNodePtr -> size;
// 				freeNodePtr -> nextP() -> size = requestedMemInWords + DSIZE;
// 				freeNodePtr -> nextP() -> getFooter() -> size = requestedMemInWords + DSIZE;
// 				aLock.unlock();
// 				return ptr_add<void>(freeNodePtr -> nextP(), 4);
// 			}
// 			Debug::printf("This should not happen! Something is wrong in malloc!\n");
// 		}
// 		// this one isn't large enough, go try the next free node
// 		freeNodePtr = freeNodePtr -> nextFreeNode;		
// 	}

// 	// Debug::printf("Unable to find a large enough free node to host your request. Sorry!\n");
// 	aLock.unlock();
// 	// cannot find any eligible free node to allocate memory
// 	return nullptr;
// }

// merge and change the pointer
void merge(Header* tgt) {
	Header* rightP = tgt -> nextP();
	int aSize = (tgt -> size) + (rightP -> size);
    setBlockSize(tgt, aSize);
	remove_node(rightP);
}

void coalesce(Header *bp) {
    if (inRange(bp)){
        // merge right first
        if (bp -> nextP() -> isFree()) {
            merge(bp);
        }
        // then left and change the head pointer 
        if (bp -> prevP() -> isFree()) {
            merge(bp -> prevP());
        }
    }
}

void free(void* p) {
    aLock.lock();
    if (p != nullptr && inRange(p)) {
    Header* freeHp = ptr_add<Header>(p, -WSIZE);
    size_t curSize = freeHp ->size;
    if (curSize < 0) {
        //Debug::printf("try to free a block with size of negative, which means the sign of size is not correct\n");
    }
    //free this block first
    freeHp ->changeState();
    add_to_head(freeHp);
    coalesce(freeHp);
    }
    aLock.unlock();
}



void showleft(void* p) {
    Header* a = (Header*) p;
    Debug::printf("curSize = %d\n", a -> size);
    Debug::printf("curisfree = %d\n", a -> isFree());
    Debug::printf("curpos = %d\n", a);
    Debug::printf("left size = %d\n", a ->prevP() -> size);
    Debug::printf("left isfree = %d\n", a -> prevP() -> isFree());
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
