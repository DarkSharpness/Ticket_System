#include "bplus.h"
#include "Dark/inout"
#include "string.h"
#include <filesystem>


signed main() {
    using tree = dark::bpt <dark::string <68>,int,100,64,4>;
    typename tree::return_list data;
    std::filesystem::create_directory("output");
    tree t("output/a");
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