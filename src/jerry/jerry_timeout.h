#ifndef _JERRY_TIMEOUT_H_
#define _JERRY_TIMEOUT_H_

#include <stdint.h>
#include "jerryscript.h"
#include "utils.h"
#include "jerry_events.h"

#ifndef CONFIG_JZ_TIMEOUT_LIST_SIZE
#define CONFIG_JZ_TIMEOUT_LIST_SIZE 10
#endif

#define JZ_TIMEOUT_OPTION_ENABLED   0x01
#define JZ_TIMEOUT_OPTION_PERIODIC  0x02

typedef struct  jz_timeout_list_entry_st {
    jerry_value_t callback;
    uint32_t elapsed;
    uint32_t trigger;
    utils_flag_t options;
} jz_timeout_list_entry;

void
jz_timeout_init(jz_timeout_list_entry *timeout_list);

void
jz_timeout_new (jz_timeout_list_entry *timeout_list,
                uint32_t ms_time,
                utils_flag_t options,
                jerry_value_t callback);

void
jz_timeout_update(  jz_timeout_list_entry *timeout_list,
                    int elapsed);

void
jz_generate_timeout_events( jz_timeout_list_entry *timeout_list,
                            jz_event_queue *event_queue);

#endif //_JERRY_TIMEOUT_H_