#ifndef _DARK_HASH_H_
#define _DARK_HASH_H_

#include "list.h"

#include <utility>


namespace dark {

namespace hash {

struct node_base;
struct forward_tag {};
using baseptr = node_base *;

}

namespace hash {

struct node_base {
    baseptr real = nullptr; /* pointer to real data in the hash-map. */
};

template <class value_t>
struct node : node_base , list::node_base {
    value_t data; /* Real data. */
    template <class ...Args>
    node(forward_tag,Args &&...objs) : data(std::forward <Args> (objs)...) {} 
};

}

}


#endif
