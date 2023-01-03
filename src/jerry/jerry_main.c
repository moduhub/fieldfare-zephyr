#include "jerry_main.h"
#include "handlers.h"

#include <stdint.h>

#include <zephyr/zephyr.h>
#include "jerryscript.h"
#include "utils.h"


#ifndef CONFIG_JZ_TIMEOUT_LIST_SIZE
#define CONFIG_JZ_TIMEOUT_LIST_SIZE 10
#endif

#ifndef CONFIG_JZ_EXEC_QUEUE_SIZE
#define CONFIG_JZ_EXEC_QUEUE_SIZE 10
#endif

#define JZ_TIMEOUT_OPTION_ENABLED   0x01
#define JZ_TIMEOUT_OPTION_PERIODIC  0x02


int jz_load_user_code (void) 
{
	const jerry_char_t script[] = "print('This is from the Jerry Thread!'); \
        var a = 5; \
		print('variable = ' + a); \
		var b = 10; \
		print('This is double = ' + b); \
		print('It works!'); \
	";

	const jerry_length_t script_size = sizeof (script) - 1;

	/* Setup Global scope code */
	jerry_value_t parsed_code = jerry_parse (script, script_size, NULL);

	if (!jerry_value_is_exception (parsed_code))
	{
		/* Execute the parsed source code in the Global scope */
		printk("Parsed code is not an exception!\n");
		jerry_value_t ret_value = jerry_run (parsed_code);

		/* Returned value must be freed */
		jerry_value_free (ret_value);
	} else {
		printk("--- ERROR: Parsed code is an exception!\n");
	}

	/* Parsed source code must be freed */
	jerry_value_free (parsed_code);

	return 0;
}

void jz_register_handlers()
{

    jerryx_register_global ("print", jerryx_handler_print);
    //jerryx_register_global("fs_init", fs_init_handler);

}

// ----- exec queue management

typedef struct {
    jerry_value_t *start;
    jerry_value_t *head;
    jerry_value_t *tail;
    jerry_value_t *end;
} jz_exec_queue;

void
jz_exec_queue_init( jz_exec_queue *queue,
                    jerry_value_t *queue_buffer,
                    unsigned int buffer_size)
{
    queue->start = queue_buffer;
    queue->head = queue_buffer;
    queue->tail = queue_buffer;
    queue->end = queue_buffer + buffer_size;
}

int
jz_exec_queue_num_entries (jz_exec_queue *queue)
{
    int tNumEntries = 0;
    if (queue->head > queue->tail)
    {
        tNumEntries = queue->head - queue->tail;
    } else 
    if (queue->head < queue->tail)
    {
        tNumEntries = queue->end - queue->head + queue->tail - queue->start;
    }
    return tNumEntries;
}

void
jz_exec_queue_push (jz_exec_queue *queue, jerry_value_t new_value)
{
    queue->head++;
    if(queue->head == queue->end)
    {
        queue->head = queue->start;
    }
    if(queue->head == queue->tail)
    {
        //fatal: overflow
        printk("jz_exec_queue_push fatal error: overflow");
    }
    *(queue->head) = new_value
}

jerry_value_t
jz_exec_queue_pop (jz_exec_queue *queue)
{
    if(queue->head == queue->tail)
    {
        //fatal: underflow
        printk("jz_exec_queue_pop fatal error: underflow");
    }
    jerry_value_t tLastValue = *(queue->tail);
    queue->tail++;
    if(queue->tail == queue->end)
    {
        queue->tail = queue->start;
    }
    return tLastValue;
}

//----- timeout list management

typedef struct  {
    jerry_value_t callback;
    uint32_t elapsed;
    uint32_t trigger;
    utils_flag_t options;
} jz_timeout_list_entry;

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
jz_timeout_enqueue_exec (   jz_timeout_list_entry *timeout_list,
                            jz_exec_queue *queue)
{
    for (int i=0; i<CONFIG_JZ_TIMEOUT_LIST_SIZE; i++)
    {
        if(jz_exec_queue_free_space() == 0)
        {
            break;
        }
        if (timeout_list[i].elapsed >= timeout_list[i].trigger)
        {
            jz_exec_queue_push (queue, timeout_list[i].callback)
            if(utils_check_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_PERIODIC) == 0)
            {
                utils_clear_flag(timeout_list[i].options, JZ_TIMEOUT_OPTION_ENABLED);
            }
            timeout_list[i].elapsed = 0;
        }
    }
}

void
jz_timeout_new (jz_timeout_list_entry *timeout_list,
                uint32_t ms_time,
                utils_flags_t options,
                jerry_value_t callback)
{
    for(int i=0; i<CONFIG_JZ_TIMEOUT_LIST_SIZE; i++)
    {
        if (utils_check_flag(timeout_list[i].flags, JZ_TIMEOUT_OPTION_ENABLED) == 0)
        {
            timeout_list[i].elapsed = 0;
            timeout_list[i].trigger = ms_time;
            timeout_list[i].callback = callback;
            timeout_list[i].options = callback | JZ_TIMEOUT_OPTION_ENABLED;
            return;
        }
    } 
    //fatal: no free slots
    printk("jz_timeout_new fatal error: no free slots");
}

//-----

void jz_main (void *, void *, void *)
{
    jz_timeout_list_entry timeout_list[CONFIG_JZ_TIMEOUT_LIST_SIZE];
    jerry_value_t exec_queue[CONFIG_JZ_EXEC_QUEUE_SIZE];
    jerry_init(JERRY_INIT_EMPTY);
    jz_register_handlers();
    jz_load_user_code();
    while (1)
    {
        jz_timeout_update(timeout_list, 10);
        jz_timeout_enqueue_exec(timeout_queue, exec_queue);
        if (jz_exec_queue_num_entries(exec_queue) > 0)
        {
            jerry_value_t tNextCall = jz_exec_queue_pop(exec_queue);
            jerry_value_t tRetValue = jerry_call(tNextCall);
            jerry_value_free(tRetValue);
            jerry_value_free(tNextCall);
        }
        else
        {
            k_sleep(K_MSEC(10));
        }
    }
	jerry_cleanup();
}