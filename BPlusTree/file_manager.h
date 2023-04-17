#ifndef _DARK_FILE_MANAGER_H_
#define _DARK_FILE_MANAGER_H_

#include "rubbish_bin.h"

#include <Dark/memleak>
#include <Dark/LRU_map>

namespace dark {

/* Simple file_state wrapper. Use highest bit to store modification state. */
struct file_state {
    int  index; /* Index of the real data. */
    bool state; /* Use highest bit to store modification state. */

    bool is_modified() const noexcept { return state; }
    void modify() noexcept { state = true; }

};

/* Simple wrapper of fstream read-in function. */
template <class T>
struct reading_func {
    void operator ()(std::fstream &__f,T &obj) {
        __f.read((char *)(&obj),sizeof(T));
    }
};

/* Simple wrapper of fstream write function. */
template <class T>
struct writing_func {
    void operator ()(std::fstream &__f,const T &obj) {
        __f.write((const char *)(&obj),sizeof(T));
    }
};

}

/* DEBUG USE ONLY! */
std::ostream &operator << (std::ostream &os,dark::file_state x) 
{  return os << x.index; }


namespace std {

/* Custom hash. */
template <>
struct hash <dark::file_state> {
    size_t operator() (dark::file_state t) const noexcept 
    { return t.index; }
};

/* Custom equal to. */
template <>
struct equal_to <dark::file_state> {
    bool operator() (dark::file_state x,dark::file_state y) 
    const noexcept { return x.index == y.index; }
};

}


namespace dark {


/**
 * @brief A file manager requiring two files.
 * 
 * @tparam T The inner data type.
 * @tparam table_size Table size of inner LRU_map.
 * @tparam cache_size Cache size of inner LRU_map.
 * @tparam bias Bias of data in seekg and seekp.
 * @tparam i_func Custom reading function wrapper.
 * @tparam o_func Custom writing function wrapper.
 */
template <
    class T,
    size_t table_size,
    size_t cache_size,
    size_t bias = 0,
    class i_func =  reading_func <T>,
    class o_func = writing_func <T>
> 
class file_manager {
  private:
    using map_t    = LRU_map <file_state,T,table_size>;
    using iterator = typename map_t::iterator;

    rubbish_bin  bin;      /* Rubbish bin. */
    std::fstream dat_file; /* Pure data file. */
    map_t map;             /* Map of cache.   */
    T cache;               /* Cache Block.    */

    /* Insert the map with data in cache block at given iterator. */
    void insert_map(iterator &iter,file_state state) {
        /* Need to write back to cache first. */
        std::pair <file_state,T> *__t;

        /* If modified and full sized , write to disk first. */
        if(map.size() == cache_size && (__t = map.last())->first.is_modified())
            write_object(__t->second,__t->first.index);

        /* Insert the element after iterator and update iterator. */
        iter = map.insert_after(state,cache,iter,map.size() == cache_size);
    }

    /* Locate the position for reading. */
    void locate_in (int index) 
    { dat_file.seekg(index * sizeof(T) + bias); }
    /* Locate the position for writing. */
    void locate_out(int index) 
    { dat_file.seekp(index * sizeof(T) + bias); }

  public:
    [[no_unique_address]]i_func reader; /* Read  func. Modifiable. */
    [[no_unique_address]]o_func writer; /* Write func. Modifiable.*/

    /* Visitor to cache data. */
    struct visitor {
        std::pair <file_state,T> *__p;

        inline bool is_modified() noexcept { return __p->first.is_modified(); }

        /* Use this function whenever the state is modified. */
        inline void modify() noexcept { return __p->first.modify(); }

        /* Customly modify the data by passing function and args. */
        template <class modify_func,class ...Args>
        inline void modify(modify_func &&__f,Args &&...objs) noexcept
        { __f(__p->second,std::forward <Args> (objs)...); __p->first.modify(); }

        inline T &data() { return __p->second; }

        T &operator * () const noexcept { return  __p->second; }
        T *operator ->() const noexcept { return &__p->second; }

        inline void copy(const T &rhs) noexcept
        { memcpy(&__p->second,&rhs,sizeof(T)); modify(); }
        inline void assign(const T &rhs) noexcept
        { __p->second = rhs; modify(); }

        inline int index() const noexcept 
        { return __p->first.index; }
    };


    /* Can't start from nothing. */
    file_manager() = delete;
    /* Initialize by passing 2 file path */
    file_manager(std::string __dat,std::string __bin) noexcept :
        bin(__bin), dat_file(__dat,std::ios::in | std::ios::out | std::ios::binary)  {
        if(!dat_file.good()) {
            dat_file.close(); dat_file.open(__dat,std::ios::out);
            dat_file.close(); dat_file.open(__dat,std::ios::in | std::ios::out | std::ios::binary);
        }
    }

    /* Write out information. */
    ~file_manager() {
        /* Write cache info from data to disk*/
        for(auto &&iter : map) {
            if(iter.first.is_modified())
                write_object(iter.second,iter.first.index);
        }
    }

    /* Return reference to given data. */
    visitor get_object(int index) {
        auto iter = map.find_pre({index});
        auto *__p = iter.next_data();

        if(__p) return {__p};     /* Cache hit case.*/

        read_object(cache,index); /* Read to cache first. */
        insert_map(iter,{index,0}); /* Insert to map from cache. */
        return {iter.next_data()};
    }

    /* Recycle an old node. */
    void recycle(int index) {
        /* Recycle first to rubbish bin. */
        bin.recycle(index);

        auto iter = map.find_pre({index});
        auto *__p = iter.next_data();
        /* If existed and modified , write back to disk first. */
        if(__p && __p->first.is_modified())
            write_object(__p->second,__p->first.index);
        /* Erase element from map. */
        map.erase_after(iter);
    }

    /* Allocate a new node for further modification. */
    visitor allocate() {
        int index = bin.allocate(); /* Allocate a new node. */

        auto iter = map.find_pre({index}); /* Iterator before {index}. */

        /* Of course, newly allocated node will be modified. */
        insert_map(iter,{index,1});
        return {iter.next_data()};
    }


    /* Read object from disk. */
    void read_object(T &obj,int index) {
        locate_in(index);
        reader(dat_file,obj);
    }
    /* Write object to disk. */
    void write_object(const T &obj,int index) {
        locate_out(index);
        writer(dat_file,obj);
    }

    /* Count of all nodes. */
    size_t size() const noexcept { return bin.size(); }
    /* Whether node count is zero. */
    bool empty() const noexcept { return !bin.size(); }
};

}


#endif
