#include "system/ticket_system.h"
#include <filesystem>

bool work() {
    /* Wankupi is my friend :) */
    dark::ticket_system Wankupi;
    int x;
    while(!(x = Wankupi.work()));
    return x == 2;
}

void clean_files() {
    remove("bin/");
}


signed main() {
    std::filesystem::create_directory("bin");
    while(work()) clean_files();
    return 0;
}
