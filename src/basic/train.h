#ifndef _TICKET_TRAIN_H_
#define _TICKET_TRAIN_H_


#include "utility.h"
#include "time.h"


namespace dark {

/* Train info holder. */
struct train {
    realtrainID_t tid;
    
    number_t stat_num;
    number_t seat_num;

    calendar start;
    calendar sale_beg; /* Begin time on sale. */
    calendar sale_end; /*  End  time on sale. */
    trainType_t type;  /* Train Type. */

    stationName_t    names[kSTATION];
    prices_t         price[kSTATION];

    travel_t travel_time[kSTATION];
    stopov_t stopov_time[kSTATION];

    void copy(const char *__i,const char *__n,
              const char *__m,const char *__s,
              const char *__p,const char *__x,
              const char *__t,const char *__o,
              const char *__d,const char *__y) {
        tid      = __i;
        stat_num = to_unsigned_integer <number_t> (__n);
        seat_num = to_unsigned_integer <number_t> (__m);
        start    = to_time(__x);
        get_date(sale_beg,sale_end,__d);
        type     = to_type(__y);

        get_strings (names,__s);
        get_integers(price,__p);
        get_integers(travel_time,__t);
        get_integers(stopov_time,__o);
    }

};

static_assert(sizeof(train) <= 4096,"fault");

/* Seats info holder. */
struct seats {
    int count[92][kSTATION]; /* Count on given date between given stations. */
};

static_assert(sizeof(seats) <= 9 * 4096,"fault");

/* Train state holder. */
struct train_state {
    size_t __hash; /* Inner hash code. */

    int index_data; /* Index of train data.    */
    int index_seat; /* Index of train seat. */

    bool is_released() { return index_seat != -1; }
};


}

namespace std {

template <>
struct hash <::dark::train_state> {
    size_t operator()(const ::dark::train_state &__t) 
    const noexcept { return __t.__hash; }
};

template <>
struct equal_to <::dark::train_state> {
    size_t operator()(const ::dark::train_state &lhs,
                      const ::dark::train_state &rhs) 
    const noexcept { return lhs.__hash == rhs.__hash; }
};


};

#endif
