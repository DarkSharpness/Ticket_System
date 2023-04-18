#ifndef _DARK_BASIC_H_
#define _DARK_BASIC_H_

#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <version>

/**
 * @brief DarkSharpness's namespace.
 * Including some third party functions.
 * 
 */
namespace dark {

/**
 * @brief Fast built-in LOG2 function from
 * "骆可强《论程序底层优化的一些方法与技巧".
 * You can access that at 
 * https://github.com/lzyrapx/Competitive-Programming-Docs/.
 * 
 * 
 * @return log2(x) in 32-bit. Note that log2(0) will return 0;
 */
inline unsigned int LOG2(unsigned x){
    unsigned ret = 0;
    __asm__ __volatile__ ("bsrl %1, %%eax":"=a"(ret):"m"(x));
    return ret;
}

}

#endif
