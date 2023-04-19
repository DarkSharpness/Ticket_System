#include "Dark/memleak"
#include "Dark/inout"
#include "bplus.h"


signed main() {
    using tree = dark::b_plus::tree;

    typename tree::return_list data;
    dark::b_plus::tree t("a");


    t.check_function();

    return 0;
}