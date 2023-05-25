#ifndef _TICKET_TRAIN_SYSTEM_H_
#define _TICKET_TRAIN_SYSTEM_H_

#include "header.h"


namespace dark {

class train_system {
  private:
    using set_t = external_hash_set <train_state,12000>;
    using map1_t = bpt <size_t,train_view,500,128,1>;
    using map2_t = bpt <ticket,int,500,128,1>;

    using data_file_t  = file_manager <train>;
    using seat_file_t  = file_manager <seats>;
    using order_file_t = file_manager <order,sizeof(order)>;


    set_t    train_set; /* Set containing train_state. */
    map1_t station_map; /* Map from station to the passing-by trains.  */
    map2_t   order_map; /* Map from ticket type to index of order. */

    data_file_t   data_file; /* File manager of train data. */
    seat_file_t   seat_file; /* File manager of seats info. */
    order_file_t order_file; /* File manager of seats info. */

    train cache_train;     /* Cached. */
    seats cache_seats;     /* Cached. */

    int allocate_train() { return data_file.allocate(); }
    int allocate_seats() { return seat_file.allocate(); }

    void read_train(int index) { data_file.read_object(cache_train,index); }
    void read_seats(int index) { seat_file.read_object(cache_seats,index); }

    void write_train(int index) { data_file.write_object(cache_train,index); }
    void write_seats(int index) { seat_file.write_object(cache_seats,index); }

    void recyle_train(int index) { data_file.recycle(index); }
    void recyle_seats(int index) { seat_file.recycle(index); }

  public:

    train_system() = delete;

    ~train_system() = default;

    train_system(std::string __path) :
          train_set(__path + "tset"),
        station_map(__path + "smap"),
          order_map(__path + "omap"),
          data_file(__path + "t.dat",__path + "t.bin"),
          seat_file(__path + "s.dat",__path + "s.bin"),
         order_file(__path + "o.dat",__path + "o.bin") {}

    bool add_train(const char *__i,const char *__n,
                   const char *__m,const char *__s,
                   const char *__p,const char *__x,
                   const char *__t,const char *__o,
                   const char *__d,const char *__y) {
        /* Such train has existed. */
        size_t hid_i = string_hash(__i);
        if(train_set.exist(hid_i)) return false;

        /* State: No released. */
        int index = allocate_train();
        /* Write train data. */
        cache_train.copy(__i,__n,__m,__s,__p,__x,__t,__o,__d,__y);
        write_train(index); 

        /* Record the state to train_set. */
        train_state state;
        state.__hash = hid_i;
        state.set_train(index,to_relative_day(cache_train.sale_beg));
        state.set_seats(  -1 ,to_relative_day(cache_train.sale_end));
        train_set.insert(state);
        return true;
    }

    bool delete_train(const char *__i) {
        auto iter = train_set.find_pre(string_hash(__i));
        auto *__p = iter.next_data();

        /* No such train || Train released. */
        if(!__p || __p->is_released()) return false;

        /* Clean the train's data. */
        recyle_train(__p->train_index());
        train_set.erase(iter);
        return true;
    }

    bool release_train(const char *__i) {
        /* No such train || Has been released. */
        auto __t = train_set.find(string_hash(__i));
        if(!__t || __t->is_released()) return false;

        /* Read the train first. */
        read_train(__t->train_index());

        auto /* Begin and end day. */
        __beg = to_relative_day(cache_train.sale_beg),
        __end = to_relative_day(cache_train.sale_end);

        /* Initializing the info. */
        memset(&cache_seats,0,sizeof(cache_seats));
        for(int i = __beg ; i <= __end ; ++i)
            for(int j = 0 ; j != cache_train.stat_num ; ++j)
                cache_seats.count[i][j] = cache_train.seat_num;

        /* Write out the seat info. */
        __t->update_seats(allocate_seats());
        write_seats(__t->seats_index());

        /* Initialize the data. */
        train_view view {
            {__t->train_index(),0}, /* Index,Number */
            0, /* Prefix price */
            0, /* Arrival time. */
            0, /* Leaving time.*/
            to_time(cache_train.sale_beg),
            __beg,
            __end,
        };
        station_map.insert(string_hash(cache_train.names[0]),view);

        for(int i = 1 ; i != cache_train.stat_num ; ++i) {
            view.add_number();
            view.price   = cache_train.price[i];
            view.arrival = cache_train.arrival_time[i];
            view.leaving = cache_train.leaving_time[i];
            station_map.insert(string_hash(cache_train.names[i]),view);
        }
        return true;
    }

    std::pair <train*,int *> query_train(const char *__i,const char *__d) {
        auto __p = train_set.find(string_hash(__i));
        int day  = date_to_relative_day(__d);

        /* Do not exist || Not in given range. */
        if(!__p || day < __p->beg() || __p->end() < day) return {nullptr,nullptr};

        /* Read all necessary information. */
        read_train(__p->train_index());
        if(__p->is_released()) read_seats(__p->seats_index());

        return {&cache_train,__p->is_released() ? cache_seats.count[day] : nullptr}; 
    }

};



}


#endif
