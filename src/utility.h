/**
 * @brief This file includes basic typedef info.
 * You shouldn't include this file directly. Try
 * to include header.h instead.
 * 
 */
#ifndef _TICKET_UTILITY_H_
#define _TICKET_UTILITY_H_

#include "../BPlusTree/utility.h"
#include "../BPlusTree/string.h"

/* Some declarations and defines. */
namespace dark {

constexpr size_t kSTATION = 100;

using username_t = string <24>; /* Username as the only marker. */
using password_t = string <32>; /* Password of an account. */
using realname_t = string <16>; /* Real name of the user. */
using mailaddr_t = string <32>; /* The mail address. */

enum privilege_t : unsigned char; /* Special privilege type. */
/* Map of privilege to string. */
const char privilege_table[][3] = {
    "0","1","2","3","4","5","6","7,","8","9","10"
};


using trainID_t  = string <24>;  /* Train ID string. */
using station_t  = string <32>;  /* Station name of one station. */

using stations_t = station_t[kSTATION];
using travel_t   = size_t[kSTATION - 1];
using stopov_t   = size_t[kSTATION - 2];

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

}



#endif
