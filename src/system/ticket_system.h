#ifndef _DARK_TICKET_SYSTEM_H_
#define _DARK_TICKET_SYSTEM_H_

#include "parser.h"

namespace dark {

class ticket_system {
  private:
    command_parser Hastin;

    void add_user() {}

    void login() {}

    void log_out() {}

    void query_profile() {}

    void modify_profile() {}

    void add_train() {}

    void delete_train() {}

    void release_train() {}

    void query_train() {}

    void query_ticket() {}

    void query_transfer() {}

    void buy_ticket() {}

    void query_order() {}

    void refund_ticket() {}

    void clean() {}

  public:
    
    /**
     * @brief Switch part of the function.
     * 
     * @return False only if 
     */
    bool work() noexcept {
        switch(Hastin.parse()) {
            case command_t::A_US: add_user();       break;
            case command_t::L_IN: login();          break;
            case command_t::L_OU: log_out();        break;
            case command_t::Q_PR: query_profile();  break;
            case command_t::M_PR: modify_profile(); break;
            case command_t::A_TR: add_train();      break;
            case command_t::D_TR: delete_train();   break;
            case command_t::R_TR: release_train();  break;
            case command_t::Q_TR: query_train();    break;
            case command_t::Q_TK: query_ticket();   break;
            case command_t::Q_TF: query_transfer(); break;
            case command_t::B_TK: buy_ticket();     break;
            case command_t::Q_OR: query_order();    break;
            case command_t::R_TK: refund_ticket();  break;
            case command_t::CLR_: clean();          break;
            case command_t::EXIT: return false;
            default: ; /* This should never happen! */
        } return true;
    }
};


}

#endif
