#include "Dark/inout"
#include "bplus.h"


signed main() {
    using tree = dark::b_plus::tree 
        // <dark::string<68>,int,4095,200>
    ;
    typename tree::return_list data;
    tree t("a");
    int n = dark::read <int> ();
    dark::string <68> str;
    while(n--) {
        dark::read(str.str);
        if(str.str[0] == 'i') {
            dark::read(str.str);
            t.insert(str,dark::read <int> ());
        } else if(str.str[0] == 'd') {
            dark::read(str.str);
            t.erase(str,dark::read <int> ());
        } else {
            dark::read(str.str);
            t.find(str.str,data);
            if(data.empty()) puts("null");
            else {
                for(auto iter : data) dark::print(iter,' ');
                data.clear();
                putchar('\n');
            }
        }
    }
    return 0;
}