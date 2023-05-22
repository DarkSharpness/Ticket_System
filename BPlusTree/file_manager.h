#ifndef _DARK_FILE_MANAGER_H_
#define _DARK_FILE_MANAGER_H_

#include "rubbish_bin.h"
#include "Dark/LRU_map"

namespace dark {


/**
 * @brief A file manager requiring two files.
 * 
 * @tparam T The inner data type.
 * @tparam table_size Table size of inner LRU_map.
 * @tparam cache_size Cache size of inner LRU_map.
 * @tparam page_size Size of a page for writing.
 */
template <
    class T,
    size_t table_size,
    size_t cache_size,
    size_t page_size = ((sizeof(T) - 1) / 4096 + 1) * 4096
>
class file_manager {
  public:
    struct visitor; /* Declaration. */

  private:
    using map_t    = linked_hash_map <file_state,T,table_size>;
    using iterator = typename map_t::iterator;

    rubbish_bin  bin;      /* Rubbish bin. */
    std::fstream dat_file; /* Pure data file. */
    map_t map;             /* Map of cache.   */
    T cache;               /* Cache Block.    */

    /* Insert the map with data in cache block at given iterator. */
    visitor insert_map(file_state state) {
        /* If full sized , erase the oldest. */
        if(map.size() == cache_size) {
            auto *__t = map.last();
            /* If modified , write to disk first. */
            if(__t->first.is_modified())
                write_object(__t->second,__t->first.index);
            map.erase(__t->first); /* Erase it! */
        }

        /* Insert the element after iterator and update iterator. */
        return {map.insert(state,cache,state.state).next_data()};
    }

  public:
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

    /**
     * @brief Construct a new file manager object.
     * 
     * @param __dat The path for .dat file.
     * @param __bin The path for .bin file.
     */
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
        auto *__p = map.find_pre({index,0}).next_data();
        if(__p) return {__p};         /* Cache hit case.*/
        read_object(cache,index);     /* Read to cache first. */
        return insert_map({index,0}); /* Insert to map from cache. */
    }

    /* Recycle an old node. */
    void recycle(int index) {
        bin.recycle(index);           /* Recycle first to rubbish bin. */
        map.erase({index,0});         /* Erase element from map. */
    }

    /* Allocate a new node for further modification. */
    visitor allocate() {
        /* Of course, newly allocated node will be modified. */
        return insert_map({bin.allocate(),1});
    }

    /* Skip the last block. Users should manage the block themselves. */
    void init() { bin.skip_block(); }

    /* Read object from disk at given index. */
    void read_object(T &obj,int index) {
        dat_file.seekg(index * page_size);
        dat_file.read((char *)&obj,page_size);
    }
    /* Write object to disk at given index. */
    void write_object(const T &obj,int index) {
        dat_file.seekp(index * page_size);
        dat_file.write((const char *)&obj,page_size);
    }

    /* Clear the data within. */
    void clear() {
        bin.reset();
        map.clear();
    }

    /* Count of all nodes. */
    size_t size() const noexcept { return bin.size(); }
    /* Whether node count is zero. */
    bool empty() const noexcept { return !bin.size(); }
};

}


#endif
