#include <zephyr/zephyr.h>
#include "jerry/jerry_main.h"

#define JS_STACK_SIZE 4096
#define JS_PRIORITY 5

/* Start Jerry engine on a new thread */
// K_THREAD_DEFINE(js_tid, JS_STACK_SIZE,
//                 jerry_start, NULL, NULL, NULL,
//                 JS_PRIORITY, 0, 0);

K_THREAD_STACK_DEFINE(jz_stack_area, JS_STACK_SIZE);
struct k_thread jz_thread_data;

void main() {
    printk("Zephyr main() start");
    
    printk("Initializing JerryScript thread");
    k_tid_t my_tid = k_thread_create(&jz_thread_data, jz_stack_area,
                                 K_THREAD_STACK_SIZEOF(jz_stack_area),
                                 jz_main,
                                 NULL, NULL, NULL,
                                 JS_PRIORITY, 0, K_NO_WAIT);

    while(1) {
        //blink led?
        printk('Main thread alive...');
        k_sleep(100);
    }
}