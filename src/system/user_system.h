#ifndef _TICKET_USER_SYSTEM_H_
#define _TICKET_USER_SYSTEM_H_

#include "header.h"

namespace dark {

class user_system {
  private:
    using set_t = hash_set <size_t,4093>;

    set_t user_set; /* Users that have logged in. */

  public:


};



}


#endif
