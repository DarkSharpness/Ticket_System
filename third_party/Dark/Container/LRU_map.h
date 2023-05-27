#ifndef _DARK_LRU_MAP_H_
#define _DARK_LRU_MAP_H_

#include "hash.h"
#include "allocator.h"
#include "Dark/trivial_array"

#include <iostream>
#include <cstring>

namespace dark {


/**
 * @brief Map for homework only. Do not use it!
 * 
 */
template <
    class key_t,
    class   T,
    size_t kTABLESIZE,
    class Hash    = std::hash <key_t>,
    class Compare = std::equal_to <key_t>
> 
class linked_hash_map {
  public:
    using value_t = std::pair <key_t,T>;
    struct iterator;

  private:

    using node_base = hash::node_base;
    using list_node = list::node_base;
    using baseptr   = hash::baseptr;
    using listptr   = list::baseptr;
    using node      = hash::linked_node <value_t>;
    using pointer   = hash::linked_node <value_t> *;
    using Implement = implement <node,std::allocator <node>,Compare,Hash>;

  private:

    Implement impl;     /* Implement of the map. */ 
    list_node header;   /* Header of list. */
    node_base cache;    /* Custom memory pool. */
    node_base table[kTABLESIZE]; /* Hash table. */

  private:

    /* Access a node and update it. */
    void access(baseptr __p) {
        list::delink(static_cast <pointer> (__p));
        list::link_before(&header,static_cast <pointer> (__p));
    }

    /* Return the first pointer by given key. */
    baseptr find_index(const key_t &__k) noexcept
    { return &table[Hash(impl)(__k) % kTABLESIZE]; }

    /* Allocate one node with given key and value. */
    baseptr allocate(const key_t &__k,const T &__v,bool useless) {
        if(cache.real) { /* Allocate from cache if available. */
            baseptr temp = cache.real; 
            cache.real   = temp->real;
            static_cast <pointer> (temp)->data.first = __k;
            if(!useless) memcpy(&static_cast <pointer> (temp)->data.second,
                                &__v,
                                sizeof(T));
            return temp;
        } else return impl.alloc(hash::forward_tag(),__k,__v);
    }

    /* Deallocate one node after given pointer. */
    void deallocate(baseptr __p) {
        __p->real  = cache.real;
        cache.real = __p;
    }

    /* Find the previous baseptr in the map.  */
    baseptr find_key(const key_t &__k) {
        baseptr __p = find_index(__k);
        while(__p->real) {
            if( /* Given key equal to the key of current node. */
                Compare(impl) ( __k,
                static_cast <pointer> (__p->real) -> data.first)
            ) break;
            __p = __p->real;
        } return __p;
    }

  public:

    linked_hash_map() noexcept : header({&header,&header}) {}

    ~linked_hash_map() {
        listptr __p = header.next;
        while(__p != &header) {
            listptr __n = __p->next; /* Next node. */
            impl.dealloc(static_cast <pointer> (__p));
            __p = __n;
        }
        while(cache.real) {
            baseptr __p = cache.real;
            cache.real  = __p->real;
            impl.dealloc(static_cast <pointer> (__p));
        }
    }

    /* Force to insert a key-value pair. */
    iterator insert(const key_t &__k,const T &__t,bool useless = false) {
        baseptr __p = find_index(__k);

        /* Allocate. */
        ++impl.count;
        baseptr __n = allocate(__k,__t,useless);

        /* Relinking. */
        __n->real   = __p->real;
        __p->real   = __n;
        list::link_before(&header,static_cast <pointer> (__p->real));

        return {__p}; /* Return iterator to previous hash node. */
    }

    /* Try to erase a key from hash_map. */
    void erase(const key_t &__k) {
        baseptr __p = find_key(__k);
        if(!__p->real) return; /* Case: Not found. */

        /* Relinking. */
        baseptr __n = __p->real;
        __p->real   = __n->real;
        list::delink(static_cast <pointer> (__n));

        /* Deallocate. */
        --impl.count;
        deallocate(__n);
    }

    /* Find the previous node in hash_map and access it.  */
    iterator find_pre(const key_t &__k) {
        baseptr __p = find_key(__k);
        if(__p->real) access(__p->real);
        return {__p};
    }

    /* Return pointer to last node's value. */
    value_t *last() { return &static_cast <pointer> (header.next)->data; }

    /* Return count of elements in the map. */
    size_t size() const noexcept { return impl.count; }

    /* Clear all the data within. */
    void clear() {
        listptr __p = header.next;
        while(__p != &header) {
            find_index(static_cast <pointer> (__p)->data.first)
                ->real = nullptr; /* Clear hash table. */
            deallocate(static_cast <pointer> (__p));
            __p = __p->next;
        } /* Now the list is cleared empty.  */
        header.next = header.prev = &header;
    }

 public:

    /* Simple iterator implement. */
    struct iterator {
        baseptr __p; /* Pointer to node_base before the real data. */

        /* Return pointer to real data in next node in hash_map. */
        value_t *next_data() const noexcept
        { return __p->real ? &static_cast <pointer> (__p->real)->data : nullptr; }

        iterator &operator ++(void) noexcept { 
            __p = static_cast <pointer> (static_cast <pointer> (__p)->next);
            return *this;
        }

        value_t &operator * (void) const noexcept
        { return  static_cast <pointer> (__p)->data; }

        value_t *operator ->(void) const noexcept
        { return &static_cast <pointer> (__p)->data; }

        void *base() const noexcept { return __p; }
    };

    iterator begin() { return { static_cast <pointer> (header.next) }; }
    iterator end()   { return { static_cast <pointer> (&header) }; }

    friend bool operator != (const iterator &lhs,const iterator &rhs)
    noexcept { return lhs.base() != rhs.base(); }
};


/**
 * @brief Map for homework only. Do not use it!
 * 
 */
template <
    class key_t,
    size_t kTABLESIZE,
    class Hash    = std::hash <key_t>,
    class Compare = std::equal_to <key_t>
> 
class linked_hash_set {
  public:
    struct iterator;

  private:

    using node_base = hash::node_base;
    using list_node = list::node_base;
    using baseptr   = hash::baseptr;
    using listptr   = list::baseptr;
    using node      = hash::linked_node <key_t>;
    using pointer   = hash::linked_node <key_t> *;
    using Implement = implement <node,std::allocator <node>,Compare,Hash>;

  private:

    Implement impl;     /* Implement of the map. */ 
    list_node header;   /* Header of list. */
    node_base cache;    /* Custom memory pool. */
    node_base table[kTABLESIZE]; /* Hash table. */

  private:

    /* Return the first pointer by given key. */
    baseptr find_index(const key_t &__k) noexcept
    { return &table[Hash(impl)(__k) % kTABLESIZE]; }

    /* Return the first pointer by given hash_code. */
    baseptr find_index(size_t __h) noexcept
    { return &table[__h % kTABLESIZE]; }

    /* Allocate one node with given key and value. */
    baseptr allocate(const key_t &__k) {
        if(cache.real) { /* Allocate from cache if available. */
            baseptr temp = cache.real; 
            cache.real   = temp->real;
            temp->data   = __k; 
            return temp;
        } else return impl.alloc(hash::forward_tag(),__k);
    }

    /* Deallocate one node after given pointer. */
    void deallocate(baseptr __p) {
        __p->real  = cache.real;
        cache.real = __p;
    }

    /* Find the previous baseptr in the set.  */
    baseptr find_key(const key_t &__k) {
        baseptr __p = find_index(__k);
        while(__p->real) {
            if( /* Given key equal to the key of current node. */
                Compare(impl) ( __k,
                static_cast <pointer> (__p->real) -> data)
            ) break;
            __p = __p->real;
        } return __p;
    }

    /* Find the previous baseptr in the set.  */
    baseptr find_hash(size_t __h) {
        baseptr __p = find_index(__h);
        while(__p->real) {
            if( /* Given hash equal to the hash of current node. */
                __h == Hash(impl)
                        (static_cast <pointer> (__p->real) -> data)
            ) break;
            __p = __p->real;
        } return __p;
    }

    bool erase_next(baseptr __p) {
        if(!__p->real) return false; /* Case: Not found. */

        /* Relinking. */
        baseptr __n = __p->real;
        __p->real   = __n->real;
        list::delink(static_cast <pointer> (__n));

        /* Deallocate. */
        --impl.count;
        deallocate(__n);
        return true;
    }

  public:

    linked_hash_set() noexcept : header({&header,&header}) {}

    ~linked_hash_set() {
        listptr __p = header.next;
        while(__p != &header) {
            listptr __n = __p->next; /* Next node. */
            impl.dealloc(static_cast <pointer> (__p));
            __p = __n;
        } shrink();
    }

    /**
     * @brief Force to insert a key into the set.
     * 
     * @param __k Key to insert.
     */
    void insert(const key_t &__k) {
        baseptr __p = find_index(__k);

        /* Allocate. */
        ++impl.count;
        baseptr __n = allocate(__k);

        /* Relinking. */
        __n->real   = __p->real;
        __p->real   = __n;
        list::link_before(&header,static_cast <pointer> (__p->real));

        // return {__p}; /* Return iterator to previous hash node. */
    }

    /**
     * @brief Tries to erase a key from the set.
     * 
     * @param __k Key to erase.
     */
    bool erase(const key_t &__k) { return erase_next(find_key(__k)); }

    /**
     * @brief Tries to erase a key from the set by its hash code.
     * 
     * @param __h Hash code.
     */
    bool erase(size_t __h)      { return erase_next(find_hash(__h)); }

    /**
     * @brief Tries to erase a key from the set by its hash code.
     * 
     * @param __h Hash code.
     */
    bool erase(iterator iter) { return erase_next(iter.__p->real); }

    /* Judge whether the key existed. */
    bool exist(const key_t &__k) { return find_key(__k)->real; }

    /* Judge whether the hash_code existed. */
    bool exist(size_t __h)      { return find_hash(__h)->real; }

    /* Find the pointer to key with given key. */
    key_t *find(const key_t &__k) { return find_pre(__k).next_data(); }

    /* Find the pointer to key with given hash_code. */
    key_t *find(size_t __h)       { return find_pre(__h).next_data(); }

    /* Find the previous node in hash_set.  */
    iterator find_pre(const key_t &__k) { return {find_key(__k)}; }

    /* Find the previous node in hash_set.  */
    iterator find_pre(size_t __h)      { return {find_hash(__h)}; }

    /* Return count of elements in the set. */
    size_t size() const noexcept { return impl.count; }

    /* Return whether the set is empty. */
    bool empty() const noexcept { return !impl.count; }

    /* Clear all the data within. */
    void clear() {
        listptr __p = header.next;
        while(__p != &header) {
            find_index(static_cast <pointer> (__p)->data)
                ->real = nullptr; /* Clear hash table. */
            deallocate(static_cast <pointer> (__p));
            __p = __p->next;
        } header.next = header.prev = &header;
    }
    
    /* Clear all the cache info. */
    void shrink() {
        while(cache.real) {
            baseptr __p = cache.real;
            cache.real  = __p->real;
            impl.dealloc(static_cast <pointer> (__p));
        } cache.real = nullptr;
    }

 public:

    /* Simple iterator implement. */
    struct iterator {
        baseptr __p; /* Pointer to node_base before the real data. */

        /* Return pointer to real data in next node in hash_map. */
        key_t *next_data() const noexcept
        { return __p->real ? &static_cast <pointer> (__p->real)->data : nullptr; }

        iterator &operator ++(void) noexcept { 
            __p = static_cast <pointer> (static_cast <pointer> (__p)->next);
            return *this;
        }

        key_t &operator * (void) const noexcept
        { return  static_cast <pointer> (__p)->data; }

        key_t *operator ->(void) const noexcept
        { return &static_cast <pointer> (__p)->data; }

        void *base() const noexcept { return __p; }
    };

    iterator begin() { return { static_cast <pointer> (header.next) }; }
    iterator end()   { return { static_cast <pointer> (&header) }; }

    friend bool operator != (const iterator &lhs,const iterator &rhs)
    noexcept { return lhs.base() != rhs.base(); }
};

}

#endif
