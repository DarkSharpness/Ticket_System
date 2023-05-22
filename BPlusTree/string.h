#ifndef _DARK_BPLUS_STRING_H_
#define _DARK_BPLUS_STRING_H_

#include "utility.h"
#include <cstring>

namespace dark {

template <size_t N>
struct string {
    char str[N];

    string() noexcept { str[0] = '\0'; };

    string(const char *rhs) noexcept { strcpy(str,rhs); }

    string(const string &rhs) = default;

    string &operator = (const string &rhs) = default;

    string &operator = (const char *rhs) { strcpy(str,rhs); return *this; }

    char &operator [](size_t __n) noexcept { return str[__n]; }
    const char &operator [](size_t __n) const noexcept { return str[__n]; }

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

template <size_t __n>
void write(const string <__n> &str) { write(str.base()); }

size_t string_hash(const char *__s) noexcept {
    static size_t fix_random = rand();
    size_t __h = fix_random;
    while(*__s) { __h = __h * 137 + *(__s++); }
    return __h;
}

}

namespace std {

/* Custom String Hash. */
template <size_t __n>
struct hash <::dark::string <__n>> {
    size_t operator()(const ::dark::string <__n> &str)
    const noexcept { return ::dark::string_hash(str.base()); }
};



}

#endif
