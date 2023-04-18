#ifndef _DARK_DYNAMIC_ARRAY_H_
#define _DARK_DYNAMIC_ARRAY_H_

#include "../third_party/basic.h"
#include "iterator.h"

#include <memory>
#include <initializer_list>

namespace dark {

/**
 * @brief A dynamic %array that can expand itself automatically.
 * In other words, user don't need to consider the allocation and 
 * deallocation of space.
 * Note that if the elements within are pointers, the data pointed 
 * to won't be touched. It's user's responsibility to manage them.
 * 
 * @tparam value_t The type of elements stored within the %array.
 */
template <class value_t>
class dynamic_array : private std::allocator <value_t> {
  private:
    value_t *head; /* Head pointer to first element. */
    value_t *tail; /* Tail pointer to one past the last element. */
    value_t *term; /* Terminal pointer to the end of storage. */

    /* Allocate memory of __n elements. */
    value_t *alloc(size_t __n) { return this->allocate(__n); }
    /* Deallocate of the memory of head. */
    void dealloc() { this->deallocate(head,capacity()); }
    
    /* Destory __n elements */
    void destroy_n(value_t *pointer,size_t __n) {
        while(__n--) { this->destroy(pointer++); }
    }

    /* End of unfolding. */
    void reserved_push_back() noexcept {}

    /* Push back a sequence of elements with space reserved in advance. */
    template <class U,class ...Args>
    void reserved_push_back(U &&obj,Args &&...objs) {
        this->construct(tail++,std::forward <U> (obj));
        reserved_push_back(std::forward <Args> (objs)...);
    }

  public:

    /* Construct a new empty %array. */
    dynamic_array() noexcept : head(nullptr),tail(nullptr),term(nullptr) {}
    /* Destroy all the elements and deallocate the space. */
    ~dynamic_array() noexcept { this->destroy_n(head,size()); dealloc(); }

    /* Construct a new %array from an initializer_list. */
    dynamic_array(std::initializer_list <value_t> __l) 
        : dynamic_array(__l.size()) {
        for(auto &iter : __l) { this->construct(tail++,std::move(iter)); }
    }

    /* Construct a new %array with __n elements' space reserved. */
    dynamic_array(size_t __n) {
        term = (head = tail = alloc(__n)) + __n;
    }

    /**
     * @brief Construct a new %array filled with given length and element.
     * 
     * @param __n The initial length of the %array.
     * @param obj The element to fill the %array.
     */
    dynamic_array(size_t __n,const value_t &obj) 
        : dynamic_array(__n) {
        while(tail != term) { this->construct(tail++,obj); }
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
    dynamic_array(const dynamic_array &rhs) 
        : dynamic_array(rhs.size()) {
        for(const auto &iter : rhs) { this->construct(tail++,iter);}
    }

    /**
     * @brief Construct a new %array with identical elements with another %array.
     * It will just take away the pointers from another %array.
     * 
     * @param rhs The %array to move from.
     * @attention Constant time complexity in any case.
     */
    dynamic_array(dynamic_array &&rhs) noexcept {
        head = rhs.head;
        tail = rhs.tail;
        term = rhs.term;
        rhs.head = rhs.tail = rhs.term = nullptr;
    }

    /**
     * @brief Copy assign a new %array with identical elements with another %array.
     * Note that no vacancy of %array remains, 
     * which means the new %array's size() equals its capacity(). 
     * 
     * @param rhs The %array to copy from.
     * @attention Linear time complexity with respect to the size() of rhs,
     * multiplied by the construction time of one element,
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    dynamic_array &operator = (const dynamic_array &rhs) {
        if(this != &rhs) { copy_range(rhs.begin(),rhs.size()); }
        return *this;
    }

    /**
     * @brief Construct a new %array with identical elements with another %array.
     * It will just move the pointers from another %array.
     * 
     * @param rhs The %array to move from.
     * @attention Constant time complexity in any case.
     */
    dynamic_array &operator = (dynamic_array &&rhs) noexcept {
        if(this != &rhs) {
            this->~dynamic_array();
            head = rhs.head;
            tail = rhs.tail;
            term = rhs.term;
            rhs.head = rhs.tail = rhs.term = nullptr;
        }
        return *this;
    }

    /* Swap the content of two %array in constant time. */
    dynamic_array &swap(dynamic_array &rhs) noexcept {
        std::swap(head,rhs.head);
        std::swap(tail,rhs.tail);
        std::swap(term,rhs.term);
        return *this;
    }
    /* Swap the content of two %array in constant time. */
    friend void swap(dynamic_array &lhs,dynamic_array &rhs) noexcept {
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

    /* Doing nothing to the %array. */
    void push_back() noexcept {}

    /**
     * @brief Push one element to the back of the %array.
     * 
     * @param obj The object pushed back to initialize the element. 
     * @attention Amortized constant time complexity,
     * multiplied by the construction time of one element.
     */
    template <class U>
    void push_back(U &&obj) {
        if(tail == term) { reserve(size() << 1 | empty()); } 
        this->construct(tail++,std::forward <U> (obj));
    }

    /**
     * @brief Push a sequnce of elements to the back of the %array.
     * 
     * @param objs The objects pushed back to initialize the element.
     * @attention Amortized linear time complexity with respect to count of objs,
     * multiplied by the construction time of one element.
     */
    template <class ...Args>
    void push_back(Args &&...objs) {
        size_t count = sizeof...(objs); // count >= 2 
        if(vacancy() < count) {
            size_t space = capacity() + empty();
            reserve(space << (LOG2((size() + count - 1) / space) + 1));
        }
        reserved_push_back(std::forward <Args> (objs)...);
    }

    /**
     * @brief Construct one element after the back of the %array.
     * 
     * @param obj The object pushed back to initialize the element. 
     * @attention Amortized constant time complexity,
     * multiplied by the construction time of one element.
     */
    template <class ...Args>
    void emplace_back(Args &&...objs) {
        if(tail == term) { reserve(size() << 1 | empty()); }
        this->construct(tail++,std::forward <Args> (objs)...);
    }

    /* Destroy the last element in the back, with no returning. */
    void pop_back() noexcept { this->destroy(--tail); }

    /**
     * @brief Clear all the elements in the %array.
     * Note that the capacity() of the %array won't shrink.
     * 
     * @attention Linear complexity with respect to size(),
     * multiplied by the deconstruction time of one element.
     */
    void clear() noexcept {
        this->destroy_n(head,size());
        tail = head;
    }

    /**
     * @brief Clear the vacancy of the %array.
     * Note that it will disable the optimization of the %array.
     * 
     * @attention Linear complexity with respect to size(),
     * multiplied by the deconstruction time of one element.
     */
    void shrink() {
        if(tail != term) {
            value_t *temp = alloc(size());
            for(size_t i = 0 ; i < size() ; ++i) {
                this->construct(temp + i,std::move(head[i]));
            }

            this->~dynamic_array();
            term = tail = temp + size();
            head = temp;
        }
    }

    /**
     * @brief Resize the %array to __n.
     * The original data with index smaller than __n won't be touched.  
     * If __n is greater than size(), elements will be appended to the back.
     * These new elements will be assigned by default construction function.
     * 
     * @param __n The new size of the %array.
     * @attention Linear complexity with respect to __n ,
     * multiplied by the construction time of one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    void resize(size_t __n) {
        if(__n <= size()) {
            size_t count = size() - __n;
            tail -= count;
            this->destroy_n(tail,count);
        } else {
            reserve(__n);
            size_t count = __n - size();
            while(count--) { this->construct(tail++); } 
        }
    }


    /**
     * @brief Resize the %array to __n.
     * The original data with index smaller than __n won't be touched.  
     * If __n is greater than size(), elements will be appended to the back.
     * These new elements will be assigned by val.
     * 
     * @param __n The new size of the %array.
     * @param val The object to assign the new value.
     * @attention Linear complexity with respect to __n ,
     * multiplied by the construction time of one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    void resize(size_t __n,const value_t &val) {
        if(__n <= size()) {
            size_t count = size() - __n;
            tail -= count;
            this->destroy_n(tail,count);
        } else {
            reserve(__n);
            size_t count = __n - size();
            while(count--) { this->construct(tail++,val); } 
        }
    }

    /**
     * @brief Reserve space for __n elements.
     * If __n < capacity(), nothing will be done.
     * 
     * @param __n The space reserved for elements.
     * @attention Linear time complexity with respect to __n,
     * only if __n >= capacity(), multiplied by the time of (de-)construction. 
     * Otherwise, constant time complexity.
     */
    void reserve(size_t __n) {
        if(capacity() < __n) {
            value_t *temp = alloc(__n);
            for(size_t i = 0 ; i < size() ; ++i) {
                this->construct(temp + i,std::move(head[i]));
            }

            this->~dynamic_array();
            term = temp + __n;
            tail = temp + size();
            head = temp;
        }
    }

    /**
     * @brief Resize the %array to __n and assign all the elements by 
     * default construction function of value_t. 
     * 
     * @param __n The new size of the %array.
     * @attention Linear complexity with respect to __n ,
     * multiplied by the construction time of one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    void assign(size_t __n) {
        const value_t val = value_t();
        if(__n <= size()) {
            size_t count = size() - __n;
            tail -= count;
            this->destroy_n(tail,count);
            for(auto &iter : *this) { iter = val; }
        } else {
            for(auto &iter : *this) { iter = val; }
            reserve(__n);
            size_t count = __n - size();
            while(count--) this->construct(tail++);
        }
    }

    /**
     * @brief Resize the %array to __n and assign all the elements by val. 
     * 
     * @param __n The new size of the %array.
     * @param val The object to assign the value.
     * @attention Linear complexity with respect to __n ,
     * multiplied by the construction time of one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    void assign(size_t __n,const value_t &val) {
        if(__n <= size()) {
            size_t count = size() - __n;
            tail -= count;
            this->destroy_n(tail,count);
            for(auto &iter : *this) { iter = val; }
        } else {
            for(auto &iter : *this) { iter = val; }
            reserve(__n);
            size_t count = __n - size();
            while(count--) this->construct(tail++,val);
        }
    }
    
    /**
     * @brief Copy elements from a range [first,last).
     * Note that the Iterator must be random access iterator.
     * Otherwise, you should provide the count of elements.
     * 
     * @tparam Iterator A random access iterator type.
     * @param first Iterator to the first element.
     * @param last  Iterator to one past the last element.
     * @attention Linear time complexity with respect to (last - first)
     * multiplied by the time of moving one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    template <class Iterator>
    void copy_range(Iterator first,Iterator last) {
        return copy_range(first,last - first);
    }

    /**
     * @brief Copy elements from a range [first,last).
     * The number of elements in the range should be exactly __n,
     * or unexpected error may happen.
     * 
     * 
     * @param first Iterator to the first element.
     * @param last  Iterator to one past the last element.
     * @param __n   Count of all the elements in the range.
     * @attention Linear time complexity with respect to __n,
     * multiplied by the time of copying one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    template <class Iterator>
    void copy_range(Iterator first,size_t __n) {
        if(__n <= capacity()) {
            value_t *temp = head;
            while(__n-- && temp != tail) { *(temp++) = *(first++); }
            ++__n;
            while(__n--) { this->construct(tail++,*(first++)); }
            this->destroy_n(temp,tail - temp);
            tail = temp;
        } else {
            this->~dynamic_array();
            term = (tail = head = alloc(__n)) + __n;
            while(__n--) { this->construct(tail++,*(first++)); } 
        }
    }


    /**
     * @brief Move elements from a range [first,last).
     * Note that the Iterator must be random access iterator.
     * Otherwise, you should provide the count of elements.
     * 
     * @tparam Iterator A random access iterator type.
     * @param first Iterator to the first element.
     * @param last  Iterator to one past the last element.
     * @attention Linear time complexity with respect to (last - first),
     * multiplied by the time of moving one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    template <class Iterator>
    void move_range(Iterator first,Iterator last) {
        return move_range(first,last - first);
    }

    /**
     * @brief Move elements from a range [first,last).
     * The number of elements in the range should be exactly __n,
     * or unexpected error may happen.
     * 
     * @param first Iterator to the first element.
     * @param last  Iterator to one past the last element.
     * @param __n   Count of all the elements in the range.
     * @attention Linear time complexity with respect to __n,
     * multiplied by the time of moving one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    template <class Iterator>
    void move_range(Iterator first,size_t __n) {
        if(__n <= capacity()) {
            value_t *temp = head;
            while(__n-- && temp != tail) { *(temp++) = std::move(*(first++)); }
            ++__n;
            while(__n--) { this->construct(tail++,std::move(*(first++))); }
            this->destroy_n(temp,tail - temp);
            tail = temp;
        } else {
            this->~dynamic_array();
            term = (tail = head = alloc(__n)) + __n;
            while(__n--) { this->construct(tail++,std::move(*(first++))); } 
        }
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
