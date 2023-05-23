#ifndef _DARK_BPLUS_UTILITY_H_
#define _DARK_BPLUS_UTILITY_H_

#include <fstream>
#include <iostream>
#include "Dark/inout"

namespace dark {

enum class node_type : bool  { INNER = 1,OUTER = 0 };


/* Custom error caller. */
struct error {
    error(std::string str) {
        freopen("CON","w",stdout);
        std::cout << str << '\n';
    }
};


/* Simple file_state wrapper. Use highest bit to store modification state. */
struct file_state {
    int  index; /* Index of the real data. */
    bool state; /* Use highest bit to store modification state. */

    /* Return whether the file is modified. */
    bool is_modified() const noexcept { return state; }
    /* Modify the file_state. */
    void modify() noexcept { state = true; }
    
};


/* Simple file index wrapper to distinguish inner and outer node for B+ tree. */
struct header {
    int state; /* Index state of target node || outer_node < 0 || inner_node >= 0 */
    int count; /* Subtree count of target node. */

    /* Judge whether targe node is inner node. */
    bool is_inner() const noexcept { return state >= 0; }

    /* Return the real index of the target node. */
    int real_index() const noexcept { return is_inner() ? state : ~state ; }

    /**
     * @brief Set the index of the target node.
     * 
     * @param __n The real index of the node.
     * @param flag Whether the node is an inner node.
     */
    void set_index(int __n,node_type type)
    noexcept { state = bool(type) ? __n : ~__n; }
};


/* Simple compare function wrapper for B+ tree. */
template <class T>
struct Compare {
    inline int operator ()(const T &x,const T &y)
    const noexcept { return x < y ? -1 : y < x; }
};


/* A simple class , compare integers by difference. */
template <class integer>
struct Compare_Int {
    int operator ()(const integer &lhs,const integer &rhs)
    const noexcept { return lhs - rhs; }
};

}


namespace std {


/* Custom hash. */
template <>
struct hash <dark::file_state> {
    inline size_t operator() (dark::file_state t) 
    const noexcept { return t.index; }
};


/* Custom equal to. */
template <>
struct equal_to <dark::file_state> {
    bool operator() (dark::file_state x,dark::file_state y)
    const noexcept { return x.index == y.index; }
};


}

#endif
