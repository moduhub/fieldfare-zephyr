#include "jerry_main.h"
#include "jerry_events.h"
#include "jerry_timeout.h"
#include "handlers.h"

#include <zephyr/zephyr.h>
#include "jerryscript.h"
#include "utils.h"

jz_timeout_list_entry *timeout_list_ptr;

int jz_load_user_code (void)
{
	// const jerry_char_t script[] = "print('Hello from JavaScript');\
    //     var count = 0;\
    //     while (count < 3) {\
    //         setTimeout(() => {\
    //             print('timeout ' + count + ' called');\
    //         }, count*1000);\
    //         count++;\
    //     }\
	// 	print('script ended with count=' + count + '\n');\
	// ";

    const jerry_char_t script[] = "print('Hello from JavaScript');\
        var count = 0;\
        setTimeout(() => {\
            print('timeout!');\
        }, 1000);\
        while (count < 3) {\
            print('js count: ' + count);\
            count++;\
        }\
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

jerry_value_t
jz_timeout_handler( const jerry_call_info_t *call_info_p,
				    const jerry_value_t arguments[],
				    const jerry_length_t arguments_count)
{	
	if(arguments_count == 2) {
        jerry_value_t tCallback = arguments[0];
        jerry_value_t tTime = arguments[1];
        uint32_t cTime = jerry_value_as_uint32 (tTime);
        jz_timeout_new(timeout_list_ptr, cTime, 0, tCallback);
        jerry_value_free(tTime);
    } else {
        printk("jz_timeout_handlere error: invalid arguments");
        k_fatal_halt(EINVAL);
    }

	return jerry_undefined();
}

void jz_register_handlers()
{

    jerryx_register_global ("print", jerryx_handler_print);
    //jerryx_register_global("fs_init", fs_init_handler);

    jerryx_register_global ("setTimeout", jz_timeout_handler);

}


void jz_main (void *v1, void *v2, void *v3)
{
    jz_timeout_list_entry timeout_list[CONFIG_JZ_TIMEOUT_LIST_SIZE];
    jerry_value_t event_queue_buffer[CONFIG_JZ_EVENT_QUEUE_SIZE];
    jz_event_queue event_queue;

    jz_timeout_init(timeout_list);
    timeout_list_ptr = timeout_list;

    jz_event_queue_init(&event_queue, event_queue_buffer, CONFIG_JZ_EVENT_QUEUE_SIZE);

    jerry_init(JERRY_INIT_EMPTY);
    jz_register_handlers();
    jz_load_user_code();
    while (1)
    {
        jz_timeout_update(timeout_list, 10);
        jz_generate_timeout_events(timeout_list, &event_queue);
        if (jz_event_queue_num_entries(&event_queue) > 0)
        {
            printk(">>event queue pop\n");
            jerry_value_t tNextCall = jz_event_queue_pop(&event_queue);
            printk(">>event call: %d\n", tNextCall);
            jerry_value_t tRetValue = jerry_call(tNextCall, jerry_undefined(), NULL, 0);
            printk(">>free values\n");
            jerry_value_free(tRetValue);
            jerry_value_free(tNextCall);
            printk(">>event end\n");
        }
        else
        {
            printk("js thread idle...\n");
            k_sleep(K_MSEC(100));
        }
    }
	jerry_cleanup();
}