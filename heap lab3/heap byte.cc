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

    inline int absSize() {
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
void setBlock(Header* hp, int bSize, Header* next, Header* prev) {
    hp -> size = bSize; 
    hp -> getFooter() ->size = hp -> size;
    hp -> prevFreeNode = prev;
    hp -> nextFreeNode = next;
}

void setBlockSize(Header* hp, int bSize) {
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

bool inRange (void* p) {
    if (((char*)p > (char*)heap_listp) 
    && ((char*)p < ((char*)heap_listp + heap_size))) {
    return true;
    } 
    return false;
}

void* malloc(size_t bytes) {
    aLock.lock();
    int asize;
    if((int) bytes > heap_size - 8 || (int) bytes < 0)) return nullptr;
    if(bytes == 0) 
        return heap_listp;
    asize = ALIGN(bytes);
   // if (asize < 8) asize = 8;
    Header* tempP = headp;
    while (tempP != nullptr)
    {  
        int tempSize = tempP ->absSize();
        //Debug::printf("temp size = %d\n", tempSize);
        // We got enough free space 
        if (asize <= tempSize - DSIZE) {
           // Debug::printf(" enough place found ");
            // if the required size take most part of temp node, just give all of temp node 
            // since we don't have enough space to form new node
            if (asize >= tempSize - 4*WSIZE) {
                // Debug::printf("give whole block");
                tempP -> changeState();
                remove_node(tempP);
                if (!inRange(tempP) && tempP -> size != 0) {
                    Debug::printf("not in range");
                }
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
                // if (!inRange(tempP) && tempP -> size != 0) {
                //     Debug::printf("not in range");
                // }
                aLock.unlock();
                return ptr_add<void>(tempP -> nextP(), WSIZE);
            }
        }
    // iterate the temp node
        tempP = tempP ->nextFreeNode;
    }
    aLock.unlock();
    //Debug::printf("malloc failed\n");
    return nullptr;
}

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
    //free this block first
    freeHp ->changeState();
    add_to_head(freeHp);
    coalesce(freeHp);
    }
    aLock.unlock();
}

/*
void showleft() {
    Debug::printf("curSize = %d\n", headp -> size);
    Debug::printf("curisfree = %d\n", headp -> isFree());
    Debug::printf("left size = %d\n", headp ->prevP() -> size);
    Debug::printf("left isfree = %d\n", headp -> prevP() -> isFree());
}
*/

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
