#include "bplus.h"
#include "Dark/inout"
#include "string.h"
#include <filesystem>

size_t hash_str(const char *str) {
    static size_t fix_random = rand() * rand();
    size_t __h = fix_random;
    while(*str) { __h = __h * 137 + *(str++); }
    return __h;
}

signed main() {
    using tree = dark::bpt <size_t,int,1023,10000,2>;
    typename tree::return_list data;
    typename tree::iterator    iter;
    std::filesystem::create_directory("output");
    tree t("output/a");
    int n = dark::read <int> ();
    dark::string <68> str;
    while(n--) {
        dark::read(str.str);
        if(str.str[0] == 'i') {
            dark::read(str.str);
            t.insert(hash_str(str.base()),dark::read <int> ());
        } else if(str.str[0] == 'd') {
            dark::read(str.str);
            t.erase(hash_str(str.base()),dark::read <int> ());
        } else {
            dark::read(str.str);
            // t.find(hash_str(str.base()),data);
            size_t __h = hash_str(str.base());
            iter = t.lower_bound(__h);
            while(iter.valid() && iter.key() == __h) {
                data.push_back(*iter);
                ++iter;
            }
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
