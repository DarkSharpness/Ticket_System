#include "Dark/memleak"
#include "Dark/inout"
#include "bplus.h"


signed main() {
    using tree = dark::b_plus::tree;

    typename tree::return_list data;
    dark::b_plus::tree t("a");

    // for(int i = 0 ; i < 65 ; ++i)
    //     t.insert("a",i);
    // t.insert("a",66);
    t.insert("a",3);
    t.erase("a",3);

    t.check_function();

    return 0;
}