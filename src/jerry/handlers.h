#ifndef _JERRY_HANDLERS_H_
#define _JERRY_HANDLERS_H_

#include "jerryscript.h"

#include "jerryscript-ext/handlers.h"
#include "jerryscript-ext/properties.h"

jerry_value_t
jz_handler_writeFile(   const jerry_call_info_t *call_info_p,
				        const jerry_value_t arguments[],
				        const jerry_length_t arguments_count);

jerry_value_t                        
jz_handler_readFile(const jerry_call_info_t *call_info_p,
				    const jerry_value_t arguments[],
				    const jerry_length_t arguments_count);


#endif //_JERRY_HANDLERS_H_