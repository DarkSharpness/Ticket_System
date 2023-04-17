#include <iostream>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <string>
#include <fstream>
#include <Dark/inout>
#include "file_manager.h"

namespace dark {

namespace b_plus {

using key_t = char [68];
using   T   = int;

constexpr size_t kBLOCK_SIZE = 100;

class tree {
  private:
    struct pair {
        key_t key;
        T value;
    };

    struct leave_node {
        int prev;
        int next;
        pair data[kBLOCK_SIZE];
    };


  public:
    tree() = delete;



};


}

}


signed main() {
    return 0;
}
