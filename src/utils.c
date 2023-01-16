
#include "utils.h"

int utils_check_flag(const utils_flag_t flags, utils_flag_t mask)
{
    if (((flags) & (mask)) == 0)
    {
        return 0;
    }
    return 1;
}

void utils_set_flag(utils_flag_t *flags, utils_flag_t mask)
{
    (*flags) = (*flags) | mask;
}

void utils_clear_flag(utils_flag_t *flags, utils_flag_t mask)
{
    (*flags) = (*flags) & ~mask;
}