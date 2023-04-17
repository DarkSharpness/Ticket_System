#ifndef _DARK_RUBBISH_BIN_H_
#define _DARK_RUBBISH_BIN_H_


#include <Dark/trivial_array>

#include <fstream>

namespace dark {

/**
 * @brief A rubbish bin for file management.
 * 
 */
class rubbish_bin {
    std::fstream bin_file; /* First 16 Byte : total and count. Then data array. */
    size_t total; /* Count of nodes. */
    trivial_array <size_t> bin_array;  /* Cache of unused nodes. */

  public:

    /* Initialize rubbish bin. */
    rubbish_bin(std::string __bin) noexcept : 
        bin_file(__bin,std::ios::in | std::ios::out | std::ios::binary) {
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

    /* Write bin data to disk. */
    ~rubbish_bin() {
        bin_file.seekp(0);
        std::pair <size_t,size_t> buffer(total,bin_array.size());
        bin_file.write((char *)&buffer,sizeof(buffer));
        bin_file.write((char *)bin_array.data(),buffer.second * sizeof(size_t));
        bin_file.close();
    }

    /* Allocate one index. */
    size_t allocate() {
        if(!bin_array.empty()) return bin_array.pop_back();
        else return total++;
    }

    /* Recyle one index. */
    void recycle(size_t index) { bin_array.push_back(index); }

    /* Return count of all nodes. */
    size_t size() const noexcept { return total; }

};





}


#endif
