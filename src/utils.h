#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>

typedef uint32_t utils_flag_t;

int utils_check_flag(const utils_flag_t flags, utils_flag_t mask);
void utils_set_flag(utils_flag_t *flags, utils_flag_t mask);
void utils_clear_flag(utils_flag_t *flags, utils_flag_t mask);

#endif //_UTILS_H_