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

/* Maximum station  count. */
constexpr size_t kSTATION = 100;
/* Day count of each month. */
constexpr int day_map[] = {
    0,31,59,90,120,151,181,212,243,273,304,334
};


using username_t  = string <24>; /* Username as the only marker ||  3Byte unused. */
using password_t  = string <32>; /* Password of an account      ||  1Byte unused. */
using realname_t  = string <16>; /* Real name of the user       ||  0Byte unused. */
using mailaddr_t  = string <32>; /* The mail address            ||  1Byte unused. */
using privilege_t = char; /* Special privilege type. */


using realtrainID_t = string <24>; /* Train ID string.            || 3Byte unused. */
using stationName_t = string <32>; /* Station name of one station || 1Byte unused. */
using number_t      = int;   /* Range: 0 ~ kSTATION */
using prices_t      = int;   /* Range: 0 ~ 1e5      */
using calendar      = int;   /* Range: 0 ~ 60 * 24 * 365 */
using travel_t      = short; /* Range: 0 ~ 1e4      */
using stopov_t      = short; /* Range: 0 ~ 1e4      */
using trainType_t   = char;           /* Range: 'A' ~ 'Z'    */


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

}

/* Function and main classes part. */
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


/* Write a bool integer as 0/1. */
void writeline(bool x) noexcept { puts(x ? "0" : "-1" ); }


/* Convert a c-string into a string array. */
template <class string>
void get_strings(string *names,const char *__n) noexcept {
    do { /* Loop. */
        size_t i = 0;
        while(*__n && *__n != '|') (*names)[i++] = *(__n++); 
        ++names;
    } while(*(__n++));
}


/* Convert a c-string into an integer array. */
template <class integer>
void get_integers(integer *price,const char *__p) noexcept {
    static_assert(std::is_integral_v <integer>,"Must be integers!");
    do { /* Loop. */
        while(*__p && *__p != '|') {
            *price = *price * 10 + (*(__p++) ^ '0'); 
        } ++price;
    } while(*(__p++));
}


/* Convert a c-string time into calendar recording date. */
calendar date_to_calendar(const char *__t) noexcept {
    return (calendar)( 
        ( 
            day_map[(__t[0] ^ '0') * 10 + (__t[1] ^ '0') * 1 - 1]
          + (__t[3] ^ '0') * 10  + (__t[4] ^ '0') * 1 
        ) * 1440
    );
}

/* Convert a c-string time into calendar recording minute. */
calendar time_to_calendar(const char *__t) noexcept {
    return (calendar)(
        (__t[0] ^ '0') * 600 + (__t[1] ^ '0') * 60
      + (__t[3] ^ '0') * 10  + (__t[4] ^ '0') * 1
    );
}

/* Convert a calendar into day count.  */
unsigned to_day(calendar __c) noexcept { return __c / 1440; }

/* Convert a c-string time into calendar recording date relative to 06-01.*/
int to_relative_day(calendar __c) 
noexcept { return to_day(__c) - (day_map[5] + 1); }

/* Get date for begin and end range. */
void get_dates(calendar &__beg,calendar &__end,const char *__t) 
noexcept {
    __beg = date_to_calendar(__t + 0);
    __end = date_to_calendar(__t + 6);
}

}



#endif
