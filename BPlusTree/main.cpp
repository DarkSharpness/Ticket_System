#include "Dark/memleak"
#include "bplus.h"

signed main() {
    dark::b_plus::tree t("a");
    t.insert("abcd",1);
    t.insert("abcd",3);
    t.insert("abcd",4);
    t.insert("abcd",9);
    t.insert("abcd",10);
    t.insert("abcd",0);
    t.insert("abcd",-3);
    t.insert("0",3);
    t.insert("1",1);
    t.insert("2",3);
    t.insert("3",6);
    t.insert("1",0);
    t.insert("b",2);
    t.insert("b",1);
    t.insert("b",3);
    t.insert("b",5);
    t.insert("c",3);
    t.insert("a",0);
    t.insert("0",0);
    t.insert("b",4);
    t.insert("0",1);
    t.insert("a",2);
    t.insert("2",0);
    t.insert("a",3);
    t.insert("a",4);
    t.insert("c",6);
    t.insert("c",5);   
    
    dark::trivial_array <int> v;
    t.find("abcd",v);
    for(auto && iter : v) std::cout << iter << ' ';

    t.check_function();
    return 0;
}