#include "Dark/memleak"
#include "Dark/inout"
#include "bplus.h"


signed main() {
    using tree = dark::b_plus::tree;

    typename tree::return_list data;
    dark::b_plus::tree t("a");

    for(int i = 0 ; i < 100 ; ++i)
        t.insert("a",rand() % 100);

    for(int i = 0 ; i < 100 ; ++i)
        t.erase("a",rand() % 100);

    t.check_function();
    return 0;
}