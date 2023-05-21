#ifndef _DARK_ACCOUNT_H_
#define _DARK_ACCOUNT_H_

#include "utility.h"

namespace dark {

struct account {
    username_t user; /* Username. */
    password_t pswd; /* Password. */
    realname_t name; /* Realname. */
    mailaddr_t mail; /* MailAddr */
    privilege_t lvl; /* Priority(Level). */
};

/* Only writeline will be used for accounts. */
void writeline(const account &__a) {
    dark::writeline(__a.user,
                    __a.pswd,
                    __a.name,
                    __a.mail,
                    privilege_table[__a.lvl]);
}


}



#endif
