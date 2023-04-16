#ifndef _DARK_FILE_MANAGER_H_
#define _DARK_FILE_MANAGER_H_

#include <fstream>
#include <Dark/trivial_array>
#include <Dark/LRU_map>
#include <Dark/memleak>

namespace dark {

/* Simple file_state wrapper. Use one bit to store modified state. */
struct file_state {
    size_t state;
    bool is_modified() const noexcept { return state >> 63; }
    size_t index() const noexcept { return state & LONG_LONG_MAX; }
    void modify() noexcept { state |= (1ULL << 63); }

};

}


namespace std {

template <>
struct hash <dark::file_state> {
    size_t operator() (dark::file_state t) const noexcept 
    { return t.index(); }
};

template <>
struct equal_to <dark::file_state> {
    size_t operator() (dark::file_state x,dark::file_state y) 
    const noexcept { return x.index() == y.index(); }
};

}


namespace dark {


template <
    class T,
    size_t table_size,
    size_t cache_size
> class file_manager {
  private:
    using map_t    = LRU_map <file_state,T,table_size>;
    using iterator = typename map_t::iterator;

    std::fstream dat_file; /* Pure data file. */
    std::fstream bin_file; /* First 16 Byte : total and count. Then data array. */
    trivial_array <size_t> bin_array;  /* Cache of unused nodes. */
    size_t total; /* Count of nodes. */
    map_t map;    /* Map of cache.   */
    T cache;      /* Cache Block.    */

    /* Insert the map with data in cache block at given iterator. */
    void insert_map(iterator iter,size_t index) {
        /* Need to write back to cache first. */
        std::pair <file_state,T> *__t;

        /* If modified and full sized , write to disk first. */
        if(map.size() == cache_size && (__t = map.last())->first.is_modified())
            write_object(__t->second,__t->first.index());

        /* Insert the element after iterator. */
        map.insert_after({index},cache,iter,map.size() == cache_size);
    }

    /* Return the index of the block newly allocated. */
    size_t allocate_index() {
        if(!bin_array.empty()) return bin_array.pop_back();
        write_object(cache,total); /* Write to enlarge the space. */
        return total++;
    }


  public:

    /* Visitor to cache data. */
    struct visitor {
        std::pair <file_state,T> *__p;

        inline bool is_modified() noexcept { return __p->first.is_modified(); }

        /* Use this function whenever the state is modified. */
        inline void modify() noexcept { return __p->first.modify(); }

        inline T &data() { return __p->second; }

        T &operator * () const noexcept { return  __p->second; }
        T *operator ->() const noexcept { return &__p->second; }

        inline void copy(const T &rhs) noexcept
        { memcpy(&__p->second,&rhs,sizeof(T)); modify(); }
        inline void assign(const T &rhs) noexcept
        { __p->second = rhs; modify(); }

        inline size_t index() const noexcept 
        { return __p->first.index(); }
    };


    /* Can't start from nothing. */
    file_manager() = delete;
    /* Initialize by passing 2 file path */
    file_manager(std::string __dat,std::string __bin) noexcept :
        dat_file(__dat,std::ios::in | std::ios::out | std::ios::binary),
        bin_file(__bin,std::ios::in | std::ios::out | std::ios::binary) {
        if(!dat_file.good()) {
            dat_file.close(); dat_file.open(__dat,std::ios::out);
            dat_file.close(); dat_file.open(__dat,std::ios::in | std::ios::out | std::ios::binary);
        }

        /* Read bin data. */
        if(!bin_file.good()) {
            bin_file.close(); bin_file.open(__bin,std::ios::out);
            const char buffer[sizeof(size_t) << 1] = {};
            bin_file.write(buffer,sizeof(buffer));
            total = 0;
        } else {
            /* Read buffer. */
            std::pair <size_t,size_t> buffer;
            bin_file.read((char *)&buffer,sizeof(buffer));

            /* Update info. */
            total = buffer.first;
            bin_array.resize(buffer.second);
            bin_file.read((char *)bin_array.data(),buffer.second * sizeof(size_t));
        }
    }
    /* Write out information. */
    ~file_manager() {
        /* Write bin data to disk first. */
        bin_file.seekp(0);
        std::pair <size_t,size_t> buffer(total,bin_array.size());
        bin_file.write((char *)&buffer,sizeof(buffer));
        bin_file.write((char *)bin_array.data(),buffer.second * sizeof(size_t));
        bin_file.close();

        /* Write cache info from data to disk*/
        for(auto &&iter : map) 
            if(iter.first.is_modified())
                write_object(iter.second,iter.first.index());
    }


    /* Return reference to given data. */
    visitor get_object(size_t index) {
        auto iter = map.find_pre({index});
        auto *__p = iter.next_data();
        if(__p) return {__p};
        read_object(cache,index); /* Read to cache first. */
        insert_map(iter,index);
        return {iter.next_data()};
    }


    /* Recycle an old node. */
    void recycle(size_t index) {
        bin_array.push_back(index);
        auto iter = map.find_pre({index});
        auto *__p = iter.next_data();
        /* If existed and modified , write back to disk first. */
        if(__p && __p->first.is_modified())
            write_object(__p->second,__p->first.index());
        /* Erase element from map */
        map.erase_after(iter);
    }

    /* Allocate a new node for further modification. */
    visitor allocate() {
        size_t index = allocate_index();
        auto iter = map.find_pre({index});
        /* Of course, newly allocated node will be modified. */
        insert_map(iter,index | 1ULL << 63);
        return {iter.next_data()};
    }


    /* Read object from disk. */
    void read_object(T &obj,size_t index) {
        locate_in(index);
        dat_file.read((char *)&obj,sizeof(T));
    }
    /* Write object to disk. */
    void write_object(const T &obj,size_t index) {
        locate_out(index);
        dat_file.write((char *)&obj,sizeof(T));
    }


    /* Locate the position for reading. */
    void locate_in (size_t index,size_t bias = 0) 
    { dat_file.seekg(index * sizeof(T) + bias); }
    /* Locate the position for writing. */
    void locate_out(size_t index,size_t bias = 0) 
    { dat_file.seekp(index * sizeof(T) + bias); }


    /* Count of all nodes. */
    size_t size() const noexcept { return total; }
    /* Whether node count is zero. */
    bool empty() const noexcept { return !total; }

};

}


#endif
