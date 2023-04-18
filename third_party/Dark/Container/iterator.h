#ifndef _DARK_ITERATOR_H_
#define _DARK_ITERATOR_H_

#include <version>
#include <type_traits>
#include <cstddef>

/* Function part. */
namespace dark::RandomAccess {

/* Difference Type */
using diff_t = std::ptrdiff_t;

/* Advance forward. */
template <class T>
inline void advance_ptr(T *&__p,std::true_type)
noexcept { ++__p; }

/* Advance backward. */
template <class T>
inline void advance_ptr(T *&__p,std::false_type)
noexcept { --__p; }

/* Advance forward __n steps. */
template <class T>
inline void advance_ptr(T *&__p,diff_t __n,std::true_type)
noexcept { __p += __n; }

/* Advance backward __n steps. */
template <class T>
inline void advance_ptr(T *&__p,diff_t __n,std::false_type)
noexcept { __p -= __n; }

/* Normal distancing 2 pointers. */
template <class T>
inline diff_t distance_ptr(const T *lhs,const T *rhs,std::true_type) 
noexcept { return lhs - rhs; }

/* Reverse distancing 2 pointers. */
template <class T>
inline diff_t distance_ptr(const T *lhs,const T *rhs,std::false_type) 
noexcept { return rhs - lhs; }



/**
 * @brief Advance toward given diretion.
 * 
 * @tparam dir || 0 if backward || 1 if forward ||
 * @param __p Pointer to advance.
 * @return The new pointer.
 */
template <bool dir,class T>
inline void advance(T *&__p) noexcept
{ return advance_ptr(__p,std::bool_constant <dir>()); }

/**
 * @brief Advance toward diretion by __n steps.
 * 
 * @tparam dir || 0 if backward || 1 if forward ||
 * @param __p Pointer to advance.
 * @param __n The distance to advance.
 * @return The new pointer.
 */
template <bool dir,class T>
inline void advance(T *&__p,diff_t __n) noexcept
{ return advance_ptr(__p,__n,std::bool_constant <dir>()); }

template <bool dir,class T>
inline diff_t distance(const T *lhs,const T *rhs) noexcept
{ return distance_ptr(lhs,rhs,std::bool_constant <dir>()); }



}

/* Class part. */
namespace dark::RandomAccess {

template <class T,bool is_const,bool dir>
class iterator_base {
  private:
    using U = std::conditional_t <is_const,const T,T>; 
    using pointer = U *;
    pointer ptr;
  public:
    iterator_base(pointer __p = nullptr) noexcept : ptr(__p) {}
    iterator_base(const iterator_base <T,false,dir> &rhs)
    noexcept : ptr(rhs.ptr) {}

    iterator_base &operator = (const iterator_base <T,false,dir> &rhs) 
    noexcept { ptr = rhs.ptr; return *this; }

    pointer base() const noexcept { return ptr; }

    iterator_base &operator ++ (void) noexcept
    { advance <dir> (ptr);  return *this; }

    iterator_base &operator -- (void) noexcept 
    { advance <!dir> (ptr); return *this; }

    iterator_base &operator += (diff_t __n) noexcept 
    { advance <dir> (ptr,__n);  return *this; }

    iterator_base &operator -= (diff_t __n) noexcept
    { advance <!dir> (ptr,__n); return *this; }

    /* Non-memeber function part.*/

    friend iterator_base operator ++ (iterator_base &lhs,int)  
    noexcept { iterator_base tmp = lhs; ++lhs; return tmp; }
    friend iterator_base operator -- (iterator_base &lhs,int)  
    noexcept { iterator_base tmp = lhs; --lhs; return tmp; }

    friend iterator_base operator + (iterator_base lhs,diff_t __n) 
    noexcept { return lhs += __n; }

    friend iterator_base operator + (diff_t __n,iterator_base rhs) 
    noexcept { return rhs += __n; }

    friend iterator_base operator - (iterator_base lhs,diff_t __n)
    noexcept { return lhs -= __n; }

    U &operator * (void) const { return *ptr; }
    U *operator ->(void) const { return  ptr; }

    /**
     * @brief Return reference to element __n steps in the direction.
     * 
     * @param __n The distance of targeted element.
     */
    U &operator [](diff_t __n) const {
        pointer tmp = ptr;
        advance <dir> (tmp,__n);
        return *tmp;
    }
};

/* Some other global functions. */

template <class T,bool k1,bool k2,bool dir>
diff_t operator - (const iterator_base <T,k1,dir> &lhs,
                   const iterator_base <T,k2,dir> &rhs) {
    return distance <dir> (lhs.base(),rhs.base());
}

template <class T,bool k1,bool k2,bool dir>
bool operator == (const iterator_base <T,k1,dir> &lhs,
                  const iterator_base <T,k2,dir> &rhs) {
    return lhs.base() == rhs.base();
}

template <class T,bool k1,bool k2,bool dir>
bool operator != (const iterator_base <T,k1,dir> &lhs,
                  const iterator_base <T,k2,dir> &rhs) {
    return lhs.base() != rhs.base();
}

/* Namespace for Random Access Iterators.  */

}

#endif
