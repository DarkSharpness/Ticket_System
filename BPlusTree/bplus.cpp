#include <iostream>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <string>
#include <fstream>
#include <Dark/inout>
#include "file_manager.cpp"

namespace dark {

namespace b_plus {

const int N = 100;
using key_t = char[32];
using   T   = size_t;

class tree {
  private:
    file_manager leave;
    file_manager index;

  public:
    tree() = delete;
    tree(std::string path1,std::string path2) :
        leave((path1 + ".dat"),(path1 + ".in")), 
        index((path2 + ".dat"),(path2 + ".in")) { 
        if(index.empty()) {
            index.
        }


    }
    ~tree() = default;



};


}

}


signed main() {
    return 0;
}
