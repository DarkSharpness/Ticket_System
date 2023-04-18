#ifndef _DARK_RANDOM_H_
#define _DARK_RANDOM_H_

#include <cstdlib>

namespace dark {


template <class integer>
integer random_basic() {
    char *__p = new char;
    integer ans = reinterpret_cast <size_t> (__p);
    delete __p;
    return ans;
}

template <class integer>
inline integer random() = delete;


template <>
inline int random() { return random_basic <int>(); }

template <>
inline long long random() { return random_basic <long long>(); }

template <class integer>
inline integer random_in(const integer &lhs,const integer &rhs) {
    return (random <integer> () ^ 114514) % (rhs - lhs) + lhs;
}




}



#endif

