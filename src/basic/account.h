#ifndef _DARK_ACCOUNT_H_
#define _DARK_ACCOUNT_H_

#include "utility.h"

namespace dark {

/* Account */
struct account {
    username_t user; /* Username. */
    password_t pswd; /* Password. */
    realname_t name; /* Realname. */
    mailaddr_t mail; /* MailAddr. */
};

/* Only writeline will be used for accounts. */
void writeline(const account &__a,privilege_t __p) {
    dark::writeline(__a.user,
                    __a.pswd,
                    __a.name,
                    __a.mail,
                    privilege_table[__p]);
}


}



#endif
