#include "system/ticket_system.h"
#include <filesystem>

bool work() {
    /* Wankupi is my friend :) */
    dark::ticket_system Wankupi;
    int x;
    while(!(x = Wankupi.work())) std::fflush(stdout);
    return x == 2;
}

void clean_files() {
    std::string path[] = {
        "train/o.bin",
        "train/o.dat",
        "train/omap.bin",
        "train/omap.dat",
        "train/s.bin",
        "train/s.dat",
        "train/sid.dat",
        "train/smap.bin",
        "train/smap.dat",
        "train/t.bin",
        "train/t.dat",
        "train/tset.dat",
        "user/omap.bin",
        "user/omap.dat",
        "user/user.dat",
    };
    for(auto &&iter : path) remove(iter.data());
}

signed main() {
    std::filesystem::create_directory("user");
    std::filesystem::create_directory("train");
    while(work()) clean_files();
    return 0;
}
