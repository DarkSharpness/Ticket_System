#ifndef _TICKET_TRAIN_SYSTEM_H_
#define _TICKET_TRAIN_SYSTEM_H_

#include "header.h"

namespace dark {

class train_system {
  private:
    using set_t  = external_hash_set <train_state,12000>;
    using data_file_t = file_manager <train>;
    using seat_file_t = file_manager <seats>;

    set_t train_set;

    data_file_t data_file;
    seat_file_t seat_file;

    train cache_train;

  public:

    train_system() = delete;

    ~train_system() = default;

    train_system(std::string __path1,std::string __path2,std::string __path3) :
        train_set(std::move(__path1)),
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
        cache_train.copy(__i,__n,__m,__s,__p,__x,__t,__o,__d,__y);

        data_file.write_object(cache_train,state.index_data);
        return true;
    }

    bool delete_train(const char *__i) {
        int index;
        bool result = train_set.erase_if(string_hash(__i),
                                        [&index](train_state &__s)->bool {
                                            index = __s.index_data;
                                            return !__s.is_released();
                                        });
        if(result) data_file.recycle(index);
        return result;
    }

    bool release_train(const char *__i) {
        auto __t = train_set.find(string_hash(__i));
        
        /* No such train || Has been released. */
        if(!__t || __t->is_released()) return false;

        __t->index_seat = seat_file.allocate();
        data_file.read_object(cache_train,__t->index_data);

        int beg;

        return true;
    }

};



}



#endif
