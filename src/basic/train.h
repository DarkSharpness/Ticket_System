#ifndef _TICKET_TRAIN_H_
#define _TICKET_TRAIN_H_

#include "utility.h"


namespace dark {

/* Use 19 bit to store index and 13 bit to store time.   */
struct compressor {
    int data; /* Real data. */

    compressor() = default;
    compressor(int x,int y = 0) noexcept : data((x << 13) | y) {}

    static constexpr int MAX_TIME = (1 << 13) - 1;

    /* Index of train data. */
    int index() const noexcept { return data >> 13; }

    /* A time. (Smaller than max_range.)  */
    int time () const noexcept { return data & MAX_TIME; }

    /* Set the data for index and time. */
    void set_data(int __i,int __r) noexcept { data = __i << 13 | __r; }

    /* Set the index and clear time. */
    void set_index(int __i) noexcept { data = __i << 13; }

    /* Update index to a new value. */
    void update_index(int __i) noexcept { data = (__i << 13) | time();}
};

static_assert(sizeof(compressor) == 4);



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

    compressor index_data;
    compressor index_seat;

    /* Return the index of train data. */
    int train_index() const noexcept { return index_data.index(); }
    /* Return the index of seats data. */
    int seats_index() const noexcept { return index_seat.index(); }

    /* Return the relative day of beginning. */
    int beg() const noexcept { return index_data.time(); }

    /* Return the relative day of ending. */
    int end() const noexcept { return index_seat.time(); }

    /* Set train data. */
    void set_train(int x,int y) noexcept { return index_data.set_data(x,y); }

    /* Set seats data. */
    void set_seats(int x,int y) noexcept { return index_seat.set_data(x,y); }

    /* Update seats index. */
    void update_seats(int x)  noexcept { return index_seat.update_index(x); }

    /* As it means...... */
    bool is_released() const noexcept { return index_seat.data < 0; }
};

/* Preview data of a train. */
struct train_view {
    compressor data;  /* Index of train data.   */
    prices_t  price;  /* Prefix cost.           */
    travel_t arrival; /* Arrival time.          */
    travel_t leaving; /* Leaving time.          */
    travel_t  start;  /* Start time in one day. */
    char      __beg;  /* Relative begin date.   */ 
    char      __end;  /* Relative  end  date.   */

    /* Index of the train data. */
    int index() const noexcept { return data.index(); }

    /* Number of the station. */
    int number() const noexcept { return data.time(); }

    /* Set the index of the train data and reset the  */
    void set_index(int __i) noexcept { data.set_index(__i); }

    /* Add the inner number of station. */
    void add_number() noexcept { ++data.data; }

    /* Set the prefix cost. */
    void set_price(prices_t __p) noexcept { price = __p; }

};

static_assert(sizeof(train_view) == 16,"Die");

using ticket = compressor;

}

namespace dark {

template <>
struct Compare <::dark::train_view> {
    int operator()(const ::dark::train_view &lhs,const ::dark::train_view &rhs) 
    const noexcept {
        return lhs.index() < rhs.index() ? -1 : rhs.index() < lhs.index();
    }
};

/* Write the info of a train and its seat. */
void writeline(std::pair <train *,int *> __p) {
    train *__t = __p.first;
    if(!__t) return (void)puts("-1");
    dark::writeline(__t->tid,__t->type());

    number_t n = __t->stat_num;

    int time = 0;
    int cost = 0;

    dark::writeline(__t->names[0],"xx-xx xx:xx","->");

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
