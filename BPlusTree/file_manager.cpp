#ifndef _DARK_FILE_MANAGER_H_
#define _DARK_FILE_MANAGER_H_

#include <fstream>
#include <Dark/trivial_array>
#include <Dark/LRU_map>

namespace dark {


using T = long long;

class file_manager {
  private:
    std::fstream dat_file; /* Pure data file. */
    std::fstream bin_file; /* First 16 Byte : total and count. Then data array. */
    
    dark::trivial_array <long> bin_array; /* Cache of unused nodes. */

    long total = -1; /* Total of nodes. */
    bool bin_read = false;

    /* Update bin whenever read from bin. */
    void update_bin() {
        bin_read = true;
        std::pair <long,long> temp;
        bin_file.read((char *)(&temp),sizeof(long) << 1);
        total = temp.first;
        bin_array.resize(temp.second);
        bin_file.read((char *)bin_array.data(),sizeof(long) * temp.second);
    }

    LRU_map <long,T> map; /* Map of cache. */

  public:

    file_manager() = delete;
    /* Initialize by passing 2 file path */
    file_manager(std::string __dat,std::string __bin) noexcept :
        dat_file(__dat,std::ios::in | std::ios::out | std::ios::binary),
        bin_file(__bin,std::ios::in | std::ios::out | std::ios::binary) {
        if(!dat_file.good()) {
            dat_file.close(); dat_file.open(__dat,std::ios::out);
            dat_file.close(); dat_file.open(__dat,std::ios::in | std::ios::out | std::ios::binary);
        }
        if(!bin_file.good()) {
            bin_file.close(); bin_file.open(__bin,std::ios::out);
            bin_file.close(); bin_file.open(__dat,std::ios::in | std::ios::out | std::ios::binary);
            total = 0;
            const char empty[sizeof(long) << 1] = {};
            bin_file.write(empty,sizeof(long) << 1);
        }
    }

    ~file_manager() {
        if(bin_read) {
            bin_file.seekp(0);
            auto &&temp = std::make_pair(total,bin_array.size()); 
            bin_file.write((char *)&temp,sizeof(temp));
            bin_file.write((char *)bin_array.data(),bin_array.size() * sizeof(long));
        } else if(!bin_array.empty()) {
            bin_file.seekp(std::ios::end);
            bin_file.write((char *)bin_array.data(),bin_array.size() * sizeof(long));
        }   
        bin_file.close(); /* Bin part ended. */
    }

    /* Read to an object with given size. */
    void read_object(long index,T &r,size_t __n = sizeof(T)) {
        
    }

    /* Write to an object with given size. */
    void write_object(long index,const T &r,size_t __n = sizeof(T)) {
        
    }

    /* Recycle an old node. */
    void recycle(long index) { bin_array.push_back(index); }

    /* Allocate a new node. */
    long allocate() {
        if(bin_array.empty()) {
            if(bin_read) return total++;
            update_bin();
            if(bin_array.empty()) return total++;
        }
        return bin_array.pop_back();
    }

    /* Locate the position for reading. */
    void locate_in (long index,long bias = 0) 
    { dat_file.seekg(index * sizeof(T) + bias); }

    /* Locate the position for writing. */
    void locate_out(long index,long bias = 0) 
    { dat_file.seekp(index * sizeof(T) + bias); }

    /* Count of all nodes. */
    long size()  const noexcept { return  total; }
    /* Whether node count is zero. */
    bool empty() const noexcept { return !total; }

};

}


#endif

signed main() {
    

    return 0;
}