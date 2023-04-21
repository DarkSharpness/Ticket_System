#ifndef _DARK_TRIVIAL_ARRAY_H_
#define _DARK_TRIVIAL_ARRAY_H_

#include "iterator.h"

#include <memory>
#include <string.h>
#include <initializer_list>

namespace dark {

/**
 * @brief An array holding trivial types , which can
 * utilizing memcpy and memset to speed up greatly.
 * 
 * @tparam value_t A trivial type.
 */
template <class value_t>
class trivial_array : private std::allocator <value_t>  {
  private:
    value_t *head; /* Head pointer to first element. */
    value_t *tail; /* Tail pointer to one past the last element. */
    value_t *term; /* Terminal pointer to the end of storage. */

    /* Allocate memory of __n elements. */
    value_t *alloc(size_t __n) { return this->allocate(__n); }
    /* Deallocate of the memory of head. */
    void dealloc() { this->deallocate(head,capacity()); }

  public:

    trivial_array() noexcept : head(nullptr),tail(nullptr),term(nullptr) {}

    ~trivial_array() noexcept { dealloc(); }

    /* Construct a new %array with __n elements' space reserved. */
    trivial_array(size_t __n) { term = (head = tail = alloc(__n)) + __n; }
    
    /* Construct a new %array with __n elements' space filled 0. */
    trivial_array(size_t __n,std::nullptr_t)
    : trivial_array(__n) { memset(head,0,__n * sizeof(value_t)); tail = term; }

    /* Construct a new %array from an initializer_list. */
    trivial_array(std::initializer_list <value_t> __l) 
        : trivial_array(__l.size()) {
        memcpy(head,__l.begin(),size() * sizeof(value_t));
        tail = term;
    }

    /**
     * @brief Construct a new %array with identical elements with another %array.
     * Note that no vacancy of %array remains, 
     * which means the new %array's size() equals its capacity(). 
     * 
     * @param rhs The %array to copy from.
     * @attention Linear time complexity with respect to the size() of rhs,
     * multiplied by the construction time of one element.
     */
    trivial_array(const trivial_array &rhs) 
        : trivial_array(rhs.size()) {
        memcpy(head,rhs.head,size() * sizeof(value_t));
    }

    /**
     * @brief Construct a new %array with identical elements with another %array.
     * It will just take away the pointers from another %array.
     * 
     * @param rhs The %array to move from.
     * @attention Constant time complexity in any case.
     */
    trivial_array(trivial_array &&rhs) noexcept {
        head = rhs.head;
        tail = rhs.tail;
        term = rhs.term;
        rhs.head = rhs.tail = rhs.term = nullptr;
    }

    /* Swap the content of two %array in constant time. */
    trivial_array &swap(trivial_array &rhs) noexcept {
        std::swap(head,rhs.head);
        std::swap(tail,rhs.tail);
        std::swap(term,rhs.term);
        return *this;
    }

    /* Swap the content of two %array in constant time. */
    friend void swap(trivial_array &lhs,trivial_array &rhs) noexcept {
        std::swap(lhs.head,rhs.head);
        std::swap(lhs.tail,rhs.tail);
        std::swap(lhs.term,rhs.term);
    }

  public:

    /* Count of elements within the %array. */
    size_t size() const noexcept { return tail - head; }
    /**
     * @brief Count of elements the %array can hold 
     * before the next allocation.
     */
    size_t capacity() const noexcept { return term - head; }
    /**
     * @brief Count of vacancy in the back of the %array  
     * before the next allocation.
     */
    size_t vacancy() const noexcept { return term - tail; }
    /* Test whether the %array is empty */
    bool empty()  const noexcept { return head == tail; }

    /* Push back one element using memcpy. */
    void copy_back(const value_t &obj) {
        if(tail == term) { reserve(size() << 1 | empty()); } 
        memcpy(tail++,&obj,sizeof(value_t));
    }

    /* Push back one element using assignment. */
    void push_back(const value_t &obj) {
        if(tail == term) { reserve(size() << 1 | empty()); }
        *(tail++) = obj;
    }

    /* Pop out and return the last element with no destruction. */
    value_t &pop_back() noexcept { return *--tail;  }

    /* Clear the container in constant time. */
    void clear() noexcept { tail = head; }

    /* Reserve space for __n elements. */
    void reserve(size_t __n) {
        if(capacity() < __n) {
            value_t *temp = alloc(__n);
            memcpy(temp,head,size() * sizeof(value_t));
            this->dealloc();

            term = temp + __n;
            tail = temp + size();
            head = temp;
        }
    }

    /* Resize to __n. */
    void resize(size_t __n) { reserve(__n); tail = head + __n; }

    /* Resize to __n and fill extra space with 0. */
    void resize(size_t __n,std::nullptr_t) {
        reserve(__n);
        if(size() < __n) memset(tail,0,(__n - size()) * sizeof(value_t));
        tail = head + __n;
    }

  public:
    /* Return the pointer to the first element. */
    value_t *data() { return head; }
    /* Return the pointer to the first element. */
    const value_t *data() const { return head; }
    /* Subscript access to the data in the %array.  */
    value_t &operator [] (size_t __n) { return head[__n]; }
    /* Subscript access to the data in the %array.  */
    const value_t &operator [] (size_t __n) const { return head[__n]; }

    /* Reference to the first element. */
    value_t &front() {return *begin();}
    /* Reference to the  last element. */
    value_t &back()  {return *--end();}
    /* Const reference to the first element. */
    const value_t &front() const {return *cbegin();}
    /* Const reference to the  last element. */
    const value_t &back()  const {return *--cend();}


    using iterator               = RandomAccess::iterator_base <value_t,0,1>;
    using const_iterator         = RandomAccess::iterator_base <value_t,1,1>;
    using reverse_iterator       = RandomAccess::iterator_base <value_t,0,0>;
    using const_reverse_iterator = RandomAccess::iterator_base <value_t,1,0>;

    /* Iterator to the first element. */
    iterator begin() noexcept { return head; }
    /* Iterator to one past the last element. */
    iterator end()   noexcept { return tail; }

    /* Const_iterator to the first element. */
    const_iterator begin()  const noexcept {return head;}
    /* Const_iterator to one past the last element. */
    const_iterator end()    const noexcept {return tail;}
    /* Const_iterator to the first element. */
    const_iterator cbegin() const noexcept {return head;}
    /* Const_iterator to one past the last element. */
    const_iterator cend()   const noexcept {return tail;}


    /* Reverse iterator to the last element. */
    reverse_iterator rbegin() noexcept { return tail - 1; }
    /* Reverse iterator to one before the first element. */
    reverse_iterator rend()   noexcept { return head - 1; }

    /* Const_reverse_iterator to the last element. */
    const_reverse_iterator rbegin()  const noexcept {return tail - 1;}
    /* Const_reverse_iterator to one before the first element. */
    const_reverse_iterator rend()    const noexcept {return head - 1;}
    /* Const_reverse_iterator to the last element. */
    const_reverse_iterator crbegin() const noexcept {return tail - 1;}
    /* Const_reverse_iterator to one before the first element. */
    const_reverse_iterator crend()   const noexcept {return head - 1;}

};


}


#endif
