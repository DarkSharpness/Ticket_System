#ifndef _TICKET_TIME_H_
#define _TICKET_TIME_H_

#include "utility.h"


namespace dark {

/* Day count of each month. */
constexpr unsigned day_map[] = {
    0,31,59,90,120,151,181,212,243,273,304,334
};
/* Minute count of one hour. */
constexpr unsigned hour_minute = 60;
/* Minute count of one day. */
constexpr unsigned day_minute = 60 * 24;

/* Custom calendar type recording total minute. */
enum calendar : unsigned {};

calendar operator + (calendar x,calendar y)
{ return (calendar)((unsigned)x + (unsigned)y); }

calendar operator - (calendar x,calendar y)
{ return (calendar)((unsigned)x - (unsigned)y); }

calendar &operator += (calendar &x,calendar y) {
    (unsigned &)x += (unsigned)y;
    return x;
}

calendar &operator -= (calendar &x,calendar y) {
    (unsigned &)x -= (unsigned)y;
    return x;
}


}

namespace dark {

/* Convert a c-string time into calendar recording date. */
calendar to_date(const char *__t) {
    return (calendar)( 
        ( 
            day_map[(__t[0] ^ '0') * 10 + (__t[1] ^ '0') * 1 - 1]
          + (__t[3] ^ '0') * 10  + (__t[4] ^ '0') * 1 
        ) * day_minute
    );
}

/* Convert a c-string time into calendar recording time. */
calendar to_time(const char *__t) {
    return (calendar)(
        (__t[0] ^ '0') * 600 + (__t[1] ^ '0') * 60
      + (__t[3] ^ '0') * 10  + (__t[4] ^ '0') * 1
    );
}

/* Get date for begin and end range. */
void get_date(calendar &__beg,calendar &__end,const char *__t) {
    __beg = to_date(__t + 0);
    __end = to_date(__t + 6);
}


}


#endif
