#ifndef _TICKET_TRAIN_H_
#define _TICKET_TRAIN_H_

#include "utility.h"


namespace dark {

/* Train info holder. */
struct train {
    realtrainID_t tid; /* TrainID string. */
    
    number_t stat_num; /* Station number. */
    number_t seat_num; /* Seat number. */

    calendar sale_beg; /* Begin time on sale (Containing hour-minute) */
    calendar sale_end; /*  End  time on sale (Containing hour-minute) */

    stationName_t    names[kSTATION];
    prices_t         price[kSTATION];

    travel_t travel_time[kSTATION];
    stopov_t stopov_time[kSTATION];

    char &type() noexcept { return tid[sizeof(tid) - 1]; }

    void copy(const char *__i,const char *__n,
              const char *__m,const char *__s,
              const char *__p,const char *__x,
              const char *__t,const char *__o,
              const char *__d,const char *__y)
    noexcept {
        tid      = __i;
        stat_num = to_unsigned_integer <number_t> (__n);
        seat_num = to_unsigned_integer <number_t> (__m);
        type()   = to_type(__y);

        get_dates(sale_beg,sale_end,__d,__x);
        get_strings (names,__s);
        get_integers(price,__p);
        get_integers(travel_time,__t);

        /* Ensure stopov_time[stat_num - 2] = 0. */
        stat_num ? *get_integers(stopov_time,__o) = 0 : stopov_time[0] = 0;
    }
};
static_assert(sizeof(train) <= 4096,"fault");


/* Seats info holder. */
struct seats {
    int count[DURATION][kSTATION]; /* Count on given date between given stations. */
};
static_assert(sizeof(seats) <= 9 * 4096,"fault");


/* Train state holder. */
struct train_state {
    size_t __hash; /* Inner hash code. */
    int index_data; /* Index of train data.    */
    int index_seat; /* Index of train seat. */

    bool is_released() const noexcept { return index_seat != -1; }
};

/* Preview data of a train. */
struct train_view {
    int train_index;

    number_t time; /* Prefix time. (No greater than 72  * 60 ) */
    prices_t cost; /* Prefix cost. (No greater than 1e5 * 100) */

    static_assert(sizeof(size_t) == 8,"Fail");
    static constexpr size_t unit[3] = {1,1e6,1e12};
 
    /* Real data. Stopov + begin time + end time.*/
    size_t time_data;

    /* Stop over time. */
    number_t stopov() const noexcept { return time_data % unit[1]; }
    /* Begin time. */
    calendar beg() const noexcept { return (time_data / unit[1]) % unit[1]; }
    /* End time. */
    calendar end() const noexcept { return time_data / unit[2];  }

    /* Set its time range and clear stop ov. */
    void set_range(calendar beg,calendar end) noexcept
    { time_data = beg * unit[1] + end * unit[2]; }

    /* Reset its stopover time. */
    void set_stop_ov(number_t stop_ov) noexcept
    { time_data += stop_ov - stopov(); }

    /* Add its time. */
    void add_time(number_t __t) { time += __t; } 

    /* Add its cost. */
    void add_cost(prices_t __c) { cost += __c; }

    /* Copy part of the data. */
    void copy(size_t __h,number_t __t,prices_t __c)
    noexcept { __hash = __h; time = __t; cost = __c; }

};


template <>
struct Compare <::dark::train_view> {
    int operator()(const ::dark::train_view &lhs,const ::dark::train_view &rhs) 
    const noexcept { return lhs.__hash < rhs.__hash ? -1 : rhs.__hash < lhs.__hash; }
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
