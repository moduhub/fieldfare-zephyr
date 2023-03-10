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
            timeout_list[i].callback = callback;
            timeout_list[i].ms_time = ms_time;
            timeout_list[i].trigger = sys_clock_timeout_end_calc(K_MSEC(ms_time));
            timeout_list[i].options = options | JZ_TIMEOUT_OPTION_ENABLED;
            return;
        }
    } 
    //fatal: no free slots
    printk("jz_timeout_new fatal error: no free slots\n");
    k_fatal_halt(EOVERFLOW);
}

void
jz_generate_timeout_events( jz_timeout_list_entry *timeout_list,
                            jz_event_queue *event_queue)
{
    const uint64_t uptime_ticks = k_uptime_ticks();
    for (int i=0; i<CONFIG_JZ_TIMEOUT_LIST_SIZE; i++)
    {
        if(jz_event_queue_free_space(event_queue) == 0)
        {
            break;
        }
        if (utils_check_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_ENABLED)
        && uptime_ticks > timeout_list[i].trigger)
        {
            jerry_value_t tCallbackReference = jerry_value_copy(timeout_list[i].callback);
            jz_event_queue_push (event_queue, tCallbackReference);
            if(utils_check_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_PERIODIC))
            {
                timeout_list[i].trigger = sys_clock_timeout_end_calc(K_MSEC(timeout_list[i].ms_time));
            } else {
                utils_clear_flag(&timeout_list[i].options, JZ_TIMEOUT_OPTION_ENABLED);
                jerry_value_free(timeout_list[i].callback);
                memset(&timeout_list[i], 0, sizeof(jz_timeout_list_entry));
            }
            
        }
    }
}
