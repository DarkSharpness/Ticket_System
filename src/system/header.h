/**
 * @brief This file includes all the typedef info and 
 * basic class info (e.g. train,account,etc.).
 * You should include this file rather than
 * other modules (e.g. utility.h) directly. 
 * 
 */
#ifndef _TICKET_HEADER_H_
#define _TICKET_HEADER_H_

#include "../basic/account.h"
#include "../basic/train.h"

#include "../../BPlusTree/bplus.h"

namespace dark {

template <class RandomIt, class Compare>
void slow_sort(RandomIt first,size_t len,Compare comp) {
    for(size_t i = 0 ; i < len ; ++i)
        for(size_t j = 0 ; j < i ; ++j) 
            if(comp(first[i],first[j])) 
                std::swap(first[i],first[j]);
}

template <class RandomIt, class Compare>
void sort(RandomIt first, RandomIt last, Compare comp) {
    size_t len = last - first;
    if(len <= 16) return slow_sort(first,len,comp);
    RandomIt mid = first + rand() % len;
    std::swap(*first,*mid);
    const auto value = *first;
    RandomIt l = first,r = last - 1;
    while(l != r) {
        while(l != r && !comp(*r,value) ) --r;
        if(l != r) *l = *r;
        while(l != r && !comp(value,*l)) ++l;
        if(l != r) *r = *l;
    } // now l == r
    *l = value; 
    sort(first, l  ,comp);
    sort(l + 1,last,comp);
}


}

#endif
