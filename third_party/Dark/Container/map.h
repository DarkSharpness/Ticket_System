#ifndef _DARK_MAP_H_
#define _DARK_MAP_H_

#include "tree.h"

#include <cstddef>
#include <cstdlib>
#include <functional>

namespace dark {


template <class key_t,
          class T,
          class Compare = std::less <key_t> >
class map {
  public:

    using value_t   = std::pair <const key_t,T>;

    using iterator               = tree::iterator_base <value_t,0,1>;
    using const_iterator         = tree::iterator_base <value_t,1,1>;
    using reverse_iterator       = tree::iterator_base <value_t,0,0>;
    using const_reverse_iterator = tree::iterator_base <value_t,1,0>;

    using return_t  = std::pair <iterator,bool>;
  
  private:

    using node_base = tree::node_base;
    using baseptr   = tree::node_base *;
    using node      = tree::node <value_t>;
    using pointer   = tree::node <value_t> *;
    using implement = implement <node,std::allocator <node>,Compare>;

  private:

    implement impl;   /* Implement of compare and memory function. */
    node_base header; /* Parent as root node || son[0] as largest || son[1] as smallest. */

    /* Return the root node of the tree. */
    baseptr root() const noexcept { return header.parent; }

    /* Special case : insert at root node. */
    return_t insert_root(baseptr __n) {
        ++impl.count;
        header.parent = header.son[0] = header.son[1] = __n;
        __n->parent   = &header;
        __n->color    = tree::Color::BLACK;
        return {__n,true};
    }

    /* Clean subtree info without modifying tree size. */
    void clean(baseptr __p) {
        if(!__p) return;
        clean(__p->son[0]);
        clean(__p->son[1]);
        impl.dealloc(static_cast <pointer> (__p));
    }

    /* Copy sub_tree information. Note that __p can't be null! */
    pointer copy(pointer __p) {
        pointer __c = impl.alloc(*__p); /* Current node. */
        if(__c->son[0]) {
            __c->son[0] = copy((pointer)__p->son[0]);
            __c->son[0]->parent = __c;
        }
        if(__c->son[1]) {
            __c->son[1] = copy((pointer)__p->son[1]);
            __c->son[1]->parent = __c;
        }
        return __c;
    }

    /**
     * @brief Tries to locate a node with given key.
     * Note that the tree mustn't be empty.
     * 
     * @param __k Key to locate.
     * @return Pointer to node with given key.
     * If not existing , return nullptr.
     */
    baseptr locate(const key_t &__k) const noexcept {
        if(empty()) return nullptr;
        baseptr __p = root();
        while(__p) {
            if(impl (__k,pointer(__p)->data.first) )
                __p = __p->son[0];
            else if(impl (pointer(__p)->data.first,__k) )
                __p = __p->son[1];
            else break;
        }
        return __p;
    }

    /**
     * @brief Insert a key-value pair into the map.
     * 
     * @param __n Pointer to new memory block or nullptr.
     * @param __v Key-value to be inserted.
     * @return return_t A pair of an iterator of the element and
     * a boolean of whether the insertion has been successful.
     */
    template <class U>
    return_t insert_pair(U &&__v) {
        /* 
            In fact, this function was intended to support std::piecewise construct.
            However,it turned out that this is too much for me.
            I'm fed up . So you see such a function as below , like a piece of SHIT.
        */

        if(empty()) /* Just empty , so insert at root node. */
            return insert_root(impl.alloc(tree::forward_tag(),
                                           std::forward <U> (__v)));

        baseptr __p = header.parent;
        bool dir;
        while(true) {
            if(impl (__v.first,pointer(__p)->data.first) ) {
                if(__p->son[0]) __p = __p->son[0];
                else { dir = 0; break; }
            }
            else if(impl (pointer(__p)->data.first,__v.first) )
                if(__p->son[1]) __p = __p->son[1];
                else { dir = 1 ;break; }
            else return {__p,false};
        }

        ++impl.count;
        __p->son[dir] = impl.alloc(tree::forward_tag(),
                                    std::forward <U> (__v));
        __p->son[dir]->parent = __p;

        /* May update the largest / smallest. */
        if(__p == header.son[!dir]) header.son[!dir] = __p->son[dir];

        tree::insert_at(__p = __p->son[dir]);
        return {__p,true};
    }

    /**
     * @brief Inner function to erase given node.
     * 
     * @param __p Pointer to given node.
     * @return Always be 1.
     */
    size_t erase_pair(baseptr __p) {
        /*  Update largest in constant time. */
        if(__p == header.son[0])
            header.son[0] = __p->son[0] ? __p->son[0] : __p->parent;
        /* Update smallest in constant time. */
        if(__p == header.son[1])
            header.son[1] = __p->son[1] ? __p->son[1] : __p->parent;

        tree::erase_at(__p);
        --impl.count;
        impl.dealloc(static_cast <pointer> (__p));
        return 1;
    }

    /**
     * @brief Access the the value tied with given key.
     * 
     * @param __k Key to locate.
     * @return Pointer to the value. If not found return nullptr. 
     */
    T *access(const key_t & __k) const noexcept {
        baseptr iter = locate(__k);
        if(!iter) return nullptr;
        else return &pointer(iter)->data.second;
    }

    /* Manually initialize the header and count of nodes. */
    void initialize() noexcept { ::new (&header) node_base(&header); impl.count = 0; }

  public:
    /* Initialize from empty. */
    map() noexcept : impl(),header(&header) {}

    /* Initialize by copying another map. */
    map(const map &rhs) : impl(rhs.impl),header(&header) {
        if(rhs.empty()) return;

        header.parent  = copy(pointer(rhs.root()));
        root()->parent = &header;

        /* Find out the smallest and largest node. */
        baseptr __p;

        __p = root(); /* Smallest */
        while(__p->son[0]) __p = __p->son[0];
        header.son[1] = __p;

        __p = root(); /* Largest  */
        while(__p->son[1]) __p = __p->son[1];
        header.son[0] = __p;
    }

    /* Initialize by moving another map.  */
    map(map &&rhs) noexcept : impl(std::move(rhs.impl)),header(&header) {
        if(rhs.empty()) return;

        header = rhs.header;
        root()->parent = &header;

        /* Clean the data of rhs map. */
        rhs.initialize();
    }

    /* Too lazy to overload. */
    map &operator = (const map &rhs) {
        if(this != &rhs) {
            this->~map();
            ::new (this) map(rhs);
        }
        return *this;
    }

    /* Too lazy to overload. */
    map &operator = (map &&rhs) {
        if(this != &rhs) {
            this->~map();
            ::new (this) map(std::move(rhs));
        }
        return *this;
    }

    /* Clean the storage. */
    ~map() noexcept { clear(); }

    /* Return whether the map is empty. */
    bool empty()  const noexcept { return !impl.count; }

    /* Return count of key-value pair in this map. */
    size_t size() const noexcept { return impl.count;  }

    /**
     * @brief Insert a key-value pair to the map.
     * If there exists indentical key, insertion will fail.
     *
     * @param __v Key-value pair to be inserted.
     * @return A pair of an iterator of the element and
     * a boolean of whether the insertion has been successful.
     */
    return_t insert(const value_t &__v) { return insert_pair(__v); }
    /**
     * @brief Insert a key-value pair to the map.
     * If there exists indentical key, insertion will fail.
     *
     * @param __v Key-value pair to be inserted.
     * @return A pair of an iterator of the element and
     * a boolean of whether the insertion has been successful.
     */
    return_t insert(value_t &&__v) { return insert_pair(std::move(__v)); }

    /**
     * @brief Erase all key-value pair with given key from the map.
     * If no such key exists, nothing will be done.
     * 
     * @param __k Key to locate the key-value pair.
     * @return Count of key-value pairs erased. (0 or 1)
     */
    size_t erase(const key_t &__k) {
        baseptr __p = locate(__k);
        if(!__p) return 0;
        else return erase_pair(__p);
    }

    /**
     * @brief Erase one key-value pair with given iterator from the map.
     * 
     * @param __i Non-const (reverse) iterator to a key-value pair.
     * @return Count of key-value pairs erased. (0 or 1)
     */
    template <bool dir>
    size_t erase(tree::iterator_base <value_t,0,dir> __i) {
        if(is_header(__i.base())) return 0;
        else return erase_pair(__i.base());
    }

    /**
     * @brief Find a node with given key.
     * 
     * @param __k Key to locate the node.
     * @return Iterator to the node if found,
     * otherwise iterator to end.
     */
    iterator find(const key_t &__k) noexcept {
        baseptr __p = locate(__k);
        if(!__p)    return end();
        else        return {__p};
    }

    /**
     * @brief Find a node with given key.
     * 
     * @param __k Key to locate the node.
     * @return Iterator to the node if found,
     * otherwise iterator to end.
     */
    const_iterator find(const key_t &__k) const noexcept {
        baseptr __p = locate(__k);
        if(!__p)    return end();
        else        return {__p};
    }

    /* Return counts of node with given key. (0 or 1) */
    size_t count(const key_t &__k) const noexcept 
    { return locate(__k) != nullptr; }

    /* Clear the memory storage. */
    void clear() noexcept {
        if(impl.count) {
            clean(root());
            initialize();
        }
    }

    T &operator [] (const key_t &__k) {
        /// TODO: Reduce the expansion of code as much as possible.
        throw ;
    }

  public:
    /* Iterator Part. */

    iterator begin() noexcept { return header.son[1]; }
    iterator end()   noexcept { return   &header;     }

    const_iterator begin() const noexcept  { return header.son[1]; }
    const_iterator end()   const noexcept  { return   &header;     }
    const_iterator cbegin() const noexcept { return header.son[1]; }
    const_iterator cend()   const noexcept { return   &header;     }

    reverse_iterator rbegin() { return header.son[0]; }
    reverse_iterator rend()   { return   &header;     }
    const_reverse_iterator rbegin() const noexcept  { return header.son[0]; }
    const_reverse_iterator rend()    const noexcept { return   &header;     }
    const_reverse_iterator crbegin() const noexcept { return header.son[0]; }
    const_reverse_iterator crend()   const noexcept { return   &header;     }
};



}


#endif
