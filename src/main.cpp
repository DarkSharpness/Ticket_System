#include "system/ticket_system.h"
#include <filesystem>

signed main() {
    std::filesystem::create_directory("bin");
    /* Wankupi is my friend :) */
    dark::ticket_system Wankupi;
    while(Wankupi.work());
    return 0;
}
