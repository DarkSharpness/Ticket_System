#ifndef _DARK_BPLUS_STRING_H_
#define _DARK_BPLUS_STRING_H_

#include "utility.h"

namespace dark {

template <size_t __n>
struct string {
    char str[__n];

    string() noexcept { str[0] = '\0'; };

    string(const char *rhs) noexcept { strcpy(str,rhs); }

    string(const string &rhs) = default;

    string &operator = (const string &rhs) = default;

    const char *base() const noexcept { return str; }
};

template <size_t __n>
inline bool operator < (const string <__n> &lhs,const string <__n> &rhs) 
noexcept { return strcmp(lhs.base(),rhs.base()) < 0; }

template <size_t __n>
inline bool operator > (const string <__n> &lhs,const string <__n> &rhs) 
noexcept { return strcmp(lhs.base(),rhs.base()) > 0; }

template <size_t __n>
inline bool operator <= (const string <__n> &lhs,const string <__n> &rhs) 
noexcept { return strcmp(lhs.base(),rhs.base()) <= 0; }

template <size_t __n>
inline bool operator >= (const string <__n> &lhs,const string <__n> &rhs) 
noexcept { return strcmp(lhs.base(),rhs.base()) >= 0; }

template <size_t __n>
inline bool operator == (const string <__n> &lhs,const string <__n> &rhs) 
noexcept { return strcmp(lhs.base(),rhs.base()) == 0; }

template <size_t __n>
inline bool operator != (const string <__n> &lhs,const string <__n> &rhs) 
noexcept { return strcmp(lhs.base(),rhs.base()) != 0; }

template <size_t __n>
struct Compare <string <__n>> {
    int operator ()(const string <__n> &lhs,const string <__n> &rhs)
    const noexcept {  return strcmp(lhs.base(),rhs.base()); }
};

}

#endif
