#ifndef _TICKET_SYSTEM_H_
#define _TICKET_SYSTEM_H_

#include "header.h"
#include "parser.h"
#include "train_system.h"
#include "user_system.h"

namespace dark {


class ticket_system : command_parser,train_system,user_system  {
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

    void add_train() {
        dark::writeline(
            train_system::add_train(argument('i'),argument('n'),
                                    argument('m'),argument('s'),
                                    argument('p'),argument('x'),
                                    argument('t'),argument('o'),
                                    argument('d'),argument('y'))
        );
    }

    void delete_train() {
        dark::writeline(
            train_system::delete_train(argument('i'))
        );
    }

    void release_train() {
        dark::writeline(
            train_system::release_train(argument('i'))
        );
    }

    void query_train() {
        dark::writeline(
            train_system::query_train(argument('i'),argument('d'))
        );
    }

    void query_ticket() {
        dark::trivial_array <order_view> __v = 
            train_system::query_ticket(argument('s'),
                                       argument('t'),
                                       argument('d'));
        if(__v.empty()) return dark::writeline('0');

        dark::writeline(__v.size());
        auto *__p = train_system::trainID();
        if(argument('p')[0] == 't') {
            sort(__v.begin(),__v.end(),
                [=](const order_view &x,const order_view &y) ->bool {
                    return x.interval != y.interval ?
                           x.interval <  y.interval :
                           __p[x.index] < __p[y.index];
                    }
            );
        } else {
            sort(__v.begin(),__v.end(),
                [=](const order_view &x,const order_view &y) {
                    return x.price != y.price ?
                           x.price <  y.price :
                           __p[x.index] < __p[y.index];
                }
            );
        }
        const char *__s = argument('s');
        const char *__t = argument('t');
        for(auto view : __v) {
            dark::writeline(
                __p[view.index],
                __s,
                (time_wrapper){view.leaving_time()},
                "->",
                __t,
                (time_wrapper){view.arrival_time()},
                view.price,
                view.seat_num
            );
        }
    }

    void query_transfer() {
        transfer_view *view = train_system::query_transfer(
            argument('s'),
            argument('t'),
            argument('d'),
            argument('p')[0] == 't'
        );
        if(!view) return writeline('0');

        int seat_num[2];
        for(int i = 0 ; i != 2; ++i)
            seat_num[i] = /* Compiler should unfold it......maybe...... */
                train_system::query_seat(
                    view->__[i].index,
                    view->__dep[i],
                    view->__[i].start,
                    view->__[i].final
                );

        auto *__p = train_system::trainID();
        dark::writeline(
            __p[view->__[0].index],
            argument('s'),
            (time_wrapper){view->leaving_beg},
            "->",
            view->name,
            (time_wrapper){view->arrival_mid()},
            view->__[0].price,
            seat_num[0]
        );
        dark::writeline(
            __p[view->__[1].index],
            view->name,
            (time_wrapper){view->leaving_mid()},
            "->",
            argument('t'),
            (time_wrapper){view->arrival_end},
            view->__[1].price,
            seat_num[1]
        );
    }

    void buy_ticket() {
        auto *__u = user_system::is_login(argument('u'));
        auto *__o = train_system::buy_ticket(argument('i'),argument('d'),
                                             argument('n'),argument('f'),
                                             argument('t'),argument('q')[0] == 't');
        if(!__o) return (void)puts("-1");
        if(!__o->state) dark::writeline("queue");
        else dark::writeline((long long)(__o->price) * __o->count);
        user_system::add_order(__u,train_system::create_order(*__o));
    }

    void query_order() {

    }

    void refund_ticket() {

    }

    void clean() { /* This will never happen haha! */ }

    void quit() { dark::writeline("bye"); }
  public:
    
    ticket_system() :
        command_parser(),
        train_system("train/"),
         user_system("user/") {}

    /**
     * @brief Switch part of the function.
     * 
     * @return False only if exit case.
     */
    int work() noexcept {
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
            case command_t::CLR_: clean(); return 2;
            case command_t::EXIT: quit();  return 1;
            default: ; /* This should never happen! */
        } return 0;
    }
};


}

#endif
