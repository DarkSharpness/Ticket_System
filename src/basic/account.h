#ifndef _DARK_ACCOUNT_H_
#define _DARK_ACCOUNT_H_

#include "utility.h"

namespace dark {

/* Account */
struct account {
    username_t user; /* Username. */
    password_t pswd; /* Password. */
    realname_t name; /* Realname. */
    mailaddr_t mail; /* MailAddr. */
    
    account() = default;

    account(const char *__u,const char *__p,const char *__n,
            const char *__m,privilege_t __g)
    noexcept : user(__u),pswd(__p),name(__n),mail(__m) 
    { level() = __g; login() = false; }

    void copy(const char *__u,const char *__p,const char *__n,
              const char *__m,privilege_t __g) noexcept {
        user = __u; pswd = __p; 
        name = __n; mail = __m;
        level() = __g;
    }

    /* Return the user's level. */
    privilege_t &level() noexcept { return user[sizeof(user) - 1]; }

    /* Return whether the user have logged in. */
    char &login() noexcept { return user[sizeof(user) - 2]; }

    /* Return the user's level. */
    privilege_t level() const noexcept { return user[sizeof(user) - 1]; }

    /* Return whether the user have logged in. */
    bool login() const noexcept { return user[sizeof(user) - 2]; }
};

/* Write a line of privilege. */
void writeline_privilege(const privilege_t &__p) {
    if(__p != 10) { putchar(__p ^ '0'); }
    else {  putchar('1'); putchar('0'); }
    putchar('\n');
}

/* Only writeline will be used for accounts. */
void writeline(const account &__a) {
    dark::write(__a.user,
                __a.name,
                __a.mail);
    putchar(' ');
    dark::writeline_privilege(__a.level());
}

/* Only writeline will be used for accounts. */
void writeline(account *__a) {
    if(!__a) dark::writeline("-1");
    else     dark::writeline(*__a);
}

template <>
struct Compare <account> {
    int operator()(const account &lhs,const account &rhs) 
    const noexcept { return 0; }
};

}

namespace std {

template <>
struct hash <::dark::account> {
    size_t operator ()(const ::dark::account &lhs) 
    const noexcept { return ::dark::string_hash(lhs.user.base()); }
};

template <>
struct equal_to <::dark::account> {
    bool operator ()(const ::dark::account &lhs,const ::dark::account &rhs) 
    const noexcept { return lhs.user == rhs.user;  }
};


}



#endif
