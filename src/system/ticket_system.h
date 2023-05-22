#ifndef _DARK_TICKET_SYSTEM_H_
#define _DARK_TICKET_SYSTEM_H_
#include "header.h"
#include "user_system.h"
#include "parser.h"

namespace dark {


class ticket_system : command_parser,user_system  {
  private:

    void add_user() {
        dark::writeline(
            user_system::add_user(argument('c'),argument('u'),
                                  argument('p'),argument('n'),
                                  argument('m'),argument('g'))
        );
    }

    void login() {
        dark::writeline(
            user_system::login(argument('u'),argument('p'))
        );
    }

    void log_out() {
        dark::writeline(
            user_system::logout(argument('u'))
        );
    }

    void query_profile() {
        dark::writeline(
            user_system::query_profile(argument('c'),argument('u'))
        );
    }

    void modify_profile() {
        dark::writeline(
            user_system::modify_profile(argument('c'),argument('u'),
                                        argument('p'),argument('n'),
                                        argument('m'),argument('g'))
        );
    }

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

    void quit() { dark::writeline("bye"); }
  public:
    
    ticket_system() : command_parser(),user_system("bin/user") {}
    /**
     * @brief Switch part of the function.
     * 
     * @return False only if exit case.
     */
    bool work() noexcept {
        switch(command_parser::parse()) {
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
            case command_t::EXIT: quit(); return false;
            default: ; /* This should never happen! */
        } return true;
    }
};


}

#endif
