#include "jerry_timeout.h"
#include <zephyr/zephyr.h>
#include <string.h>


void
jz_timeout_init(jz_timeout_list_entry *timeout_list)
{
    memset(timeout_list, 0, sizeof(jz_timeout_list_entry) * CONFIG_JZ_TIMEOUT_LIST_SIZE);
}

void
jz_timeout_new (jz_timeout_list_entry *timeout_list,
                uint32_t ms_time,
                utils_flag_t options,
                jerry_value_t callback)
{
    for(int i=0; i<CONFIG_JZ_TIMEOUT_LIST_SIZE; i++)
    {
        if (utils_check_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_ENABLED) == 0)
        {
            printk("New timeout created: callback=%d ms_time=%d\n", callback, ms_time);
            timeout_list[i].elapsed = 0;
            timeout_list[i].trigger = ms_time;
            timeout_list[i].callback = callback;
            timeout_list[i].options = options | JZ_TIMEOUT_OPTION_ENABLED;
            return;
        }
    } 
    //fatal: no free slots
    printk("jz_timeout_new fatal error: no free slots\n");
    k_fatal_halt(EOVERFLOW);
}

void
jz_timeout_update(  jz_timeout_list_entry *timeout_list,
                    int elapsed)
{
    for(int i=0; i<CONFIG_JZ_TIMEOUT_LIST_SIZE; i++)
    {
        if (utils_check_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_ENABLED)
        && timeout_list[i].elapsed < timeout_list[i].trigger)
        {
            timeout_list[i].elapsed += elapsed;
        }
    }
}

void
jz_generate_timeout_events( jz_timeout_list_entry *timeout_list,
                            jz_event_queue *event_queue)
{
    for (int i=0; i<CONFIG_JZ_TIMEOUT_LIST_SIZE; i++)
    {
        if(jz_event_queue_free_space(event_queue) == 0)
        {
            break;
        }
        if (utils_check_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_ENABLED)
        && timeout_list[i].elapsed >= timeout_list[i].trigger)
        {
            jz_event_queue_push (event_queue, timeout_list[i].callback);
            if(utils_check_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_PERIODIC) == 0)
            {
                utils_clear_flag(&timeout_list[i].options, JZ_TIMEOUT_OPTION_ENABLED);
                jerry_value_free(timeout_list[i].callback);
            }
            timeout_list[i].elapsed = 0;
        }
    }
}
