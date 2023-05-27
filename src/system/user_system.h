#ifndef _TICKET_USER_SYSTEM_H_
#define _TICKET_USER_SYSTEM_H_

#include "header.h"

namespace dark {

class user_system {
  private:
    using set_t = external_hash_set <account,8093>;
    using map_t = bpt <size_t,order_info,1000,128,1>;

    set_t  user_set; /* Set of user data. */
    map_t order_map; /* Map from user to his orders. */
    account cache_account;  /* Cached. */

  public:

    /* Read in users. */
    user_system(std::string __path) :
        user_set (__path + "user"),
        order_map(__path + "omap")
    { for(auto &&iter : user_set) iter.login() = false; }

    ~user_system() = default;

    /* Return whether add_user succeed. */
    bool add_user(const char *__c,const char *__u,const char *__p,
                  const char *__n,const char *__m,const char *__g) {
        cache_account.login() = false;
        cache_account.count() = 0;

        if(user_set.empty()) { /* First insert. */
            cache_account.copy(__u,__p,__n,__m,10);
            user_set.insert(cache_account);
            return true;
        }

        auto *__a = user_set.find(string_hash(__c)); /* Account pointer. */
        auto  lvl = to_privilege(__g);   /* Real level. */

        /* No current user || Not logged in || Not enough level.  */
        if(!__a || __a->login() == false || __a->level() <= lvl) return false;

        /* Such user exists. */
        cache_account.user = __u;
        if(user_set.exist(cache_account)) return false;

        /* Be ready to insert into the map. */
        cache_account.pswd = __p;
        cache_account.name = __n;
        cache_account.mail = __m;
        cache_account.level() = lvl;
        user_set.insert(cache_account);
        return true;
    }


    /* Return whether login succeed. */
    bool login(const char *__u,const char *__p) {
        auto *__a = user_set.find(string_hash(__u)); /* Account pointer. */

        /* No such user || User has logged in || password is wrong   */
        if(!__a || __a->login() || strcmp(__a->pswd.base(),__p) != 0) return false;
        else return (__a->login() = true);
    }


    /* Return whether logout succeed. */
    bool logout(const char *__u) {
        auto *__a = user_set.find(string_hash(__u));
        /* No such user || Not logged in. */
        if(!__a || !__a->login()) return false;
        return !(__a->login() = false);
    }


    /* Return pointer to user's profile. */
    account *query_profile(const char *__c,const char *__u) {
        auto *__a = user_set.find(string_hash(__c)); /* Account pointer of __c. */

        /* No current user || Not logged in || Not enough level.  */
        if(!__a || __a->login() == false) return nullptr;
    
        /* Identical __c and __u. */
        if(strcmp(__c,__u) == 0) return __a;

        auto *__t = user_set.find(string_hash(__u)); /* Account pointer of __u. */
        /* No target user || Not enough level */
        if(!__t || __a->level() <= __t->level()) return nullptr;
        else return __t;
    }

    /* Return pointer to user's profile. */
    account *modify_profile(const char *__c,const char *__u,const char *__p,
                            const char *__n,const char *__m,const char *__g) {
        auto *__a = user_set.find(string_hash(__c)); /* Account pointer of __c. */

        /* No current user || Not logged in || Not enough level.  */
        if(!__a || __a->login() == false) return nullptr;
    
        decltype(__a) __t; /* Lazy...... */
        if(strcmp(__c,__u) != 0) { /* Not identical __c and __u. */
            __t = user_set.find(string_hash(__u)); /* Account pointer of __u. */
            /* No target user || Not enough level */
            if(!__t || __a->level() <= __t->level()) return nullptr;
        } else __t = __a;

        privilege_t lvl;
        /* Not enough level. */
        if(__g && (__a->level() <= (lvl = to_privilege(__g)) ) ) return nullptr;

        if(__p) __t->pswd = __p;
        if(__n) __t->name = __n;
        if(__m) __t->mail = __m;
        if(__g) __t->level() = lvl;

        return __t;
    }

    /* Return user's account pointer if logged in. */
    account *is_login(const char *__u) {
        auto *__a = user_set.find(string_hash(__u));
        return __a && __a->login() == true ? __a : nullptr;
    }

    /* Add a order for an given user. */
    void add_order(account *__u,int index) {
        order_map.insert(string_hash(__u->name),{++(__u->count()),0,index});
    }

    typename map_t::return_list *query_order(const char *__u) {
        size_t hid_u = string_hash(__u);
        auto *__a = user_set.find(hid_u);
        auto *vec = new typename map_t::return_list;
        if(!__a || __a->login() == false) return nullptr;
        order_map.find(hid_u,*vec);
        return vec;
    }

    /* Refund a ticket */
    int refund_ticket(const char *__u,const char *__n) {
        size_t hid_u = string_hash(__u);
        auto *__a = user_set.find(hid_u);
        if(!__a || __a->login() == false) return -1;

        int count  = __n ? to_unsigned_integer <number_t> (__n) : 1;
        if(!count || count > (int)__a->count()) return false;

        auto *__o  = order_map.find_pointer(hid_u,{__a->count() + 1 - count});
        if(__o->avail) return -1;
        __o->avail = 1;

        return __o->index;
    }

    

};



}


#endif
