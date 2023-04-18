#ifndef _DARK_ROUND_ARRAY_H_
#define _DARK_ROUND_ARRAY_H_


#include <memory>
#include <initializer_list>

namespace dark {

using value_t = long long;

class round_array : private std::allocator <value_t>  {
  protected:
    value_t *head;    // Head pointer
    value_t *tail;    // Tail pointer
    value_t *prefix;  // Prefix pointer
    value_t *suffix;  // Suffix pointer
    bool round;       // Whether rounded or not

    /* Allocate memory of __n elements. */
    value_t *alloc(size_t __n) { return this->allocate(__n); }
    /* Deallocate of the memory of head. */
    void dealloc() { this->deallocate(head,capacity()); }
    
    /* Destory __n elements */
    void destroy_n(value_t *pointer,size_t __n) {
        while(__n--) { this->destroy(pointer++); }
    }

    void destroy_in(value_t *ptr1,value_t *ptr2) {
        while(ptr1 != ptr2) this->destroy(ptr1++);
    }

    /* Expand elements to the back. */
    void expand(size_t __n) {
        value_t *temp = alloc(__n);
        if(round) {
            size_t i = 0;
            while(prefix != tail)
                this->construct(temp + i++,std::move(*(prefix++)));
            prefix = head;
            while(prefix != suffix)
                this->construct(temp + i++,std::move(*(prefix++)));           
            round = false;
            suffix = temp + i;
        } else {
            for(size_t i = 0 ; i < suffix - prefix ; ++i)
                this->construct(temp + i,std::move(prefix[i]));
            suffix += temp - prefix;
        }
        this->dealloc();
        tail = (head = prefix = temp) + __n;
    }

  public:
     /* Construct a new empty %array. */
    round_array() noexcept : round(false),
        head(nullptr),tail(nullptr),prefix(nullptr),suffix(nullptr) {}
    /* Destroy all the elements and deallocate the space. */
    ~round_array() noexcept {
        if(!round) this->destroy_in(prefix,suffix);
        else this->destroy_in(head,suffix),this->destroy_in(prefix,tail);
    }

    /* Construct a new %array from an initializer_list. */
    round_array(std::initializer_list <value_t> __l) 
        : round_array(__l.size()) {
        for(auto &iter : __l) { this->construct(suffix++,std::move(iter)); }
    }

    /* Construct a new %array with __n elements' space reserved. */
    round_array(size_t __n) : round(false) {
        tail = (head = prefix = suffix = alloc(__n)) + __n;
    }

    /**
     * @brief Construct a new %array filled with given length and element.
     * 
     * @param __n The initial length of the %array.
     * @param obj The element to fill the %array.
     */
    round_array(size_t __n,const value_t &obj) 
        : round_array(__n) {
        while(suffix != tail) { this->construct(tail++,obj); }
    }


  public:
    /* Return count of elements in the %array. */
    size_t size() const noexcept 
    {    return suffix - prefix + round * (tail - head); }
    /**
     * @brief Return count of elements the %array can hold 
     * before the next allocation.
     */
    size_t capacity() const noexcept 
    { return tail - head; }
    /* Test whether the %array is empty。 */
    bool empty() const noexcept 
    { return prefix == suffix && round == false; }

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
        if(size() == capacity()) {
            expand(size() << 1 | empty()); 
            this->construct(suffix,std::forward <U> (obj));
        } else {


        }
    }

    /**
     * @brief Push one element to the back of the %array.
     * 
     * @param obj The object pushed front to initialize the element. 
     * @attention Amortized constant time complexity,
     * multiplied by the construction time of one element.
     */
    template <class U>
    void push_front(U &&obj) {
        if(size() == capacity()) { reserve(size() << 1 | empty()); } 
        this->construct(tail - (++suffix),std::forward <U> (obj));
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
        if(size() == capacity()) { reserve(size() << 1 | empty()); }
        this->construct(head + (prefix++),std::forward <Args> (objs)...);
    }

    /**
     * @brief Construct one element before the front of the %array.
     * 
     * @param obj The object pushed front to initialize the element. 
     * @attention Amortized constant time complexity,
     * multiplied by the construction time of one element.
     */
    template <class ...Args>
    void emplace_front(Args &&...objs) {
        if(size() == capacity()) { reserve(size() << 1 | empty()); }
        this->construct(tail - (++suffix),std::forward <Args> (objs)...);
    }



};


}

#endif
