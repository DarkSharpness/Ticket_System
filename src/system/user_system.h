#ifndef _TICKET_USER_SYSTEM_H_
#define _TICKET_USER_SYSTEM_H_

#include "header.h"

namespace dark {

class user_system {
  private:
    using set_t = hash_set <account,8093>;

    std::fstream file;
    set_t user_set; /* Users that have logged in. */
    account temp;   /* Temp account info. */

  public:

    user_system(std::string __path) {
        __path += ".bin";
        file.open(__path,std::ios::in | std::ios::out | std::ios::binary);
        if(!file.good()) {
            file.close(); file.open(__path,std::ios::out | std::ios::binary);
        } else { /* Read from memory. */
            /* First read count. */
            file.seekg(0); size_t count;
            file.read((char *)&count,sizeof(count));

            /* Construct the array for read-in. */
            dark::trivial_array <account> t; t.resize(count);
            file.read((char *)&t,count * sizeof(account));

            /* Fill the set with data. */
            for(auto &&iter : t) {
                iter.login() = false;
                user_set.insert(iter);
            }
        }
    }

    ~user_system() {
        file.seekp(0);
        dark::trivial_array <account> t = user_set.move_data();
        /* First write count */
        size_t count = t.size();
        file.write((const char *)&count,sizeof(count));

        /* Next write main data.  */
        file.write((const char *)&t,count * sizeof(account));
    }


    bool add_user(const char *__c,const char *__u,const char *__p,
                  const char *__n,const char *__m,const char *__g) {
        temp.login() = false;

        if(user_set.empty()) { /* First insert. */
            temp.copy(__u,__p,__n,__m,10);
            user_set.insert(temp);
            return true;
        }

        auto *__a = user_set.find(string_hash(__c)); /* Account pointer. */
        auto  lvl = to_privilege(__g);   /* Real level. */

        /* No current user || Not logged in || Not enough level.  */
        if(!__a || __a->login() == false || __a->level() <= lvl) return false;

        /* Such user exists. */
        temp.user = __u;
        if(user_set.exist(temp)) return false;

        /* Sure to insert into the map. */
        temp.pswd = __p;
        temp.name = __n;
        temp.mail = __m;
        temp.level() = lvl;
        user_set.insert(temp);
        return true;
    }

    bool login(const char *__u,const char *__p) {
        auto *__a = user_set.find(string_hash(__u)); /* Account pointer. */
        
        /* No such user || User has logged in || password is wrong   */
        if(!__a || __a->login() || strcmp(__a->pswd.base(),__p) != 0) return false;
        else return (__a->login() = true);
    }

    bool logout(const char *__u) {
        auto __a = user_set.find(string_hash(__u));
        /* No such user || Not logged in. */
        if(!__a || !__a->login()) return false;
        return !(__a->login() = false); 
    }

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

    account *modify_profile(const char *__c,const char *__u,const char *__p,
                            const char *__n,const char *__m,const char *__g) {
        auto *__a = user_set.find(string_hash(__c)); /* Account pointer of __c. */

        /* No current user || Not logged in || Not enough level.  */
        if(!__a || __a->login() == false) return nullptr;
    
        decltype(__a) __t;
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



};



}


#endif
