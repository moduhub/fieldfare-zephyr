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
    printk("Zephyr main() start\n");
    
    printk("Initializing JerryScript thread\n");
    k_tid_t js_tid = k_thread_create(&jz_thread_data, jz_stack_area,
                                 K_THREAD_STACK_SIZEOF(jz_stack_area),
                                 jz_main,
                                 NULL, NULL, NULL,
                                 JS_PRIORITY, 0, K_NO_WAIT);

    printk("js thread id: %d\n", js_tid);

    while(1) {
        //blink led?
        printk("Main thread alive...\n");
        k_sleep(K_MSEC(1000));
    }
}