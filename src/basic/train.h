#ifndef _TICKET_TRAIN_H_
#define _TICKET_TRAIN_H_

#include "utility.h"
#include "Dark/trivial_array"
#include <tuple>


namespace dark {


/* Train info holder. */
struct train {
    realtrainID_t tid; /* TrainID string. */
    
    number_t stat_num; /* Station number. */
    number_t seat_num; /* Seat number. */

    calendar sale_beg; /* Begin time on sale (Containing hour-minute) */
    calendar sale_end; /*  End  time on sale (Containing hour-minute) */

    stationName_t    names[kSTATION]; /* Name of given station. */
    prices_t         price[kSTATION]; /*  Prefix sum of price. */

    travel_t arrival_time[kSTATION]; /* 1 ~ station_num. */
    stopov_t leaving_time[kSTATION]; /* 1 ~ station_num - 1. */

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

        get_integers(price + 1,__p);
        get_integers(arrival_time + 1,__t);
        get_integers(leaving_time + 1,__o);
        price[0] = arrival_time[0] = leaving_time[0] = 0;

        int i = 0;
        while(++i != stat_num) {
            price       [i] += price       [i - 1];
            arrival_time[i] += leaving_time[i - 1];
            leaving_time[i] += arrival_time[  i  ];
        } --i; /* Now i = stat_num - 1 */
        leaving_time[i] = arrival_time[i];
    }

    /* Return the hour-minute starting time.*/
    int time() const noexcept { return calendar_to_time(sale_beg); }

    /* Find the station number. */
    std::pair <int,int> find(const char *__s,const char *__t)
    const noexcept {
        int i = 0;
        while(i < stat_num && strcmp(names[i].base(),__s)) ++i;
        int j = i + 1;
        while(j < stat_num && strcmp(names[j].base(),__t)) ++j;
        return {i,j};
    }


};
static_assert(sizeof(train) <= 4096,"fault");


/* Seats info holder. */
struct seats {
    seat_info_t count[DURATION]; /* Count on given date between given stations. */
};
static_assert(sizeof(seats) <= 9 * 4096,"fault");


/* Train state holder. */
struct train_state {
    size_t __hash; /* Inner hash code. */

    short train_index; /* Train index. */
    short seats_index; /* Seats index. */

    struct {
        unsigned __beg : 7;
        unsigned __end : 7;
        unsigned count : 18;
    };

    /* As it means...... */
    bool is_released() const noexcept { return seats_index >= 0; }
};
static_assert(sizeof(train_state) == 16);


/* Preview data of a train. */
struct train_view {
    struct { /* Pack of basic info. */
        short    index;   /* Index of seat data.      */ 
        char     start;   /* Number of start station. */
        char     final;   /* Number of final station. */
        prices_t price;   /* Prefix price (from begin).   */
    };

    travel_t arrival; /* Arrival time (from begin).   */
    travel_t leaving; /* Leaving time (from begin).   */
    travel_t  _time;  /* Start time in one day(00:00).*/
    char      __beg;  /* Relative train begin day.    */
    char      __end;  /* Relative train  end  day.    */

    /* Judge whether a relative day is (from 06-01) in the range. */
    inline bool out_of_range(int day) const noexcept {
        day -= travel_day();
        return day < __beg || __end < day;
    }

    /* Whether the station is terminal station. */
    bool is_terminal() const noexcept { return start == final; }

    /* Whether the station is starting station. */
    bool is_starting() const noexcept { return !start; }

    /* Return the leaving time from begin day 00:00. */
    int leaving_time() const noexcept { return leaving + _time; }

    /* Return the arrival time from begin day 00:00. */
    int arrival_time() const noexcept { return arrival + _time; }

    /* The day traveling from begin to current station. */
    int travel_day() const noexcept
    { return calendar_to_day(leaving_time()); }

};
static_assert(sizeof(train_view) == 16);


/* Preview of a station (in query transfer). */
struct station_views : public dark::trivial_array <train_view> {
    size_t __hash; /* Inner hash code. */
    station_views() = default;
    station_views(size_t __h) noexcept : __hash(__h) {}
};


/* Preview of a transfer. */
struct transfer_view {
    stationName_t name;   /* Station name of the mid.   */
    calendar leaving_beg; /* Absolute leaving from beg. */
    calendar arrival_end; /* Absolute arrival  at  end. */
    travel_t interval[2]; /* Interval time between station. */
    char        __dep[2]; /* Depature day of begin station. */

    struct { /* Pack of basic info. */
        short    index;   /* Index of seat data.        */ 
        char     start;   /* Number of start station.   */
        char     final;   /* Number of final station.   */
        prices_t price;   /* Prefix price (from begin). */
    } __[2]; /* Value holder. */

    /* Absolute leaving from beg. */
    calendar arrival_mid() const noexcept
    { return leaving_beg + interval[0]; }

    /* Absolute arrival  at  mid.  */
    calendar leaving_mid() const noexcept
    { return arrival_end - interval[1]; }

    /* Total time from begin to end. */
    int sum() const noexcept 
    { return arrival_end - leaving_beg; }
    /* Total cost from begin to end. */
    int cost()     const noexcept 
    { return __[0].price + __[1].price; }
    
    /* Copy the data from two train_view. */
    inline void copy(const train_view &__v,
                     const train_view &__t) {
        memcpy(__ + 0,&__v,sizeof(__[0]));
        memcpy(__ + 1,&__t,sizeof(__[1]));
    }

};
static_assert(sizeof(transfer_view) == 64);

struct train_unit {
    short index; /* Index of seats.   */
    short __dep; /* The depature day. */
};
static_assert(sizeof(train_unit) == 4);


}

namespace dark {

template <>
struct Compare <train_view> {
    int operator()(const train_view &lhs,const train_view &rhs) 
    const noexcept {
        return lhs.index < rhs.index ? -1 : rhs.index < lhs.index;
    }
};

template <>
struct Compare <train_unit> {
    int operator()(const train_unit &lhs,const train_unit &rhs) 
    const noexcept {
        return lhs.index == rhs.index ?
               lhs.__dep - rhs.__dep : lhs.index - rhs.index;
    }
};



/* Write the info of a train and its seat. */
void writeline(std::tuple <train*,int *,int> __p) {
    auto [__t,__s,__d] = __p; /* Train || Seats || Starting time. */
    if(!__t) return (void) puts("-1");
    dark::writeline(__t->tid,__t->type());
    int i = 0;
    dark::writeline(
        __t->names[i],
            "xx-xx xx:xx ->",
            (time_wrapper) {
                __d + 
                __t->leaving_time[i]
            },
            __t->price[i],
            __s ? __s[i] : __t->seat_num
        );
    while(++i != __t->stat_num - 1) { 
        dark::writeline(
            __t->names[i],
            (time_wrapper) {
                __d + 
                __t->arrival_time[i]
            },
            "->",
            (time_wrapper) {
                __d + 
                __t->leaving_time[i]
            },
            __t->price[i],
            __s[i]
        );
    }
    dark::writeline(
        __t->names[i],
        (time_wrapper) {
            __d +
            __t->arrival_time[i]
        },
        "-> xx-xx xx:xx",
        __t->price[i],
        'x'
    );
};



}


namespace std {

template <>
struct hash <::dark::train_state> {
    size_t operator()(const ::dark::train_state &__t) 
    const noexcept { return __t.__hash; }
};

template <>
struct hash <::dark::station_views> {
    size_t operator()(const ::dark::station_views &__t) 
    const noexcept { return __t.__hash; }
};

template <>
struct equal_to <::dark::train_state> {
    size_t operator()(const ::dark::train_state &lhs,
                      const ::dark::train_state &rhs)
    const noexcept { return lhs.__hash == rhs.__hash; }
};

template <>
struct equal_to <::dark::station_views> {
    size_t operator()(const ::dark::station_views &lhs,
                      const ::dark::station_views &rhs) 
    const noexcept { return lhs.__hash == rhs.__hash; }
};

};


#endif
