/**
 * @brief This file includes basic typedef info.
 * You shouldn't include this file directly. Try
 * to include header.h instead.
 * 
 */
#ifndef _TICKET_UTILITY_H_
#define _TICKET_UTILITY_H_

#include "../../BPlusTree/utility.h"
#include "../../BPlusTree/string.h"


/* Some declarations and defines. */
namespace dark {

/* Prefix sum of day count of each month. */
constexpr int day_map[] = {
    0,31,59,90,120,151,181,212,243,273,304,334,365
};
/* Maximum station  count. */
constexpr int kSTATION = 100;
/* Maximum duration from 06-01 to 08-31 */
constexpr int DURATION = day_map[8] - day_map[5];

using username_t  = string <24>;   /* Username as the only marker ||  3Byte unused. */
using password_t  = string <32>;   /* Password of an account      ||  1Byte unused. */
using realname_t  = string <16>;   /* Real name of the user       ||  0Byte unused. */
using mailaddr_t  = string <32>;   /* The mail address            ||  1Byte unused. */
using privilege_t = char;          /* Special privilege type. */
using seat_info_t = int[kSTATION]; /* Seat info of a train on on day. */

using realtrainID_t = string <24>; /* Train ID string.            || 3Byte unused. */
using stationName_t = string <32>; /* Station name of one station || 1Byte unused. */
using number_t      = int;   /* Range: 0 ~ kSTATION */
using prices_t      = int;   /* Range: 0 ~ 1e5      */
using calendar      = int;   /* Range: 0 ~ 60 * 24 * 365 */
using travel_t      = short; /* Range: 0 ~ 1e4      */
using stopov_t      = short; /* Range: 0 ~ 1e4      */
using trainType_t   = char;  /* Range: 'A' ~ 'Z'    */

/* Listing all commands. */
enum class command_t : unsigned char {
    A_US, /* add_user       || Normal         */
    L_IN, /* login          || Frequent       */
    L_OU, /* logout         || Frequent       */
    Q_PR, /* query_profile  || Super Frequent */
    M_PR, /* modify_profile || Frequent       */
    A_TR, /* add_train      || Normal         */
    D_TR, /* delete_train   || Normal         */
    R_TR, /* release_train  || Normal         */
    Q_TR, /* query_train    || Normal         */
    Q_TK, /* query_ticket   || Super Frequent */
    Q_TF, /* query_transfer || Normal         */
    B_TK, /* buy_ticket     || Super Frequent */
    Q_OR, /* query_order    || Frequent       */
    R_TK, /* refund_ticket  || Normal         */
    CLR_, /* clean          || Rarely         */
    EXIT  /* exit           || Rarely         */
};

/* Simple wrappers for output. */
struct time_wrapper { calendar data; };


/* A pair using only __width bits for second data. */
template <size_t __width>
struct compressor {
    int data; /* Real data. */

    static_assert(__width < sizeof(int) * 8,"Fail");

    static constexpr int MAX_RANGE = (1 << __width) - 1;

    compressor() = default;

    compressor(int x,int y = 0) noexcept : data((x << __width) | y) {}

    /* Return the first  element. */
    int first()  const noexcept { return data >> __width; }

    /* Return the second element.  */
    int second() const noexcept { return data & MAX_RANGE; }

    /* Set the data for first and second. */
    void set_data(int __i,int __r) noexcept { data = __i << __width | __r; }

    /* Set the first and clear the second. */
    void set_first(int __i) noexcept { data = __i << __width; }

    /* Update the first to a new value without changing the second. */
    void update_first(int __i) noexcept { data = (__i << __width) | second();}
};
static_assert(sizeof(compressor <8>) == 4);


}


/* Convert function part. */
namespace dark {

/* Convert a c-string to privilege.  */
privilege_t to_privilege(const char *__s) 
noexcept { return !__s[1] ? *__s ^ '0' : 10; }

/* Convert a c-string to unsigned_integer..  */
template <class T>
T to_unsigned_integer(const char *__s) noexcept {
    static_assert(std::is_integral_v <T>,"T must be built-in integer type!");
    T ans = 0;
    while(*__s) ans = ans * 10 + (*(__s++) ^ '0');
    return ans;
}

/* Convert a c-string to trainType.  */
trainType_t to_type(const char *__s) noexcept { return *__s; }

/* Convert a c-string date into day count. */
int date_to_day(const char *__t) noexcept {
    return ( 
            day_map[(__t[0] ^ '0') * 10 + (__t[1] ^ '0') * 1 - 1]
          + (__t[3] ^ '0') * 10  + (__t[4] ^ '0') * 1 - 1
        );
}

/* Convert a c-string date into calendar recording date. */
calendar date_to_calendar(const char *__t) 
noexcept { return date_to_day(__t) * 1440; }

/* Convert a c-string date count relative to 06-01.*/
int date_to_relative_day(const char *__t) 
noexcept { return date_to_day(__t) - day_map[5]; }

/* Convert a c-string time into calendar recording minute. */
calendar time_to_calendar(const char *__t) noexcept {
    return (calendar)(
        (__t[0] ^ '0') * 600 + (__t[1] ^ '0') * 60
      + (__t[3] ^ '0') * 10  + (__t[4] ^ '0') * 1
    );
}

/* Convert a calendar into day count.  */
constexpr int calendar_to_day(calendar __c) 
noexcept { return __c / 1440; }

/* Convert a calendar into to day count relative to 06-01.*/
constexpr int calendar_to_relative_day(calendar __c) noexcept
{ return calendar_to_day(__c) - day_map[5]; }


/* Convert a calendar into time in a day only. */
constexpr int calendar_to_time(calendar __c)
noexcept { return __c % 1440; }

/* Special convert functions. */

/* Get date for begin and end range. */
void get_dates(calendar &__beg,calendar &__end,const char *__t,const char *__x) 
noexcept {
    __beg = date_to_calendar(__t + 0);
    __end = date_to_calendar(__t + 6);
    calendar time = time_to_calendar(__x);
    __beg += time;
    __end += time;
}

/* Convert a c-string into a string array. */
template <class string>
void get_strings(string *names,const char *__n) noexcept {
    do { /* Loop. */
        size_t i = 0;
        while(*__n && *__n != '|') (*names)[i++] = *(__n++); 
        (*(names++))[i] = 0;
    } while(*(__n++));
}

/* Convert a c-string into an integer array. */
template <class integer>
void get_integers(integer *price,const char *__p) noexcept {
    static_assert(std::is_integral_v <integer>,"Must be integers!");
    do { /* Loop. */
        *price = 0;
        while(*__p && *__p != '|') {
            (*price) = (*price )* 10 + (*(__p++) ^ '0'); 
        } ++price;
    } while(*(__p++));
}

/* Merge a day and time to calendar. */
constexpr calendar merge_day_time(int __d,int __t)
noexcept { return __d * 1440 + __t; } 

/* Merge a relative day and time to calendar. */
constexpr calendar merge_relative_day_time(int __d,int __t)
noexcept { return (__d + day_map[5]) * 1440 + __t; } 


}


/* Inout function. */
namespace dark {

inline bool is_blank(char ch) noexcept { return ch == ' ' || ch == '\n' || ch == '\0'; }

/* Read a visible string (supporting UTF-8) and return its tail. */
char *read_string(char *str) noexcept {
    do {   *str = getchar(); } while( is_blank(*str)); 
    do { *++str = getchar(); } while(!is_blank(*str));
    return str;
}

/* Read a line of command. */
void read_line(char *str) noexcept 
{ do { *str = getchar(); } while(*(str++) != '\n'); }

/* Write input string out directly. */
void write_input() noexcept { 
    char ch;
    do { ch = getchar(); } while(is_blank(ch));
    do { putchar(ch); } while(!is_blank(ch = getchar()));
}

/* Write a line of privilege. */
void writeline_privilege(const privilege_t &__p) noexcept {
    if(__p != 10) { putchar(__p ^ '0'); }
    else {  putchar('1'); putchar('0'); }
    putchar('\n');
}

/* Writeline a bool integer [0 -> -1 || 1 -> 0]. */
void writeline(bool x) noexcept { puts(x ? "0" : "-1" ); }

/* Write two integer in the format of time. */
template <char __c>
void write_time(int x,int y) {
    putchar('0' | (x / 10) );
    putchar('0' | (x % 10));
    putchar(__c);
    putchar('0' | (y / 10));
    putchar('0' | (y % 10));
}

/* Write a time stored in the calendar. */
void write(time_wrapper __t) noexcept {
    /* x: day/minute || y: month/hour */

    int x = calendar_to_day(__t.data);
    int y = x / 29;
    if(day_map[y] < ++x) ++y;
    x -= day_map[y - 1];
    write_time <'-'> (y,x);

    putchar(' ');

    x  = calendar_to_time(__t.data);
    y  = x / 60;
    x %= 60;
    write_time <':'> (y,x);
}


}

#endif
