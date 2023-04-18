#ifndef _DARK_HEAP_H_
#define _DARK_HEAP_H_


#include "allocator.h"

#include <cstddef>


namespace dark {


template <class value_t,class Compare>
class heap {
  private:

    struct node;
    using implement = implement <node,std::allocator <node>,Compare>;
    using pointer = node *;

  private:

    pointer   root;  /* Root node of the tree. */
    implement impl;  /* Allocator and compare function. */

    /* Merge 2 nodes x and y. */
    static pointer merge(pointer x,pointer y) {
        if(!x) return y;
        if(!y) return x;
        /* Now x and y are not empty. */
        if(impl(y->val,x->val)) std::swap(x,y); // Smaller as x
        x->rs = merge(x->rs,y);
        std::swap(x->ls,x->rs);
        return x;
    }

    /* Copy node info from cur node. */
    pointer copy(pointer cur) {
        if(!cur) return nullptr;
        return alloc(copy(cur->ls),copy(cur->rs),cur->dist,cur->val);
    }

    /* Remove current node with its sub-tree */
    void remove(pointer cur) {
        if(!cur) return;
        remove(cur->ls);
        remove(cur->rs);
        this->dealloc(cur);
    }

  public:

    /* Emptized. */
    heap() noexcept : root(nullptr) {}

    /* Copy content. */
    heap(const heap &rhs) : root(copy(rhs.root)),impl(rhs.impl) {}

    /* Move content. */
    heap(heap &&rhs) noexcept
    : root(rhs.root),impl(std::move(rhs.impl)) { rhs.root = nullptr; }

    /* Now heap is re-initialized to empty. */
    ~heap() noexcept { clear(); }

    /* Copy content. */
    heap &operator = (const heap &rhs) {
        if(this != &rhs) {
            this->~heap();
            ::new (this) heap(rhs);
        }
        return *this;
    }

    /* Move content. */
    heap &operator = (heap &&rhs) noexcept {
        if(this != &rhs) {
            this->~heap();
            ::new (this) heap(std::move(rhs));
        }
        return *this;
    }

    /* Clean all the elements. */
    void clear() noexcept { remove(root); root = nullptr; impl.count = 0; }

    /* Return const reference to the smallest element. */
    const value_t &top() const { return root->val; }

    /* Push one element to the heap. */
    template <class ...Args>
    void emplace(Args &&...objs)  { 
        root = merge(root,impl.alloc(nullptr,
                                     nullptr,
                                     std::forward <Args> (objs)...));
        ++impl.count;
    }

    void push(value_t &&obj) { 
        root = merge(root,impl.alloc(nullptr,nullptr,std::move(obj))); 
        ++impl.count;
    }

    void push(const value_t &obj) {
        root = merge(root,impl.alloc(nullptr,nullptr,obj));
        ++impl.count;
    }

    /* Pop out the top element. */
    void pop() {
        if(!root) return; /* Empty tree. */
        --impl.count;
        node *temp = root;
        root = merge(root->ls,root->rs);
        impl.dealloc(temp);
    }

    bool empty()  const noexcept { return !root; }
    size_t size() const noexcept { return impl.count; }
  
  private:

    struct node {
        pointer ls;
        pointer rs;
        value_t val;

        template <class ...Args>
        node(pointer _l,pointer _r,size_t _d,Args &&...objs):
            ls(_l),rs(_r),val(std::forward <Args> (objs)...) {}

        ~node() = default;
    };

};



}


#endif
