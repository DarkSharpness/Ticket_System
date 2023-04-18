#ifndef _DARK_ALLOCATOR_H_
#define _DARK_ALLOCATOR_H_

#include <cstdlib>
#include <memory>

/* Easy wrapping. */
namespace dark {

/**
 * @brief A simple implement for node allocator.
 * Utilizing EBO (Empty Base Optimization),
 * this can greatly compress the space,
 * because allocators, compare functions are
 * empty classes in most cases.
 * 
*/
template 
<
class value_t,
class allocator_t = std::allocator <value_t>,
class ...Ts
>
struct implement : allocator_t , Ts ... {
    using pointer = value_t *;
    size_t count = 0; /* Default as 0. User should manage it. */

    /* Allocate just one pointer. */
    template <class ...Args>
    inline pointer alloc(Args &&...objs) {
        pointer __p = this->allocate(1);
        this->construct(__p,std::forward <Args> (objs)...);
        return __p;
    }

    /* Deallocate just one pointer. */
    inline void dealloc(pointer __p) {
        this->destroy(__p);
        this->deallocate(__p,1);
    }
};
}


#endif
