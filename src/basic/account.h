#ifndef _TICKET_ACCOUNT_H_
#define _TICKET_ACCOUNT_H_

#include "utility.h"


namespace dark {

/* Account info holder. */
struct account {
    username_t user; /* Username. */
    password_t pswd; /* Password. */
    realname_t name; /* Realname. */
    mailaddr_t mail; /* MailAddr. */

    account() = default;

    account(const char *__u,const char *__p,const char *__n,
            const char *__m,privilege_t __g)
    noexcept : user(__u),pswd(__p),name(__n),mail(__m) 
    { level() = __g; login() = false; count() = 0; }

    void copy(const char *__u,const char *__p,const char *__n,
              const char *__m,privilege_t __g) noexcept {
        user = __u; pswd = __p; 
        name = __n; mail = __m;
        level() = __g;
    }

    /* Return count of order of the user. */
    short &count() noexcept
    { return *(short *)(user.base() + (sizeof(user) - sizeof(short))); }

    /* Return the user's level. */
    privilege_t &level() noexcept { return mail[sizeof(mail) - 1]; }

    /* Return whether the user have logged in. */
    char &login() noexcept { return pswd[sizeof(pswd) - 1]; }

    short count() const noexcept
    { return *(const short *)(user.base() + (sizeof(user) - sizeof(short))); }

    /* Return the user's level. */
    privilege_t level() const noexcept { return mail[sizeof(mail) - 1]; }

    /* Return whether the user have logged in. */
    bool login() const noexcept { return pswd[sizeof(pswd) - 1]; }
};

/* Total order info. */
struct order_t {
    stationName_t fr;    /* Departure.      */
    stationName_t to;    /* Terminal.       */

    int            :  0; /* Padding.        */
    short state    :  2; /* State of order. */
    short interval : 14; /* Interval time.  */
    short index    : 16; /* Index of seat.  */
    int            :  0; /* Padding.        */
    int    __dep   :  8; /* Depature day.   */
    int    count   : 24; /* Count of seats. */
    int            :  0; /* Padding.        */

    calendar leaving;    /* Leaving  time.  */
    prices_t price  ;    /* Sum of prices.  */

    char &start() noexcept { return fr[sizeof(fr) - sizeof(char)]; }
    char &final() noexcept { return to[sizeof(to) - sizeof(char)]; }

    bool is_success()  const noexcept { return state ==  1; }
    bool is_pending()  const noexcept { return state ==  0; }
    bool is_refunded() const noexcept { return state == -1; }

    void set_success()  noexcept { state =  1; }
    void set_pending()  noexcept { state =  0; }
    void set_refunded() noexcept { state = -1; }

    calendar leaving_time() const noexcept { return leaving; }
    calendar arrival_time() const noexcept { return leaving + interval; }

};
static_assert(sizeof(order_t) == 80);


/* Preview data of order. */
struct order_view {
    short       index;  /* Seat index.   */
    travel_t interval;  /* Travel time.  */
    prices_t    price;  /* Total price   */
    number_t seat_num;  /* Seat number.  */
    calendar  leaving;  /* Leaving time. */

    /* Absolute arrival time. */
    calendar arrival_time() { return leaving + interval; }
    /* Absolute leaving time. */
    calendar leaving_time() { return leaving; }

};
static_assert(sizeof(order_view) == 16);

/* Info of order in the bpt. */
struct order_info {
    int count : 31; /* Number in current user's order. */
    int avail :  1; /* Whether the ticket is refunded. */
    int index : 32; /* Index of the order.  */
};

enum class order_wrapper : int {
    REFUND  = -1,
    PENDING =  0,
    SUCCESS =  1,
};

}


namespace dark {

/* Only writeline will be used for accounts. */
void writeline(const account &__a) noexcept {
    dark::write(__a.user,
                __a.name,
                __a.mail);
    putchar(' ');
    dark::writeline_privilege(__a.level());
}

/* Only writeline will be used for accounts. */
void writeline(account *__a) noexcept {
    if(!__a) dark::writeline("-1");
    else     dark::writeline(*__a);
}

void write(order_wrapper __o) {
    switch(__o) {
        case order_wrapper::PENDING:
            return dark::write("[pending]");
        case order_wrapper::REFUND :
            return dark::write("[refunded]");
        case order_wrapper::SUCCESS:
            return dark::write("[success]");
        default: /* This can't happen */ ;
    }
}


template <>
struct Compare <account> {
    int operator ()(const account &,const account &) 
    const noexcept { return 0; }
};

template <>
struct Compare <order_info> {
    int operator ()(const order_info &lhs,const order_info &rhs) 
    const noexcept { return lhs.count - rhs.count; }
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
