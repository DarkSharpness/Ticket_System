#ifndef _DARK_LIST_ARRAY_H
#define _DARK_LIST_ARRAY_H

#include "../include/basic.h"
#include <memory>
#include <initializer_list>
#include "../iterator"

namespace dark {


/**
 * @brief A list-like %array which support %array addition and number multiplication.
 * Compared with dark::dynamic_array, this %array have no vacancy left,
 * which means that pop/push_back() and pop/push_front() are costly linear operation.
 * 
 * Abandoned! Don't use it!
 * 
 * 
 * @tparam value_t 
 */
template <class value_t>
class list_array : std::allocator <value_t> {
  protected:
    value_t *head; /* Head pointer. */
    value_t *tail; /* Tail pointer. */

    /* Construct a new %array with __n elements' space reserved. */
    list_array(size_t __n) { head = tail = alloc(__n); }

    /* Allocate memory of __n elements. */
    inline value_t *alloc(size_t __n) { return this->allocate(__n); }
    /* Deallocate of the memory of head. */
    inline void dealloc() { this->deallocate(head,size()); }
    
    /* Destory __n elements */
    void destroy_n(value_t *pointer,size_t __n) {
        while(__n--) { this->destroy(pointer++); }
    }


    /* End of unfolding. */
    inline void reserved_push_back() {}
    /* End of unfolding. */
    inline void reserved_push_front() {}


    /* Push back a sequence of elements with space reserved in advance. */
    template <class U,class ...Args>
    void reserved_push_back(U &&obj,Args &&...objs) {
        this->construct(tail++,std::forward <U> (obj));
        reserved_push_back(std::forward <Args> (objs)...);
    }
    /* Push front a sequence of elements with space reserved in advance. */
    template <class U,class ...Args>
    void reserved_push_front(U &&obj,Args &&...objs) {
        this->construct(--head,std::forward <U> (obj));
        reserved_push_back(std::forward <Args> (objs)...);
    }


    /* Reserve __n space in the back. */
    void reserve_back(size_t __n) {
        value_t *temp = alloc(size() + __n);
        for(size_t i = 0 ; i < size() ; ++i) {
            this->construct(temp + i,std::move(head[i]));
        }
        this->~list_array();
        tail = temp + size();
        head = temp;
    }
    /* Reserve __n space in the front. */
    void reserve_front(size_t __n) {
        value_t *temp = alloc(size() + __n);
        temp += __n;
        for(size_t i = 0 ; i < size() ; ++i) {
            this->construct(temp + i,std::move(head[i]));
        }
        this->~list_array();
        tail = temp + size();
        head = temp;
    }


  public:
    /* Construct a new empty %array. */
    list_array() : head(nullptr),tail(nullptr) {}
    /* Destroy all the elements and deallocate the space. */
    ~list_array() { this->destroy_n(head,size()); dealloc(); }

    /* Construct a new %array from an initializer_list. */
    list_array(std::initializer_list <value_t> __l) 
        : list_array(__l.size()) {
        for(auto &iter : __l) { this->construct(tail++,std::move(iter)); }
    }

    /**
     * @brief Construct a new %array filled with given length and element.
     * 
     * @param __n The initial length of the %array.
     * @param obj The element to fill the %array.
     */
    list_array(size_t __n,const value_t &obj) 
        : list_array(__n) {
        while(__n--) { this->construct(tail++,obj); }
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
    list_array(const list_array &rhs) 
        : list_array(rhs.size()) {
        for(const auto &iter : rhs) { this->construct(tail++,iter);}
    }

    /* Move assign by taking away pointers in constant time. */
list_array &operator = (list_array &&rhs) {
        if(this != &rhs) {
            this->~list_array();
            head = rhs.head;
            tail = rhs.tail;
            rhs.head = rhs.tail = nullptr;
        }
        return *this;
    }
    /* Copy assign by copying data in linear time. */
    list_array &operator = (const list_array &rhs) {
        if(this != &rhs) { copy_range(rhs.begin(),rhs.size());}
        return *this;
    }


    /* Swap the content of two %array in constant time. */
    list_array &swap(list_array &rhs) {
        std::swap(head,rhs.head);
        std::swap(tail,rhs.tail);
        return *this;
    }
    /* Swap the content of two %array in constant time. */
    friend void swap(list_array &lhs,list_array &rhs) {
        std::swap(lhs.head,rhs.head);
        std::swap(lhs.tail,rhs.tail);
    }
    

  public:
    /* Count of elements within the %array. */
    constexpr inline size_t size() const { return tail - head; }

    /* Test whether the %array is empty */
    constexpr inline bool empty()  const { return head == tail; }

    /* Doing nothing to the %array. */
    constexpr inline void push_back () {}
    /* Doing nothing to the %array. */
    constexpr inline void push_front() {}

    /**
     * @brief Push a sequnce of elements to the back of the %array.
     * 
     * @param objs The objects pushed back to initialize the element.
     * @attention This operation is costly linear complexity with respect to
     * (size() of the %array + count of arguments), 
     * multiplied by the time of moving one element.
     */
    template <class ...Args>
    void push_back(Args &&...objs) {
        reserve_back(sizeof...(objs));
        reserved_push_back(std::forward <Args> (objs)...);
    }

    /**
     * @brief Push a sequnce of elements to the front of the %array.
     * 
     * @param objs The objects pushed front to initialize the element.
     * @attention This operation is costly linear complexity with respect to
     * (size() of the %array + count of arguments), 
     * multiplied by the time of moving one element.
     */
    template <class ...Args>
    void push_front(Args &&...objs) {
        reserve_front(sizeof...(objs));
        reserved_push_front(std::forward <Args> (objs)...);
    }

    /**
     * @brief Construct one element after the back of the %array.
     * 
     * @param obj The object pushed back to initialize the element. 
     * @attention This operation is costly linear complexity with respect to
     * size(), multiplied by the time of moving one element.
     */
    template <class ...Args>
    void emplace_back(Args &&...objs) {
        reserve_back(1);
        this->construct(tail++,std::forward <Args> (objs)...);
    }
    /**
     * @brief Construct one element before the front of the %array.
     * 
     * @param obj The object pushed front to initialize the element. 
     * @attention This operation is costly linear complexity with respect to
     * size(), multiplied by the time of moving one element.
     */
    template <class ...Args>
    void emplace_front(Args &&...objs) {
        reserve_front(1);
        this->construct(--head,std::forward <Args> (objs)...);
    }

    /**
     * @brief Pop out elements from the back of the %array.
     * 
     * @param __n Count of elements poped from the back.
     * @attention This operation is costly linear complexity with respect to
     * size(), multiplied by the time of destroying and moving one element.
     */
    void pop_back(size_t __n = 1) {
        if(__n == 0) return; // Doing nothing!

        __n = size() - __n; // Now __n is count of elements remained.
        value_t *temp = alloc(__n);
        for(size_t i = 0 ; i < __n ; ++i) {
            this->construct(temp + i,std::move(head[i]));
        }
        this->~list_array();
        tail = (head = temp) + __n;
    }
    /**
     * @brief Pop out elements from the front of the %array.
     * 
     * @param __n Count of elements poped from the front.
     * @attention This operation is costly linear complexity with respect to
     * size(), multiplied by the time of destroying and moving one element.
     */
    void pop_front(size_t __n = 1) {
        if(__n == 0) return; // Doing nothing!

        value_t *temp = alloc(size() - __n);
        for(size_t i = __n ; i < size() ; ++i) {
            this->construct(temp + i - __n,std::move(head[i]));
        }
        this->~list_array();
        tail = temp + size() - __n;
        head = temp;
    }

    /* Clear the %array. */
    void clear() { 
        this->~list_array(); 
        head = tail = nullptr; 
    }

    /**
     * @brief Resize the %array to __n and assign all the elements by val. 
     * 
     * @param __n The new size of the %array.
     * @param val The object to assign the value.(Default value_t())
     * @attention Linear complexity with respect to __n,
     * multiplied by the construction time of one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    void assign(size_t __n,const value_t &val = value_t()) {
        if(__n == size()) {
            for(auto &iter : *this) iter = val;
        } else {
            this->~list_array();
            tail = head = alloc(__n);
            while(__n--) this->construct(tail++,val);
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
    void resize_back(size_t __n) {
        if(__n == size()) return; // Doing nothing!

        if(__n < size()) { pop_back(size() - __n); } 
        else { /* Now, __n > size() */
            __n -= size();
            reserve_back(__n);
            while(__n--) this->construct(tail++);
        }
    }
    /**
     * @brief Resize the %array to __n.
     * The original data with reverse index smaller than __n won't be touched.  
     * If __n is greater than size(), elements will be appended to the front.
     * These new elements will be assigned by default construction function.
     * 
     * @param __n The new size of the %array.
     * @attention Linear complexity with respect to __n ,
     * multiplied by the construction time of one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    void resize_front(size_t __n) {
        if(__n == size()) return; // Doing nothing!

        if(__n < size()) { pop_front(size() - __n); }
        else { /* Now, __n > size() */
            __n -= size();
            reserve_front(__n);
            while(__n--) this->construct(--head);
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
    void resize_back(size_t __n,const value_t &val) {
        if(__n == size()) return; // Doing nothing!

        if(__n < size()) { pop_back(size() - __n); } 
        else { /* Now, __n > size() */
            __n -= size();
            reserve_back(__n);
            while(__n--) this->construct(tail++,val);
        }
    }
    /**
     * @brief Resize the %array to __n.
     * The original data with reverse index smaller than __n won't be touched.  
     * If __n is greater than size(), elements will be appended to the front.
     * These new elements will be assigned by val.
     * 
     * @param __n The new size of the %array.
     * @param val The object to assign the new value.
     * @attention Linear complexity with respect to __n ,
     * multiplied by the construction time of one element.
     * Note that there might be an additional time cost linear to the 
     * elements destroyed.
     */
    void resize_front(size_t __n,const value_t &val) {
        if(__n == size()) return; // Doing nothing!

        if(__n < size()) { pop_front(size() - __n); } 
        else { /* Now, __n > size() */
            __n -= size();
            reserve_front(__n);
            while(__n--) this->construct(--head,val);
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
    inline void copy_range(Iterator first,Iterator last) {
        return copy_range(first,last,last - first);
    }


    /**
     * @brief Copy elements from a range [first,last).
     * The number of elements in the range should be exactly __n,
     * or unexpected error may happen.
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
        if(__n == size()) {
            tail = head;
            while(__n--) *(tail++) = *(first++);  
        } else {
            this->~list_array();
            tail = head = alloc(__n);
            while(__n--) this->construct(tail++,*(first++));
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
    inline void move_range(Iterator first,Iterator last) {
        return move_range(first,last,last - first);
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
    void move_range(Iterator first,Iterator last,size_t __n) {
        if(__n == size()) {
            tail = head;
            while(__n--) *(tail++) = std::move(*(first++));   
        } else {
            this->~list_array();
            tail = head = alloc(__n);
            while(__n--) this->construct(tail++,std::move(*(first++)));
        }
    }

    /**
     * @brief Repeat the content of the array for __n times.
     * The size will become __n * size.
     * 
     * @param __n The times the content is repeated.(Default 2)
     * @return Reference to the array.
     * @attention Linear time complexity with respect to size * __n,
     * mutliplied by the time of constructing one element.
     * Note that there is an additional time cost linear to size.
     */
    list_array &repeat(size_t __n = 2) {
        if(__n == 0 || empty()) { clear(); return *this; }
        if(__n == 1) return *this;

        value_t *temp = alloc(size() * __n);
        size_t i = 0;
        while(--__n) {
            for(size_t j = 0 ; j < size() ; ++j) { 
                this->construct(temp + (i++),head[j]); 
            }
        }
        for(size_t j = 0 ; j < size() ; ++j) { 
            this->construct(temp + (i++),std::move(head[j])); 
        }

        this->~list_array();
        tail = (head = temp) + i;
    }

  public:

    friend list_array operator + (const list_array &lhs,const list_array &rhs) {
        list_array ans(lhs.size() + rhs.size());
        for(const auto &iter : lhs) ans.construct(ans.tail++,iter);
        for(const auto &iter : rhs) ans.construct(ans.tail++,iter);
        return ans;
    }
    list_array &operator += (const list_array &rhs) {
        if(this == &rhs) return repeat(2);
        if(!rhs.empty()) {
            reserve_back(rhs.size());
            for(const auto &iter : rhs) construct(tail++,iter);
        }
        return *this;
    }
    list_array &operator += (list_array &&rhs) {
        if(this == &rhs) return repeat(2);
        if(rhs.empty())  return *this;
        if(empty())      return *this = std::move(rhs);

        reserve_back(rhs.size());
        for(auto &iter : rhs) this->construct(tail++,std::move(iter));

        return *this;
    }

    friend list_array operator * (const list_array &lhs,size_t __n) {
        if(__n == 0 || lhs.empty()) return list_array();
        if(__n == 1)                return lhs;
        list_array ans(lhs.size() * __n);
        while(__n--) {
            for(const auto &iter : lhs) ans.construct(ans.tail++,iter);
        }
        return ans;
    }
    inline friend list_array operator * (size_t __n,const list_array &rhs) {
        return rhs * __n;
    }
    inline list_array &operator *=(size_t __n) {
        return repeat(__n);
    }

  public:

    /* Return the pointer to the first element. */
    inline value_t *data() { return head; }
    /* Return the pointer to the first element. */
    inline const value_t *data() const { return head; }
    /* Subscript access to the data in the %array.  */
    inline value_t &operator [] (size_t __n) { return head[__n]; }
    /* Subscript access to the data in the %array.  */
    inline const value_t &operator [] (size_t __n) const { return head[__n]; }

    /* Reference to the first element. */
    inline value_t &front() {return *begin();}
    /* Reference to the  last element. */
    inline value_t &back()  {return *--end();}
    /* Const reference to the first element. */
    inline const value_t &front() const {return *cbegin();}
    /* Const reference to the  last element. */
    inline const value_t &back()  const {return *--cend();}

    using iterator       = RandomAccess::iterator       <value_t>;
    using const_iterator = RandomAccess::const_iterator <value_t>;

    /* Iterator to the first element. */
    inline iterator begin() { return head; }
    /* Iterator to the one past last element. */
    inline iterator end()   { return tail; }

    /* Const_iterator to the first element. */
    inline const_iterator begin()  const {return head;}
    /* Const_iterator to one past the last element. */
    inline const_iterator end()    const {return tail;}
    /* Const_iterator to the first element. */
    inline const_iterator cbegin() const {return head;}
    /* Const_iterator to one past the last element. */
    inline const_iterator cend()   const {return tail;}

};




}




#endif
