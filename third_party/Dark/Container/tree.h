#ifndef _DARK_TREE_H_
#define _DARK_TREE_H_

#include "allocator.h"

#include <utility>

namespace dark {

/* Header part for BW-Tree. */
namespace tree {


/* BW_Tree color */
enum struct Color : bool { BLACK = 0 , WHITE = 1 };


struct node_base;


inline bool is_root(const node_base *__p) noexcept;


template <bool dir>
[[deprecated]] inline bool is_dir (const node_base *__p) noexcept;


template <bool dir>
void advance(node_base *&__p) noexcept;

template <bool dir>
void advance(const node_base *&__p) noexcept;


/* A tag marking perfect forwarding. */
struct forward_tag {};

}


/* Node-related part. */
namespace tree {


/* BW_Tree node base */
struct node_base {
    using baseptr = node_base *;
    Color   color;
    baseptr parent;
    baseptr son[2];

    /* Return the direction of this branch. */
    inline bool dir() const noexcept { return this != parent->son[0]; }
 
    /* Whether this is on the __d branch of parent.  */
    inline bool dir(bool __d) const noexcept { return this == parent->son[__d]; }

    /* Return the brother node. It requires existence of parent node. */
    inline baseptr bro() const noexcept { return this->parent->son[dir(0)]; }

    /* Initialize by setting to null or given pointer. */
    node_base(baseptr __p = nullptr) noexcept
        : color(Color::WHITE),parent(__p) { son[0] = son[1] = __p; }
};


/* BW_Tree node for given value_t(pair <key_t,T>). */
template <class value_t>
struct node : public node_base {
    value_t data;
    node() = default;

    template <class ...Args>
    node(forward_tag,Args &&...objs) : data(std::forward <Args> (objs)...) {} 

};


/**
 * @brief The base for iterator of BW_Tree.
 * 
 * @tparam is_const Whether it's a const iterator. 
 */
template <class value_t,bool is_const,bool dir>
struct iterator_base {
  protected:
    using U = std::conditional_t <is_const,const node_base,node_base>;
    using V = std::conditional_t <is_const,const node <value_t>,node <value_t>>;
  public:
    using pointer = U *;

  protected:
    pointer ptr;

  public:

    iterator_base(pointer __p = nullptr) noexcept : ptr(__p) {}

    iterator_base(const iterator_base <value_t,false,dir> &rhs) 
    noexcept : ptr(rhs.base()) {}

    iterator_base & operator = (iterator_base <value_t,false,dir> rhs) 
    noexcept { ptr = rhs.base(); return *this; }

    iterator_base &operator ++ () noexcept
    { advance <dir>  (ptr); return *this; }

    iterator_base &operator -- () noexcept
    { advance <!dir> (ptr); return *this; }

    iterator_base operator ++ (int) noexcept
    { auto temp = *this; advance <dir>  (ptr); return temp; }

    iterator_base operator -- (int) noexcept
    { auto temp = *this; advance <!dir> (ptr); return temp; }

    value_t &operator * (void) const { return  ((V *)ptr)->data; }
    value_t *operator ->(void) const { return &((V *)ptr)->data; }

    pointer base() const noexcept { return ptr; }
};


}


/* Function part for BW-Tree. */
namespace tree {


/* Compare 2 iterator. */
template <class T,bool k1,bool k2,bool dir>
bool operator == (const iterator_base <T,k1,dir> &lhs,
                  const iterator_base <T,k2,dir> &rhs)
noexcept { return lhs.base() == rhs.base(); }

/* Compare 2 iterator. */
template <class T,bool k1,bool k2,bool dir>
bool operator != (const iterator_base <T,k1,dir> &lhs,
                  const iterator_base <T,k2,dir> &rhs)
noexcept { return lhs.base() != rhs.base(); }


/* Whether the node is root/header node or not. */
inline bool is_root(const node_base *__p) noexcept 
{ return __p->parent->parent == __p; }


/* Whether the node is header node or not. */
inline bool is_header(const node_base *__p) noexcept 
{ return is_root(__p) && __p->color == Color::WHITE; }


/**
 * @brief Whether the node is in the given branch of its parent.
 * Note that if current node is root , it returns false.
 * 
 * @tparam dir The target direction of the node.
 * @param __p Pointer to current node.
 * @return || 0 if root or on different branch || 1 otherwise ||
 */
template <bool dir>
[[deprecated]] inline bool is_dir(const node_base *__p) noexcept
{ return !is_root(__p) && __p->dir(dir); }


/* Judge whether a node is white safely. */
inline bool is_white(const node_base *__p) noexcept
{ return __p && __p->color == Color::WHITE; }


/* Judge whether a node is black safely. */
[[deprecated]] inline bool is_black(const node_base *__p) noexcept
{ return !__p || __p->color == Color::BLACK; }


/**
 * @brief Advance toward target direcition by one step.
 * 
 * @tparam dir Target direction.
 * @param __p The pointer to advance.
 */
template <bool dir>
void advance(node_base *&__p) noexcept { 
    if(!__p || __p == __p->son[dir]) return; /* Null or empty case. */

    if(__p->son[dir]) { /* Downward */
        __p = __p->son[dir];
        while(__p->son[!dir]) __p = __p->son[!dir];
    } else { /* Upward. */
        node_base * __f = __p->parent;
        while(__f->parent != __p && __f->son[dir] == __p)
            __f = (__p = __f)->parent;
        __p = __f;
    }
}

/**
 * @brief Advance toward target direcition by one step.
 * 
 * @tparam dir Target direction.
 * @param __p The pointer to advance.
 */
template <bool dir>
void advance(const node_base *&__p) noexcept { 
    if(!__p || __p == __p->son[dir]) return; /* Null or empty case. */

    if(__p->son[dir]) { /* Downward */
        __p = __p->son[dir];
        while(__p->son[!dir]) __p = __p->son[!dir];
    } else { /* Upward. */
        const node_base * __f = __p->parent;
        while(__f->parent != __p && __f->son[dir] == __p) 
            __f = (__p = __f)->parent;
        __p = __f;
    }
}


using baseptr = node_base *;


/**
 * @brief Rotate current node with its father.
 * Note that father can't be node on tree.
 * 
 * @param __p Pointer to current node.
 * @param dir It must equal __p->dir().
 */
void rotate(baseptr __p,bool dir) {
    baseptr __f = __p->parent; /* Father */
    baseptr __g = __f->parent; /* Grand-father */

    __f->son[dir]  = __p->son[!dir];
    __p->son[!dir] = __f;
    if(__f->son[dir]) __f->son[dir]->parent = __f;

    __f->parent = __p;
    __p->parent = __g;
    if(__g->parent == __f) { /* Father is root. */
        __g->parent = __p;
    } else { /* Father is not root , normal case.  */
        __g->son[__g->son[0] != __f] = __p;
    }
}


/**
 * @brief Rotate current node with its father.
 * 
 * @param __p Pointer to current node.
 */
inline void rotate(baseptr __p) { return rotate(__p,__p->dir()); }


/**
 * @brief Exchange the relative position of current node
 * and next node. Both of which shouldn't be header node.
 * Note that __p must have exactly 2 non-null sons.
 * 
 * @param __p Pointer to current node.
 */
void swap_next(baseptr __p) {
    baseptr __n = __p; /* Pointer to next node. */
    advance <1> (__n);

    __p->son[0]->parent = __n;
    __n->son[0] = __p->son[0];
    __p->son[0] = nullptr; /* Manually unfold std::swap. */

    __p->son[1]->parent = __n; /* If __p->son[1] == __n , it still works! */
    if(__n->son[1]) __n->son[1]->parent = __p;
    std::swap(__n->son[1],__p->son[1]);

    if(is_root(__p)) __p->parent->parent = __n;
    else    __p->parent->son[__p->dir()] = __n;
    if(__n->parent != __n) __n->parent->son[0] = __p;
    std::swap(__n->parent,__p->parent);

    std::swap(__n->color,__p->color);
}


/* Special case when __p and its parent are both red.  */
void insert_fix(baseptr __p) {
    /* Now parent is white , so parent can't be root.  */
    __p->parent->parent->color = Color::WHITE;

    bool dir1 = __p->dir() , dir2 = __p->parent->dir();
    if(dir1 != dir2) {
        __p->color = Color::BLACK;
        rotate(__p,dir1);
        rotate(__p,dir2);
    } else {
        __p->parent->color = Color::BLACK;
        rotate(__p->parent,dir2);
    }
}


/* Adjust the node color after inserting a new node. */
void insert_at(baseptr __p) {
    while(!tree::is_root(__p) && __p->parent->color == Color::WHITE) {
        /* Because parent is white , parent can't be root.  */
        baseptr uncle = __p->parent->bro();
        if(!is_white(uncle)) break;
        uncle->color = __p->parent ->color = Color::BLACK;
        (__p = __p->parent->parent)->color = Color::WHITE;
    }

    if(is_root(__p)) return (void) (__p->color = Color::BLACK);
    if(__p->parent->color == Color::WHITE) insert_fix(__p);
}


/* Replace current node another node in erasion. */
inline void erase_replace(baseptr __p,baseptr __n) {
    if(is_root(__p)) __p->parent->parent = __n;
    else    __p->parent->son[__p->dir()] = __n;
    __n->color  = Color::BLACK;
    __n->parent = __p->parent;
}


/* Clear the branch containing current node. */
inline void erase_branch(baseptr __p) {
    if(is_root(__p)) __p->parent->parent = __p->parent;
    else    __p->parent->son[__p->dir()] = nullptr;
}



/* Fix black leaf case. */
void erase_fix(baseptr __p) {
    while(true) { /* __p side is shorter in black length by 1.  */
        baseptr __f = __p->parent; /* Father node. */

        /* __p is root node , so do nothing now. */
        if(__f->parent == __p) break;

        bool dir = (__f->son[0] != __p); /* Direction of __p. */
        baseptr __b = __f->son[!dir];    /* Brother node. */

        if(__b->color == Color::WHITE) { /* Transform case. */
            rotate(__b,!dir);
            __f->color = Color::WHITE;
            __b->color = Color::BLACK;
            __b = __f->son[!dir]; /* Update brother node. */
        }

        /* Now __b is black. */

        /* Not-in-a-line case, which will be transformed to in-a-line case */
        if(is_white(__b->son[dir])) { 
            rotate(__b->son[dir],dir);
            __b->color = Color::WHITE;
            __b = __b->parent; /* Update to __f->son[!dir] */
            __b->color = Color::BLACK;
        }

        /* In-a-line case. */
        if(is_white(__b->son[!dir])) {
            rotate(__b,!dir); /* Now __b is the root. */
            __b->color = __f->color; /* Father color not changed. */
            __f->color = __b->son[!dir]->color = Color::BLACK;
            break;
        }

        /* Now __b has no son. */

        if(__f->color == Color::WHITE) {
            __f->color = Color::BLACK;
            __b->color = Color::WHITE;
            break;
        }

        /* Double black case require looping. */

        __b->color = Color::WHITE;
        __p = __f;
    }
}


/**
 * @brief Erase at given node. No memory is touched.
 * Deconstruction and deallocation function should be 
 * called after this function by user.
 * 
 * @param __p Pointer to target node.
 */
void erase_at(baseptr __p) {
    if(__p->son[0] && __p->son[1]) swap_next(__p);
    /* Now __p has at most 1 son. */

    /* __p has one son, so the son must be red. */
    if(__p->son[1]) return erase_replace(__p,__p->son[1]); 
    /* This case is slightly less likely to happen due to swap_next. */
    if(__p->son[0]) return erase_replace(__p,__p->son[0]);

    /* Now __p has no son. */
    if(__p->color == Color::BLACK) erase_fix(__p);

    erase_branch(__p);
}


}




}





#endif
