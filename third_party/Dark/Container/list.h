#ifndef _DARK_LIST_H_
#define _DARK_LIST_H_

#include <utility>

namespace dark {


/* Declaration part. */
namespace list {

struct node_base;
struct forward_tag {};
using baseptr = node_base *;

void delink(baseptr __p);
void link_after (baseptr __p,baseptr __c);
void link_before(baseptr __s,baseptr __c);

}

/* Struct part. */
namespace list {

struct node_base {
    baseptr prev; /* Pointer to prev node in the list. */
    baseptr next; /* Pointer to next node in the list. */
};

template <class value_t>
struct node : node_base {
    value_t data; /* Real data. */
    template <class ...Args>
    node(forward_tag,Args &&...objs) : data(std::forward <Args> (objs)...) {} 
};

}

namespace list {

/**
 * @brief Delink one node from the list.
 * 
 * @param __p The node to delink.
 */
void delink(baseptr __p) {
    __p->prev->next = __p->next;
    __p->next->prev = __p->prev;
}

/**
 * @brief Link a node into the list after certain node. 
 * 
 * @param __p The node to be linked after.
 * @param __c The node to be linked into.
 */
void link_after(baseptr __p,baseptr __c) {
    __p->next->prev = __c;
    __c->next = __p->next;
    __c->prev = __p;
    __p->next = __c;
}

/**
 * @brief Link a node into the list after certain node. 
 * 
 * @param __p The node to be linked before.
 * @param __c The node to be linked into.
 */
void link_before(baseptr __s,baseptr __c) {
    __s->prev->next = __c;
    __c->prev = __s->prev;
    __c->next = __s;
    __s->prev =__c;
}

}


}


#endif
