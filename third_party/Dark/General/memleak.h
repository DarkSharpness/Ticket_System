#ifndef _DARK_MEMINFO_H_
#define _DARK_MEMINFO_H_

#ifdef WIN64

#include <iostream>


/* A simple tool to test memory leak and get storage info. */
struct memory_recorder { 
    static std::size_t allocated;
    static std::size_t deallocated;
    static std::size_t allocatedNew;
    static std::size_t ptr_count;
    ~memory_recorder() {
        printf("\n------------------------------\n");
        printf("Space allocated : %lu\n",allocated);
        printf("Pointers leaked : %lu\n",ptr_count);
        printf("Space leaked from new only : %lu\n",
                         allocatedNew - deallocated);
        printf( "------------------------------\n" );
    }
};

static memory_recorder MEMORY_RECORDER;

std::size_t memory_recorder::allocated    = 0;
std::size_t memory_recorder::ptr_count    = 0;
std::size_t memory_recorder::deallocated  = 0;
std::size_t memory_recorder::allocatedNew = 0;

void *operator new (size_t sz) {
    void *p = std::malloc(sz);
    memory_recorder::allocated += sz;
    memory_recorder::allocatedNew += sz;
    ++memory_recorder::ptr_count;
    return p;
}

void *operator new [](size_t sz) {
    void *p = std::malloc(sz);
    memory_recorder::allocated += sz;
    ++memory_recorder::ptr_count;
    return p;
}

void operator delete(void* p,std::size_t sz) {
    if(p != nullptr) {
        memory_recorder::deallocated += sz;
        --memory_recorder::ptr_count;
    }
    return std::free(p);
}

void operator delete[](void *p) {
    if(p != nullptr) { --memory_recorder::ptr_count; }
    return std::free(p);
}


#endif

#endif
