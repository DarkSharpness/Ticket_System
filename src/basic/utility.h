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

constexpr size_t kSTATION = 100;

using username_t = string <24>; /* Username as the only marker ||  3Byte unused. */
using password_t = string <32>; /* Password of an account      ||  1Byte unused. */
using realname_t = string <16>; /* Real name of the user       ||  0Byte unused. */
using mailaddr_t = string <32>; /* The mail address            ||  1Byte unused. */

using privilege_t = char; /* Special privilege type. */

using realtrainID_t = string <24>; /* Train ID string.            || 3Byte unused. */
using stationName_t = string <32>; /* Station name of one station || 1Byte unused. */
using number_t      = unsigned int;   /* Range: 0 ~ kSTATION */
using prices_t      = unsigned int;   /* Range: 0 ~ 1e5      */
using travel_t      = unsigned short; /* Range: 0 ~ 1e4      */
using stopov_t      = unsigned short; /* Range: 0 ~ 1e4      */
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
void read_line(char *str) noexcept {
    do { *str = getchar(); } while(*(str++) != '\n');
}

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
T to_unsigned_integer(const char *__s) {
    static_assert(std::is_integral_v <T>,"T must be built-in integer type!");
    T ans = 0;
    while(*__s) ans = ans * 10 + (*(__s++) ^ '0');
    return ans;
}

/* Convert a c-string to trainType.  */
trainType_t to_type(const char *__s) { return *__s; }

/* Write a bool integer as 0/1. */
void writeline(bool x) { puts(x ? "0" : "-1" ); }

/* Convert a c-string into a string array. */
template <class string>
void get_strings(string *names,const char *__n) {
    do { /* Loop. */
        size_t i = 0;
        while(*__n && *__n != '|') (*names)[i++] = *(__n++); 
        ++names;
    } while(*(__n++));
}

/* Convert a c-string into an integer array. */
template <class integer>
void get_integers(integer *price,const char *__p) {
    static_assert(std::is_integral_v <integer>,"Must be integers!");
    do { /* Loop. */
        while(*__p && *__p != '|') {
            *price = *price * 10 + (*(__p++) ^ '0'); 
        } ++price;
    } while(*(__p++));
}

}



#endif
