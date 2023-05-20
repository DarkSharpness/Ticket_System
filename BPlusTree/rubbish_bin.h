#ifndef _DARK_RUBBISH_BIN_H_
#define _DARK_RUBBISH_BIN_H_


#include "utility.h"
#include "Dark/trivial_array"

namespace dark {

/**
 * @brief A rubbish bin for file management.
 * 
 */
class rubbish_bin {
  private:
    std::fstream bin_file; /* First 16 Byte : total and count. Then data array. */
    size_t total; /* Count of nodes. */
    trivial_array <int> bin_array;  /* Cache of unused nodes. */

  public:

    /* Initialize rubbish bin. */
    rubbish_bin(std::string __bin) noexcept :
        bin_file(__bin,std::ios::in | std::ios::out | std::ios::binary) {
        if(!bin_file.good()) {
            bin_file.close(); bin_file.open(__bin,std::ios::out | std::ios::binary);
            std::pair <size_t,size_t> buffer(0,0);
            bin_file.write((char *)&buffer,sizeof(buffer));
            total = 0;
        } else {
            /* Read buffer. */
            std::pair <size_t,size_t> buffer;
            bin_file.read((char *)&buffer,sizeof(buffer));

            /* Update info. */
            total = buffer.first;

            bin_array.resize(buffer.second);
            bin_file.read((char *)bin_array.data(),buffer.second * sizeof(int));
        }
    }

    /* Write bin data to disk. */
    ~rubbish_bin() {
        std::pair <size_t,size_t> buffer(total,bin_array.size());
        bin_file.seekp(0);
        bin_file.write((char *)&buffer,sizeof(buffer));
        bin_file.write((char *)bin_array.data(),buffer.second * sizeof(int));
        bin_file.close();
    }

    /* Allocate one index. */
    int allocate() {
        if(!bin_array.empty()) return bin_array.pop_back();
        else return total++;
    }

    /* Recyle one index. */
    void recycle(int index) { bin_array.push_back(index); }

    /* Return count of all nodes. */
    size_t size() const noexcept { return total; }

    /* Skip the last block. Users should manager the block themselves. */
    void init() { total = 1; }

    /* Reset the  */
    void reset() {
        bin_array.clear();
        bin_array.resize(total - 1);
        for(size_t i = 0 ; i != total - 1 ; ++i)
            bin_array[i] = i + 1;
    }

};





}


#endif
