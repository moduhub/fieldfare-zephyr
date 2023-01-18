#include "jerry_main.h"
#include "jerry_events.h"
#include "jerry_timeout.h"
#include "handlers.h"

#include <zephyr/zephyr.h>
#include "jerryscript.h"
#include "utils.h"

jz_timeout_list_entry *timeout_list_ptr;

static jerry_value_t
jz_backtrace_handler (  const jerry_call_info_t *call_info_p,
                        const jerry_value_t args_p[],
                        const jerry_length_t args_count)
{   
    if (!jerry_feature_enabled (JERRY_FEATURE_LINE_INFO))
        printk("JERRY_FEATURE_LINE_INFO is disabled, no backtrace information available.\n");
    } else {
        printk("backtrace start ---------\n");
        jerry_value_t backtrace_array = jerry_backtrace (5);
        uint32_t array_length = jerry_array_length (backtrace_array);
        printk("jz_print_backtrace array_length=%d", array_length);
        for (uint32_t idx = 0; idx < array_length; idx++)
        {
            jerry_value_t property = jerry_object_get_index (backtrace_array, idx);
            jerry_char_t string_buffer[64];
            jerry_size_t copied_bytes = jerry_string_to_buffer (property,
                                                                JERRY_ENCODING_UTF8,
                                                                string_buffer,
                                                                sizeof (string_buffer) - 1);
            string_buffer[copied_bytes] = '\0';
            printk(" %d: %s\n", idx, string_buffer);
            jerry_value_free (property);
        }
        printk("backtrace end ---------\n");
        jerry_value_free (backtrace_array);
    }
    return jerry_undefined();
}

int jz_load_user_code (void)
{
    const jerry_char_t script[] = "print('Hello from JavaScript');\
        var count = 0;\
        setTimeout(() => {\
            print('3 seconds timeout!');\
        }, 3000);\
        while (count < 10) {\
            print('js count: ' + count);\
            count++;\
        }\
        function func_a (message) {\
            print('func_a called');\
            print('message inside func is: ' + message);\
            backtrace();\
        };\
        function func_b() {\
            print('func b called');\
            func_a('xa');\
        };\
        setTimeout(() => {\
            print('10 seconds timeout!');\
        }, 10000);\
        setInterval(() => {\
            print('5 seconds interval');\
        }, 5000);\
        func_b();\
	";
	const jerry_length_t script_size = sizeof (script) - 1;
    jerry_parse_options_t parse_options;
    parse_options.options = JERRY_PARSE_HAS_SOURCE_NAME;
    parse_options.source_name = jerry_string_sz (".snapshot");
	jerry_value_t parsed_code = jerry_parse (script, script_size, NULL);
    jerry_value_free (parse_options.source_name);
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
        jerry_value_t tCallbackReference = jerry_value_copy(arguments[0]);
        uint32_t cTime = jerry_value_as_uint32 (arguments[1]);
        jz_timeout_new(timeout_list_ptr, cTime, 0, tCallbackReference);
    } else {
        printk("jz_timeout_handler error: invalid arguments");
        k_fatal_halt(EINVAL);
    }

	return jerry_undefined();
}

jerry_value_t
jz_interval_handler( const jerry_call_info_t *call_info_p,
				    const jerry_value_t arguments[],
				    const jerry_length_t arguments_count)
{	
	if(arguments_count == 2) {
        jerry_value_t tCallbackReference = jerry_value_copy(arguments[0]);
        uint32_t cTime = jerry_value_as_uint32 (arguments[1]);
        jz_timeout_new(timeout_list_ptr, cTime, JZ_TIMEOUT_OPTION_PERIODIC, tCallbackReference);
    } else {
        printk("jz_timeout_handler error: invalid arguments");
        k_fatal_halt(EINVAL);
    }

	return jerry_undefined();
}

void jz_register_handlers()
{

    jerryx_register_global ("print", jerryx_handler_print);
    jerryx_register_global ("backtrace", jz_backtrace_handler);
    //jerryx_register_global("fs_init", fs_init_handler);

    jerryx_register_global ("setTimeout", jz_timeout_handler);
    jerryx_register_global ("setInterval", jz_interval_handler);

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
        jz_generate_timeout_events(timeout_list, &event_queue);
        if (jz_event_queue_num_entries(&event_queue) > 0)
        {
            printk(">>event queue pop\n");
            jerry_value_t tNextCall = jz_event_queue_pop(&event_queue);
            jerry_value_t tRetValue = jerry_call(tNextCall, jerry_undefined(), NULL, 0);
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