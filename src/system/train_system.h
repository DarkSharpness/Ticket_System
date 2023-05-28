#ifndef _TICKET_TRAIN_SYSTEM_H_
#define _TICKET_TRAIN_SYSTEM_H_

#include "header.h"
#include <tuple>


namespace dark {

class train_system {
  private:
    
    
    using set1_t  = external_hash_set <train_state,12000>;
    using set2_t  = dark::linked_hash_set <station_views,4000>;
    using map1_t  = bpt <size_t,train_view,500,128,1>;
    using map2_t  = bpt <train_unit,int,500,128,1>;
    using array_t = external_array <realtrainID_t>;

    using data_file_t  = file_manager <train>;
    using seat_file_t  = file_manager <seats>;
    using order_file_t = file_manager <order_t,sizeof(order_t)>;


    set1_t   train_set; /* Set containing train_state. */
    map1_t station_map; /* Map from station to the passing-by trains.  */
    map2_t   order_map; /* Map of pending queue,from train to order_index. */
    array_t seatID_map; /* Map from seat_index to trainID and train_index. */

    data_file_t   data_file; /* File manager of train data. */
    seat_file_t   seat_file; /* File manager of seats info. */
    order_file_t order_file; /* File manager of seats info. */

    train cache_train;       /* Cached. */
    seats cache_seats;       /* Cached. */

    order_t     order; /* The order object within. */
    transfer_view ans; /* Answer for query_transfer. */

    int allocate_train() { return data_file.allocate(); }
    int allocate_seats() { return seat_file.allocate(); }

    /* Get the train index by seat index. */
    short &get_train_index(int seat_index) {
        return (short&)(seatID_map[seat_index].base()
                        [sizeof(realtrainID_t) - sizeof(short)]);
    }

    /* Read from given index. */
    inline void read_train(int index)
    { data_file.read_object(cache_train,index); }
    /* Write  to given index. */
    inline void write_train(int index)
    { data_file.write_object(cache_train,index); }
    /* Read from given index. */
    inline void read_seats(int index)
    { seat_file.read_object(cache_seats,index); }
    /* Write  to given index. */
    inline void write_seats(int index)
    { seat_file.write_object(cache_seats,index); }

    /* Read the seat info in range [beg,end] to front. */
    inline void read_seats(int index,int beg,int end) {
        seat_file.read_object(
            cache_seats,
            index,
            sizeof(seat_info_t) * beg,
            0,
            sizeof(seat_info_t) * (end - beg + 1)
        );
    }

    /* Write the seat info from front to range [beg,end]. */
    inline void write_seats(int index,int beg,int end) {
        seat_file.write_object(
            cache_seats,
            index,
            sizeof(seat_info_t) * beg,
            0,
            sizeof(seat_info_t) * (end - beg + 1)
        );
    }

    inline void recyle_train(train_state *__t)
    { data_file.recycle(__t->train_index); }
    inline void recyle_seats(train_state *__t)
    { seat_file.recycle(__t->seats_index); }

    /* Check the date before query. */
    bool relative_date_check(int day)
    { return day >= 0 && day <= DURATION + 3; }

    /* Add a delta to the range. */
    void update_seats(int i,int j,int delta) {
        while(i != j) cache_seats.count[0][i++] += delta;
    }

    /* Read order info from disk. */
    void read_order(int index) { order_file.read_object(order,index); }
    /* Write order info into disk. */
    void write_order(int index) { order_file.write_object(order,index); }
  public:

    /* Create an order and return its index.  */
    int create_order() {
        int index = order_file.allocate();
        order_file.write_object(order,index);
        if(order.is_pending())
            order_map.insert({order.index,(short)order.__dep},index);
        return index;
    }

    /* Return the order stored at index. */
    void print_order(int index) {
        read_order(index);
        dark::writeline(
            (order_wrapper)order.state,
            seatID_map[order.index],
            order.fr,
            (time_wrapper){order.leaving_time()},
            "->",
            order.to,
            (time_wrapper){order.arrival_time()},
            order.price,
            order.count
        );
    }

    /* Query the minimum seat between station i and j. */
    int query_seat(int i,int j) {
        int ans = INT_MAX;
        while(i != j)
            ans = std::min(ans,cache_seats.count[0][i++]);
        return ans;
    }

    /* Query the minimum seat between station i and j on day x. */
    int query_seat(int index,int dep,int i,int j) {
        read_seats(index,dep,dep);
        int ans = INT_MAX;
        while(i != j)
            ans = std::min(ans,cache_seats.count[0][i++]);
        return ans;
    }


    /* Return reference to realtrainID. */
    realtrainID_t *trainID() { return seatID_map.data(); }

    train_system()  = delete;
    ~train_system() = default;
    train_system(std::string __path) :
          train_set(__path + "tset"),
        station_map(__path + "smap"),
          order_map(__path + "omap"),
          data_file(__path + "t.dat",__path + "t.bin"),
          seat_file(__path + "s.dat",__path + "s.bin"),
         order_file(__path + "o.dat",__path + "o.bin") {
        seatID_map.init(__path + "sid",seat_file.size());
    }

    /* Complete. */
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
        train_state state = {
            hid_i,
            (short)index,
            (short)-1,
            (unsigned)calendar_to_relative_day(cache_train.sale_beg),
            (unsigned)calendar_to_relative_day(cache_train.sale_end),
            (unsigned)cache_train.seat_num
        };
        train_set.insert(state);
        return true;
    }

    /* Complete. */
    bool delete_train(const char *__i) {
        auto iter = train_set.find_pre(string_hash(__i));
        auto *__p = iter.next_data();

        /* No such train || Train released. */
        if(!__p || __p->is_released()) return false;

        /* Clean the train's data. */
        recyle_train(__p);
        train_set.erase(iter);
        return true;
    }

    /* Complete. */
    bool release_train(const char *__i) {
        /* No such train || Has been released. */
        auto *__t = train_set.find(string_hash(__i));
        if(!__t || __t->is_released()) return false;

        /* Read the train first and update seatID_map. */
        read_train(__t->train_index);

        (short &) /* User these last 2 Byte. */
        (cache_train.tid[sizeof(realtrainID_t) - sizeof(short)]) = __t->train_index;
        seatID_map.copy_back(cache_train.tid);
 
        auto /* Begin and end day. */
        __beg = calendar_to_relative_day(cache_train.sale_beg),
        __end = calendar_to_relative_day(cache_train.sale_end);

        /* Initializing the info. */
        memset(&cache_seats,0,sizeof(cache_seats));
        for(int i = __beg ; i <= __end ; ++i)
            for(int j = 0 ; j != cache_train.stat_num ; ++j)
                cache_seats.count[i][j] = cache_train.seat_num;

        /* Write out the seat info. */
        __t->seats_index = allocate_seats();
        write_seats(__t->seats_index);

        /* Initialize the data. */
        train_view view = {
            { 
                __t->seats_index,
                0,
                (char)(cache_train.stat_num - 1),
                0
            },     /* Prefix price. */
            0,     /* Arrival time. */
            0,     /* Leaving time. */
            (short)cache_train.time(),
            (char) __beg,
            (char) __end,
        }; /* Insert the first station. */
        station_map.insert(string_hash(cache_train.names[0]),view);

        for(int i = 1 ; i != cache_train.stat_num ; ++i) {
            view.start   = i;
            view.price   = cache_train.price[i];
            view.arrival = cache_train.arrival_time[i];
            view.leaving = cache_train.leaving_time[i];
            station_map.insert(string_hash(cache_train.names[i]),view);
        }
        return true;
    }

    /* Complete. */
    std::tuple <train*,int *,calendar> query_train(const char *__i,const char *__d) {
        const auto *__p = train_set.find(string_hash(__i));
        /* Relative day of begin station. */
        const int day = date_to_relative_day(__d);

        /* Do not exist || Not in given range. */
        if(!__p || day < __p->__beg || __p->__end < day) return {nullptr,nullptr,0};

        /* Read all necessary information. */
        read_train(__p->train_index);
        if(__p->is_released()) read_seats(__p->seats_index,day,day);
        else for(int i = 0 ; i != cache_train.stat_num ; ++i)
                cache_seats.count[0][i] = cache_train.seat_num;

        return {
            &cache_train,
            cache_seats.count[0],
            merge_relative_day_time(day,cache_train.time())
        };
    }

    /* Complete. */
    auto query_ticket(const char *__s,const char *__t,const char *__d) {
        dark::trivial_array <train_view> views;

        /* Relative day of current station. */
        const int day = date_to_relative_day(__d);
        if(!relative_date_check(day)) return views;

        station_map.find_if( /* Find trains : not terminal && in the set. */
            string_hash(__s),
            views,
            [day](const train_view &view) {
                return !(view.out_of_range(day) || view.is_terminal());
            }
        );
        if(views.empty()) return views;

        size_t hid_t = string_hash(__t);
        auto __pf = views.begin(); /* Front pointer to save space. */
        auto __ps = views.begin();
        auto __pt = station_map.lower_bound(hid_t);
        /* Double pointering. */
        while(__ps != views.end() && __pt.valid() && __pt.key() == hid_t) {
            int delta = __ps->index - __pt->index;
            if     (delta < 0) ++__ps;
            else if(delta > 0) ++__pt;
            else {/*delta = 0*/
                train_view view_s = *__ps;
                train_view view_t = *__pt;
                ++__ps,++__pt;
                if(view_t.start <= view_s.start) continue;

                /* Relative day of begin station. */
                int dep = day - view_s.travel_day();

                int remainder = query_seat(view_s.index,dep,view_s.start,view_t.start);

                /* The order view to keep. */
                order_view view = {
                    view_s.index,
                    (short)(view_t.arrival - view_s.leaving),
                    view_t.price - view_s.price,
                    remainder,
                    merge_relative_day_time(dep,view_s.leaving + view_s._time),
                };

                /* Force to pass data. */
                static_assert(sizeof(order_view) == sizeof(train_view));
                memcpy(&(*__pf++),&view,sizeof(view));
            }
        }

        views.resize(__pf - views.begin());
        return views;
    }

    /* Complete. */
    transfer_view *query_transfer(const char *__s,const char *__t,
                        const char *__d,const bool type) {
        static set2_t set; /* Data storer. Initial as empty. */

        /* Relative day of current station. */
        const int day = date_to_relative_day(__d);
        if(!relative_date_check(day)) return nullptr;

        /* Read necessary data into memory. */
        size_t hid_s = string_hash(__s);
        for(auto iter = station_map.lower_bound(hid_s);
            iter.valid() && iter.key() == hid_s ; ++iter) {
            train_view view = *iter;
            /**
             * The day must be in range of the train.
             * Also, the start station can't be the last station.
             */
            if(view.out_of_range(day) || view.is_terminal()) continue;

            int index = get_train_index(view.index);
            read_train(index);

            /* Set the relative day at begin station. */
            view.__beg = view.__end = day - view.travel_day();
            prices_t current_price  = view.price;
            for(int i = view.start + 1 ; i < cache_train.stat_num ; ++i) {
                size_t __h   = string_hash(cache_train.names[i]);
                if(!set.exist(__h)) set.insert(__h);
                auto *vec = set.find(__h); /* Data vector. */

                /* Some data need to be modified. */
                view.final   = i;
                view.price   = cache_train.price[i] - current_price;
                view.arrival = cache_train.arrival_time[i];

                vec->copy_back(view);
            }
        }

        int tag = 0; /* Special tag. */
        /* Find the answer. */
        size_t hid_t = string_hash(__t);
        for(auto iter = station_map.lower_bound(hid_t);
            iter.valid() && iter.key() == hid_t ; ++iter) {
            train_view data = *iter;
            /**
             * The terminal station can't be the first station.
             * Also, end of ticket of current station
             * can't be later than departure time(__d).
             */
            if(data.is_starting() || data.__end + data.travel_day() < day) continue;

            int index = get_train_index(data.index);
            read_train(index);
            data.final = data.start;
            prices_t current_price = data.price;
            for(int i = 0 ; i <= data.final - 1 ; ++i) {
                const auto *vec = set.find(string_hash(cache_train.names[i]));
                if(!vec || vec->empty()) continue;

                data.start   = i;
                data.leaving = cache_train.leaving_time[i];
                data.price   = current_price - cache_train.price[i];

                const calendar last_leaving = /* Last  absolute leaving time */
                    cache_train.sale_end + data.leaving;
                const calendar init_leaving = /* First absolute leaving time.*/
                    cache_train.sale_beg + data.leaving;

                bool updated = false; /* Whether middle station changed. */
                for(const auto view : *vec) {
                    if(data.index == view.index) continue;
                    calendar arrival = /* Arrival time of first train. */
                        merge_relative_day_time(view.__beg,view.arrival_time());
                    if(arrival > last_leaving) continue;
                    /* Record the transfer information. */

                    const int interval = /* Time from begin to the end. */
                        view.arrival - view.leaving +
                        data.arrival - data.leaving +
                        (arrival < init_leaving ?
                            init_leaving - arrival :
                            calendar_to_time(last_leaving - arrival));
                    const int cost = view.price + data.price; /* Total cost. */

                    if(tag) {
                        int delta[2] = {cost - ans.cost(),interval - ans.sum()};
                        int tag = delta[type];
                        if(!tag) tag = delta[!type];
                        if(tag > 0) continue;
                        if(!tag) {
                            int cmp = strcmp(seatID_map[ans.__[0].index].base(),
                                             seatID_map[ view.index ].base());
                            if(cmp < 0) continue;
                            else if(!cmp && seatID_map[ans.__[1].index] <
                                            cache_train.tid) continue;
                        }
                    } tag = updated = true; /* Set the tag now. */

                    ans.interval[0] = view.arrival - view.leaving;
                    ans.interval[1] = data.arrival - data.leaving;
                    ans.leaving_beg = arrival - ans.interval[0];
                    ans.arrival_end = ans.leaving_beg + interval;
                    ans.__dep[0]    = view.__beg;
                    ans.__dep[1]    = calendar_to_relative_day(
                                        ans.leaving_mid() -
                                        data.leaving
                                    );
                    ans.copy(view,data);
                }
                if(updated) ans.name = cache_train.names[i];
            }
        }

        /* Clear storage in case of memory limit exceed. */
        set.clear();
        set.shrink();
        return tag ? &ans : nullptr;
    }

    /* Complete */
    order_t *buy_ticket(const char *__i,const char *__d,const char *__n,
                        const char *__f,const char *__t,const bool type) {

        const auto *__p = train_set.find(string_hash(__i));
        const int count = to_unsigned_integer <number_t> (__n);

        /* Do not exist || Not released || More than count. */
        if(!__p || !__p->is_released() || count > __p->count) return nullptr;

        const int day = date_to_relative_day(__d);
        if(!relative_date_check(day)) return nullptr;

        read_train(__p->train_index);

        const auto [i,j] = cache_train.find(__f,__t);
        if(j >= cache_train.stat_num) return nullptr;

        /* Absolute day of first leaving of current station. */
        calendar init_leaving = cache_train.sale_beg + cache_train.leaving_time[i];
        int dep = day - calendar_to_relative_day(init_leaving) + __p->__beg;
        if(dep < __p->__beg || __p->__end < dep) return nullptr;

        /* Remaining seats between station i and j. */
        int remainder = query_seat(__p->seats_index,dep,i,j);
        if(remainder < count && !type) return nullptr;
        if(remainder >= count) {
            update_seats(i,j,-count);
            write_seats (__p->seats_index,dep,dep);
        }
        order.fr       = __f;
        order.to       = __t;
        order.start()  = i;
        order.final()  = j;
        order.state    = (remainder >= count);
        order.interval = cache_train.arrival_time[j] - cache_train.leaving_time[i];
        order.index    = __p->seats_index;
        order.__dep    = dep;
        order.count    = count;
        order.leaving  = merge_day_time(dep - __p->__beg,init_leaving);
        order.price    = cache_train.price[j] - cache_train.price[i];

        return &order;
    }

    /* Complete. */
    void refund_ticket(int index) {
        read_order(index);
        read_seats(order.index,order.__dep,order.__dep);
        if(order.is_success())
            update_seats(order.start(),order.final(),order.count);
        else /* Is_pending. */
            order_map.erase({order.index,(short)order.__dep},index);
        order.set_refunded();
        write_order(index);

        typename map2_t::return_list vec;
        order_map.find ({order.index,(short)order.__dep},vec);

        /* TODO: Square optimization */
        // int data[2][10];
        // memset(data[0],0,sizeof(data[0]));
        // memset(data[0],1,sizeof(data[1]));
        // for(int i = 0 ; i < 100 ; ++i)
        //     data[1][i / 10] = min(data[1][i / 10],seats.count[0][i])

        for(int iter : vec) {
            read_order(iter);
            int remainder = query_seat(order.start(),order.final());
            if(remainder >= order.count) {
                order.set_success();
                write_order(iter);
                order_map.erase({order.index,(short)order.__dep},iter);
                update_seats(order.start(),order.final(),-order.count);
            }
        }
        write_seats(order.index,order.__dep,order.__dep);
    }

};



}


#endif
