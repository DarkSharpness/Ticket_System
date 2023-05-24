#ifndef _TICKET_TRAIN_SYSTEM_H_
#define _TICKET_TRAIN_SYSTEM_H_

#include "header.h"


namespace dark {

class train_system {
  private:
    using set_t = external_hash_set <train_state,12000>;
    using map_t = bpt <size_t,train_view,1000,256,1>;

    using data_file_t = file_manager <train>;
    using seat_file_t = file_manager <seats>;

    set_t   train_set; /* Set containing simple but not full train info. */
    map_t station_map; /* Map from station to the passing-by trains.  */

    data_file_t data_file; /* File manager of train data. */
    seat_file_t seat_file; /* File manager of seats info. */

    train cache_train;     /* Cached. */
    seats cache_seats;     /* Cached. */

    void read_train(int index) { data_file.read_object(cache_train,index); }
    void read_seats(int index) { seat_file.read_object(cache_seats,index); }

    void write_train(int index) { data_file.write_object(cache_train,index); }
    void write_seats(int index) { seat_file.write_object(cache_seats,index); }

  public:

    train_system() = delete;

    ~train_system() = default;

    train_system(std::string __path1,std::string __path2,
                 std::string __path3,std::string __path4) :
        train_set(std::move(__path1)),
        station_map(std::move(__path4)),
        data_file(__path2 + ".dat",__path2 + ".bin"),
        seat_file(__path3 + ".dat",__path3 + ".dat") {}

    bool add_train(const char *__i,const char *__n,
                   const char *__m,const char *__s,
                   const char *__p,const char *__x,
                   const char *__t,const char *__o,
                   const char *__d,const char *__y) {
        /* Such train has existed. */
        size_t hid_i = string_hash(__i);
        if(train_set.exist(hid_i)) return false;

        /* State: No released. */
        train_state state = {hid_i,data_file.allocate(),-1};
        train_set.insert(state);

        /* Write train data. */
        cache_train.copy(__i,__n,__m,__s,__p,__x,__t,__o,__d,__y);
        write_train(state.index_data);   
        return true;
    }

    bool delete_train(const char *__i) {
        auto iter = train_set.find_pre(string_hash(__i));
        auto *__p = iter.next_data();

        /* No such train || Train released. */
        if(!__p || __p->is_released()) return false;

        /* Clean the train's data. */
        data_file.recycle(__p->index_data);
        train_set.erase(iter);
        return true;
    }

    bool release_train(const char *__i) {
        auto __t = train_set.find(string_hash(__i));

        /* No such train || Has been released. */
        if(!__t || __t->is_released()) return false;

        read_train(__t->index_data);

        /* Initialize the data. */

        train_view view;

        view.copy(__t->__hash,0,0);
        view.set_range(cache_train.sale_beg,cache_train.sale_end);
        station_map.insert(string_hash(cache_train.names[0]),view); 

        for(int i = 0 ; i != cache_train.stat_num - 1 ; ++i) {
            view.set_stop_ov(cache_train.stopov_time[i]);
            view.add_cost(cache_train.price[i]);
            station_map.insert(string_hash(cache_train.names[i + 1]),view);
            view.add_time(cache_train.travel_time[i] + cache_train.stopov_time[i]);
        }

        auto /* Begin and end day. */
        __beg = to_relative_day(cache_train.sale_beg),
        __end = to_relative_day(cache_train.sale_end);

        /* Initializing the info. */
        memset(&cache_seats,0,sizeof(cache_seats));
        for(int i = __beg ; i <= __end ; ++i)
            for(int j = 0 ; j != cache_train.stat_num ; ++j)
                cache_seats.count[i][j] = cache_train.seat_num;

        /* Write out the seat info. */
        __t->index_seat = seat_file.allocate();
        write_seats(__t->index_seat);

        return true;
    }

};



}


#endif
