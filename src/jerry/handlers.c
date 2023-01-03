
#include <zephyr/zephyr.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/fs/fs.h>

jerry_value_t
fs_init_handler(const jerry_call_info_t *call_info_p,
				const jerry_value_t arguments[],
				const jerry_length_t arguments_count)
{	
	printk("initializing storage\n");
	zephyr_storage_init();
	printk("storage ready\n");

	return jerry_undefined();
}

jerry_value_t
jz_handler_writeFile(   const jerry_call_info_t *call_info_p,
				        const jerry_value_t arguments[],
				        const jerry_length_t arguments_count)
{	
	// printk("Writing file\n");
	printk("number of arguments: %d\n", (sizeof(arguments) / sizeof(arguments[0])));
	// printk("Argument 1: %s\n", arguments[1]);
	// if ((sizeof(arguments) / sizeof(arguments[0])) != 3) {
	// 	printk("-- ERROR: Incorrect number of arguments on fileWrite()\n");
	// 	return jerry_undefined();
	// }

	jerry_value_t key = jerry_value_to_string(arguments[0]);
	jerry_char_t key_buffer[256];

	jerry_size_t copied_bytes = jerry_string_to_buffer(key, JERRY_ENCODING_UTF8, key_buffer, sizeof(key_buffer) - 1);
	key_buffer[copied_bytes] = '\0';

	jerry_value_free(key);

	jerry_value_t data = jerry_value_to_string(arguments[1]);
	jerry_char_t data_buffer[256];

	copied_bytes = jerry_string_to_buffer(data, JERRY_ENCODING_UTF8, data_buffer, sizeof(data_buffer) - 1);
	data_buffer[copied_bytes] = '\0';

	jerry_value_free(data);
	printk("Writing file: name: %s value: %s\n", key_buffer, data_buffer);

	/* debug only - uncomment before deploy: */
	int res = zephyr_storage_write_file(key_buffer, data_buffer);
	/* debug only - remove before deploy: */
	// int res = -1;
	printk("write result: %d\n", res);
	jerry_value_t arg[] = {0};
	if(res < 0){
		arg[0] = jerry_string("true", strlen("true"), 1);
	}
	jerry_value_t ret = jerry_call(arguments[2], jerry_undefined(), arg, 1);

	jerry_value_free(ret);

	return jerry_undefined();
}

jerry_value_t
jz_handler_readFile(const jerry_call_info_t *call_info_p,
				    const jerry_value_t arguments[],
				    const jerry_length_t arguments_count)
{
	// if(sizeof(arguments)/sizeof(arguments[0]) != 2){
	// 	return jerry_undefined();
	// }
	printk("fileRead\n");

	jerry_value_t key = jerry_value_to_string(arguments[0]);
	jerry_char_t key_buffer[256];

	jerry_size_t copied_bytes = jerry_string_to_buffer(key, JERRY_ENCODING_UTF8, key_buffer, sizeof(key_buffer) - 1);
	key_buffer[copied_bytes] = '\0';

	jerry_value_free(key);

	static char data[] = "data fromm file";
	jerry_value_t args[2] = {0, 0};
	
	/* DEBUG ONLY - UNCOMMENT BEFORE DEPLOY  */
	// int res = zephyr_storage_read_file(key_buffer, data);
	/* DEBUG ONLY - REMOVE BEFORE DEPLOY */
	int res = 1;
	if(res < 0){
		char txt[] = "true";
		args[1] = jerry_string("true", strlen("true"), 1);
		// printk("args[1]: %d ", args[1]);
		args[1] = jerry_string(txt, strlen(txt), 1);
		// printk("args[1]: %d\n", args[1]);
	} else {
		int valid = jerry_validate_string(data, strlen(data), JERRY_ENCODING_UTF8);
		if(valid){
			printk("String is valid\n");
		} else {
			printk("String is not valid\n");
		}
		// jerry_value_t data_string = jerry_string(data, strlen(data), JERRY_ENCODING_UTF8);
		// args[0] = data_string;
		args[0] = data;
		printk("args[0]: %d &data: %d\n", args[0], &data);
		// jerry_value_free(data_string);
		jerry_value_free(valid);
	}
	printk("-- here2\n");
	jerry_value_t ret = jerry_call(arguments[1], jerry_undefined(), args, 2);
	printk("-- here3\n");

	jerry_value_free(ret);
	jerry_value_free(args);

	return jerry_undefined();
}